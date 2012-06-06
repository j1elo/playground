/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  Authors:
 *  2003-12-01: Manuel Freire Moran
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <map>
#include <string>
#include <vector>
#include <algorithm>

#include "sock.h"
#include "semaphore.h"

#define MAX_BUFF 1024
#define ERR_SIZE (MAX_BUFF-100)
#define MAX_THREADS 10
#define DEFAULT_TIMEOUT 10000
#define MAX_CONN_QUEUE 5

using namespace std;
extern char **environ;

/*
 * typedefs
 */

typedef unsigned char byte;

typedef struct msg {
    byte b[MAX_BUFF];
    int len;
};

typedef vector<msg> msgbuffer;
typedef map<string,int> connectionmap;
typedef map<string,Semaphore *> synchpointmap;

// the state of the application at a given point
typedef struct state {
    connectionmap *C;   // names of open sockets
    msgbuffer *L;       // messages not yet delivered
    synchpointmap *P;   // synchpoints that have been defined
};

typedef int (*commandf) (struct command *C, state *S, char *buffer); 

// commands get parsed into this structure
typedef struct command {
    string op;          // the whole command string
    commandf f;         // the function to use in executing it (set when parsed)
    char* args;         // pointer to start of command args inside 'op'
    int timeout;        // max timeout ('<' command modifier)
    int err;            // expected errno ('!' modifier; 0 = none, -1 = all)
};

typedef vector<command> commandlist;

// used to pass initialization arguments to worker threads
typedef struct argBundle {
    int id;             // thread id
    commandlist CL;     // list of commands to execute in thread
    Semaphore mutex;    // arg mutex (to avoid overwriting args)
    Semaphore console;  // console mutex (only one thread should write at once)
    state S;            // app 'state'
};

/*
 * prototypes
 */

int readFile(FILE *f, commandlist *CL, synchpointmap *M);
void *testThread(void *args);
void sighandler(int n);

int doConnect(command *C, state *S, char *buffer);
int doOpen(command *C, state *S, char *buffer);
int doPrint(command *C, state *S, char *buffer);
int doAccept(command *C, state *S, char *buffer);
int doClose(command *C, state *S, char *buffer);
int doSend(command *C, state *S, char *buffer);
int doExtern(command *C, state *S, char *buffer);
int doReceive(command *C, state *S, char *buffer);
int doWait(command *C, state *S, char *buffer);
int doSynch(command *C, state *S, char *buffer);

void dumpBytes(char *buffer, byte *b, int n);

int getErrNum(char *name);
void stringToBytes(char *s, unsigned char *b, int *numBytes);
command createCommand(char *op);

/*
 * globals
 */
pthread_t g_mainThread;

// table of names-to-functions mappings
const struct {
    char *name;
    commandf handler;
} opNames[] = {
    {"connect", doConnect},
    {"open", doOpen},
    {"print", doPrint},
    {"accept", doAccept},
    {"close", doClose},
    {"send", doSend},
    {"extern", doExtern},
    {"receive", doReceive},
    {"wait", doWait},
    {"synch", doSynch},
    {NULL, NULL}
};

/*
 * functions
 */

/**************************************************************************
 * sighandler: catch signals and exit
 **************************************************************************/
void sighandler(int n) {
    printf("[INFO] caught %d\n", n);
    if (pthread_equal(pthread_self(), g_mainThread)) exit(0);
}

/**************************************************************************
 * readFile: read the file into memory
 **************************************************************************/
int readFile(FILE *f, commandlist *C, synchpointmap *M) {
    char buffer[MAX_BUFF];
    char *opStart;    
    int len;
    int currentThread;
    Semaphore *sem;
    command newCommand;
    
    for (int lineNum=0; !feof(f); lineNum++) {
        fgets(buffer, MAX_BUFF, f);
        if (feof(f)) break;
                
        // discard whitespace and comments
        if (strchr("\n\r\t #", buffer[0]) != NULL) 
            continue;
        
        // find thread
        if (strncmp("thread", buffer, strlen("thread")) == 0) {
            sscanf(buffer+strlen("thread"), "%d", &currentThread);
            if (currentThread > MAX_THREADS-1) {
                printf("line %d: thread number too high\n", lineNum);
                return -1;
            }
            continue;
        }
        
        // find synchpoint
        if (strncmp("synch", buffer, strlen("synch")) == 0) {
            len = strlen(buffer);
            opStart = buffer + strlen("synch ");
            buffer[len-1] = '\0';
            sem = (*M)[string(opStart)];
            if (sem == NULL) {
                sem = (Semaphore *)malloc(sizeof(Semaphore));
                (*M)[string(opStart)] = sem;
                semaphore_init(sem, 1);                
            }
            sem->v --;
            buffer[len-1] = '\n';
        }
        
        // create op    
        len = strlen(buffer);
        buffer[len-1] = 0;
        C[currentThread].push_back(createCommand(buffer)); 
    }
    
    return 0;
}

/**************************************************************************
 * create a 'command' stucture from a c string
 **************************************************************************/
command createCommand(char *op) {
    char buffer[MAX_BUFF];
    int offset;

    command C = {
        string(op), NULL, NULL, DEFAULT_TIMEOUT, 0
    };

    // parse modifiers
    if (op[0] == '!') {
        sscanf(op, "%s %n", buffer, &offset);
        C.err = -1;
        if (strlen(buffer) != 1) {
            C.err = getErrNum(buffer+1);
        }
        op += offset;
    }
    if (op[0] == '<') {
        sscanf(op, "<%d %n", &C.timeout, &offset);
        op += offset;
    }
    if (op[0] == '!') {
        sscanf(op, "%s %n", buffer, &offset);
        if (strlen(buffer) != 1) {
            C.err = getErrNum(buffer+1);
        }
        op += offset;
    }        
    
    // find op
    for (int i=0; opNames[i].name != NULL; i++) {
        int nameLen = strlen(opNames[i].name);
        if (strncmp(opNames[i].name, op, nameLen) == 0) {
            C.f = opNames[i].handler;
            C.args = strstr(C.op.c_str(), opNames[i].name) + nameLen;
            if (C.args[0] == ' ') {
                C.args ++;
            }
            else {
                C.args = NULL; // better no args than bad args
            }
            break;
        }
    }
    if (C.f == NULL) {
        printf("[ERROR] - The operation '%s' is not defined\n", op);
    }
        
    return C;
}


/**************************************************************************
 * main: read args, read file, launch threads, collect
 **************************************************************************/
int main(int argc, char **argv) {
    
    // check args
    if (argc != 2) {
        printf("Argument expected: protocol file\n");
        exit(1);
    }
        
    // read file into big vector
    FILE *f;
    if ((f = fopen(argv[1], "rb")) == NULL) {
        perror("fopen: ");
        return -1;
    }
    commandlist CL[MAX_THREADS];
    synchpointmap P;
    connectionmap C;
    msgbuffer L;
    readFile(f, CL, &P);
    

    // launch the threads
    g_mainThread = pthread_self();
    signal(SIGINT, sighandler);
    argBundle bundle;
    semaphore_init(&bundle.mutex, 1);
    semaphore_init(&bundle.console, 1);
    bundle.S.C = &C;
    bundle.S.L = &L;
    bundle.S.P = &P;

    int i;
    int rc;
    pthread_t T[MAX_THREADS];
    for (i=0; i<MAX_THREADS; i++) {
        if (CL[i].size() == 0) continue;
        semaphore_down(&bundle.mutex);
        bundle.id = i;
        bundle.CL = CL[i];
        rc = pthread_create(&(T[i]), NULL, testThread, &bundle);
        if (rc != 0) {
            perror("pthread_create");
            break;
        }
    }
    
    // join all threads and exit
    for (i=0; i<MAX_THREADS; i++) {
        if (CL[i].size() != 0) pthread_join(T[i], NULL);
    }
        
    return 0;
}

/**************************************************************************
 * testThread: a worker thread
 **************************************************************************/
void *testThread(void *args) {
    
    int id = ((argBundle *)args)->id;
    state S = ((argBundle *)args)->S;
    commandlist CL = ((argBundle *)args)->CL;
    Semaphore *console = &((argBundle *)args)->console;
    semaphore_up(&((argBundle *)args)->mutex);
    
    msgbuffer L;    
    char buffer[MAX_BUFF*4];
    struct timeval tv;
    struct timezone tz;
    struct tm broken;
    command *current;
    int rc;
    int ok;
    for (uint i=0; i<CL.size(); i++) {        
        current = &(CL[i]);
        
        gettimeofday(&tv, &tz);  
        gmtime_r(&tv.tv_sec, &broken);
        semaphore_down(console);
        printf("%d.%2.2d - %2.2d:%2.2d:%2.2d.%6.6d: %s\n", id, i,
            broken.tm_hour, 
            broken.tm_min, broken.tm_sec, (int)tv.tv_usec, current->op.c_str());
        fflush(stdout);
        semaphore_up(console);        

        rc = current->f(current, &S, buffer);
        ok = (current->err == rc) || (current->err ==-1 && rc != 0) ;
        
        semaphore_down(console);
        printf("%d.%2.2d\t[%s] %s\n", id, i, (ok?"OK":"ERROR"), buffer);
        fflush(stdout);
        semaphore_up(console);
    }    
    
    return NULL;
}

/**************************************************************************
 * doPrint: parses 'print <text>'
 **************************************************************************/
int doPrint(command *C, state *S, char *buffer) {
    sprintf(buffer, "print: %s", C->args);
    return 0;
}

/**************************************************************************
 * doConnect: parses 'connect <addr>:<port> as <id>'
 **************************************************************************/
int doConnect(command *C, state *S, char *buffer) {
    char *tmp;
    char *addr;
    char *cname;
    int port;
    int rc;
    char *op = C->args;
    tmp = strchr(C->args, ':');
    tmp[0] = 0;
    addr = op;
    op = tmp+1;
    tmp = strchr(op, ' ');
    tmp[0] = 0;
    port = atoi(op);
    rc = conecta(addr, port);
    // non-blocking: || fcntl(rc, F_SETFL, O_NONBLOCK) != 0
    if (rc < 0) { 
        rc = errno;    
        sprintf(buffer, "connect: %s", strerror(rc));
        return rc;
    }
    op = tmp+1;
    cname = strstr(op, "as ") + strlen("as ");
    (*(S->C))[string(cname)] = rc;
    sprintf(buffer, "connect");
    return 0;
}

/**************************************************************************
 * doOpen: parses 'open <port> as <id>'
 **************************************************************************/
int doOpen(command *C, state *S, char *buffer) {
    char *cname;
    int port;
    int rc;
    int offset;
    sscanf(C->args, "%d as %n", &port, &offset);    
    cname = C->args + offset;
    rc = preparaEscucha(port, MAX_CONN_QUEUE);
    if (rc < 0) {
        rc = errno;    
        sprintf(buffer, "open: %s", strerror(rc));
        return rc;
    }
    (*(S->C))[string(cname)] = rc;
    sprintf(buffer, "open");
    return 0;
}

/**************************************************************************
 * doAccept: parses 'accept <id> as <id>'
 **************************************************************************/
int doAccept(command *C, state *S, char *buffer) {
    char *lname;
    char *tmp;
    char *cname;
    int sock;
    int rc;
    lname = C->args;
    tmp = strchr(lname, ' ');
    tmp[0] = 0;
    cname = strstr(tmp+1, "as ") + strlen("as ");
    sock = (*(S->C))[string(lname)];
    
    // realiza un poll + accept (con timeout para evitar accept bloqueante)
    rc = pollIn(sock, C->timeout); 
    if (rc < 0) {
        sprintf(buffer, "poll-accept: %s", strerror(errno));
        return ETIME;
    }    
    rc = aceptaConexion(sock);
    if (rc < 0) {
        rc = errno;
        sprintf(buffer, "accept: %s", strerror(rc));
        return rc;
    }
    
    (*(S->C))[string(cname)] = rc;
    sprintf(buffer, "open");
    return 0;
}

/**************************************************************************
 * doClose: parses 'close <id>'
 **************************************************************************/
int doClose(command *C, state *S, char *buffer) {
    int port;
    int rc;
    port = (*(S->C))[string(C->args)];
    (*(S->C)).erase(string(C->args));
    rc = close(port); 
    if (rc < 0) {
        rc = errno;
        sprintf(buffer, "close: %s", strerror(rc));
        return rc;
    }
    sprintf(buffer, "close");
    return 0;
}

/**************************************************************************
 * doSend: parses 'send <who> <what>'
 **************************************************************************/
int doSend(command *C, state *S, char *buffer) {
    byte toSend[MAX_BUFF];
    char *tmp, *data;
    int sock;
    int len;
    int rc;    
    tmp = strchr(C->args, ' ');
    tmp[0] = 0;
    sock = (*(S->C))[string(C->args)];
    rc = pollOut(sock, C->timeout); 
    if (rc < 0) {
        sprintf(buffer, "poll-send: %s", strerror(errno));
        return ETIME;
    }    
    data = tmp + 1;
    stringToBytes(data, (byte *)toSend, &len);
    rc = envia(sock, (void *)toSend, len);
    toSend[len] = 0;
    if (rc < 0) {
        rc = errno;
        sprintf(buffer, "envia: %s", strerror(rc));      
        return rc;
    }
    sprintf(buffer, "%d bytes sent:\n\t", rc);
    dumpBytes(buffer+strlen(buffer), toSend, rc);    
    return 0;
}

/**************************************************************************
 * doExtern: parses 'extern <who> <what>'
 **************************************************************************/
int doExtern(command *C, state *S, char *buffer) {
    char toSend[1];
    char *tmp;
    char *end;
    int sock;
    int rc;
    char *argv[100];
    int argc;
    
    tmp = strchr(C->args, ' ');
    tmp[0] = 0;
    sock = (*(S->C))[string(C->args)];
    tmp ++;
    end = tmp +strlen(tmp);
    //printf("tmp is '%s'\n", tmp);
    
    for (argc=0; tmp != NULL; argc++) {
        argv[argc] = tmp;
        tmp = strchr(tmp, ' ');
        if (tmp == NULL) break;
        tmp[0] = 0;
        tmp ++;
        while (tmp != end && isspace(tmp[0])) tmp++;
        //printf("arg #%d is '%s'\n", argc, argv[argc]);
    }
    //printf("arg #%d is '%s'\n", argc, argv[argc]);
    argv[argc+1] = NULL;    
    //printf("arg #%d is NULL\n", argc+1);
    
    int pdes[2];
    int childpid;
    pipe(pdes);
    childpid = fork();
        //printf("childpid is %d\n", childpid);
   
    switch (childpid) {
        case 0:
            rc = dup2(pdes[1], 1);
            if (rc < 0) perror("unable to dup2");
            rc = execve(argv[0], argv, environ);
            if (rc == -1) perror("unable to execve");
            // not reached
                //printf("[ERROR] Child exiting...");
            exit(-1);
        case -1:
            rc = errno;
            sprintf(buffer, "extern-fork: %s", strerror(rc));
            return rc;
        default:
                //printf("parent survived!\n");
            break;
    }        
       
    // send output
        //printf("file opened\n");
    FILE *fe = fdopen(pdes[0], "rb");
    int c;
    int status;
    close(pdes[1]);
    
    while ((c = fgetc(fe)) != EOF) {
        toSend[0] = (unsigned char)c;
        printf("%c", toSend[0]);
        rc = envia(sock, (void *)toSend, 1);        
        if (rc < 0) {
            rc = errno;
            sprintf(buffer, "extern-envia: %s", strerror(rc));                  
            break;
        }
    }    
    
    waitpid(childpid, &status, 0);    
    sprintf(buffer, "sent extern, exited %s with %d", 
            (WIFEXITED(status)?"OK":"Forcibly"), WEXITSTATUS(status));
    
    fclose(fe);
    return rc;
}

/**************************************************************************
 * doReceive: parses 'receive <who> <start>'
 **************************************************************************/
int doReceive(command *C, state *S, char *buffer) {
    byte recvd[MAX_BUFF];
    msg end;
    msg start;
    msg *other;
    char *tmp;
    int sock;
    int rc;
    
    tmp = strchr(C->args, ' ');    
    if (tmp != NULL) {
        tmp[0] = 0;
        stringToBytes(tmp+1, start.b, &start.len);
    }
    else {
        start.len = 0;
    }
    sock = (*(S->C))[string(C->args)];
    
    // check if previously received
    rc = -1;
    for (uint i=0; i<S->L->size(); i++) { 
        other = &((*(S->L))[i]);
        if (other->len >= start.len && 
            memcmp(other->b, start.b, start.len)==0) 
        {
            memcpy(recvd, other->b, other->len);
            S->L->erase(S->L->begin() + i);
            rc = other->len;
            break;
        }
    }
    
    // if not prior, try to receive now
    if (rc < 0) {
        // printf("READY.... \n");
        rc = pollIn(sock, C->timeout); 
        if (rc < 0) {
            rc = errno;
            sprintf(buffer, "poll-receive: %s", strerror(rc));
            return rc;
        }
        // printf("WAITING.... \n");
        rc = recibe(sock, (void *)recvd, MAX_BUFF);
        if (rc < 0) {
            rc = errno;
            sprintf(buffer, "receive: %s", strerror(rc));
            return rc;
        }
        // if non-matching, store for later
        if (rc < start.len || memcmp(recvd, start.b, start.len) != 0) {
            memcpy(end.b, recvd, rc);
            end.len = rc;
            S->L->push_back(end);
            rc = -1;
        }
    }    
    
    // print results
    if (rc > 0) {
        sprintf(buffer, "received matches pattern.\n\texpected:\n\t");
        if (start.len > 0) {
            dumpBytes(buffer+strlen(buffer), start.b, start.len);    
        }
        else {
            sprintf(buffer+strlen(buffer), "*");
        }
        sprintf(buffer+strlen(buffer), "\n\treceived:\n\t");
        dumpBytes(buffer+strlen(buffer), recvd, rc); 
        rc = 0;    
    }
    else {
        sprintf(buffer, "no match.\n\texpected:\n\t");        
        dumpBytes(buffer + strlen(buffer), start.b, start.len);    
        sprintf(buffer + strlen(buffer), "\n\treceived/queued:");
        for (uint j=0; j<S->L->size(); j++) {
    	    sprintf(buffer + strlen(buffer), "\n\t<queue pos. %d>\n\t", j);
            dumpBytes(buffer + strlen(buffer), 
                (*(S->L))[j].b, (*(S->L))[j].len);
        }
        rc = ENOMSG;
    }
    return rc;
}

/**************************************************************************
 * doWait: parses 'wait <time>'
 **************************************************************************/
int doWait(command *C, state *S, char *buffer) {
    int millis;
    struct timeval tv;
    millis = atoi(C->args);
    tv.tv_sec = millis/1000;
    tv.tv_usec = millis%1000;
    select(0, NULL, NULL, NULL, &tv);
    sprintf(buffer, "wait");
    return 0;
}

/**************************************************************************
 * doSynch: parses 'synch <id>'
 **************************************************************************/
int doSynch(command *C, state *S, char *buffer) {
    Semaphore *sem;    
    sem = (*(S->P))[string(C->args)];
    semaphore_up2(sem);
    semaphore_wait(sem);
    sprintf(buffer, "synch");
    return 0;
}

/**************************************************************************
 * stringToBytes: parses a string with c-like escapes into bytes
 **************************************************************************/
void stringToBytes(char *s, byte *b, int *numBytes) {
    int len = strlen(s);
    *numBytes = 0;
    for (int i=0; i<len; i++) {
        if (s[i] == '\\' && (i+1 < len)) {
            switch(s[i+1]) {
                case '0': *(b++) = '\0'; break;
                case 'n': *(b++) = '\n'; break;
                case 'r': *(b++) = '\r'; break;
                case 'f': *(b++) = '\f'; break;
                case 't': *(b++) = '\t'; break;
                default:  *(b++) = s[i+1];
            }
            i++;
        }    
        else {
            *(b ++) = s[i];            
        }
        (*numBytes) ++;       
    }
}

/**************************************************************************
 * dumpBytes: prints hex and ASCII representations of data into a buffer
 **************************************************************************/
void dumpBytes(char *s, byte *datos, int nDatos) {
    int i, j, len;
    len = 0;
    for (i=0; i<nDatos; i += 16) {
        sprintf(s+len,"%4.4d ", i);
        len += 5;
        for (j=0; j<16; j++) {
            if (i+j<nDatos) sprintf(s+len,"%2.2x ", (unsigned char)datos[i+j]);
            else sprintf(s+len,".. ");
            len +=3;
        }
        sprintf(s+len, " ");
        len ++;
        for (j=0; j<16; j++) {
            if (i+j<nDatos) sprintf(s+len,"%c", isprint(datos[i+j])?datos[i+j]:'.');
            else sprintf(s+len," ");
            len ++;
        }
        if (i+16<nDatos) sprintf(s+len, "\n\t");
        len +=2;
    }
}

/**************************************************************************
 * getErrNum: returns an 'errno' code for a given error name (or number!)
 **************************************************************************/

//
// to generate:
// - grep define $(find /usr/include -name errno.h | xargs) | sed -e "s/.*define[\t ]*//g" | sort | uniq > t
// - sed -e 's/\([^ \t]*\)[ \t][ \t]*\([^ \t]*\)[ \t][ \t]*\(.*\)/{"\1",\2}, \3/g' t > t2 
// (still misses some obscure errors, but is anyway more than enough)
//

const struct {
    char *name;
    int number;
} errList[] = {
    {"EADDRINUSE",98}, /* Address already in use */
    {"EADDRNOTAVAIL",99}, /* Cannot assign requested address */
    {"EADV",68}, /* Advertise error */
    {"EAFNOSUPPORT",97}, /* Address family not supported by protocol */
    {"EALREADY",114}, /* Operation already in progress */
    {"EBADCOOKIE",523}, /* Cookie is stale */
    {"EBADE",52}, /* Invalid exchange */
    {"EBADFD",77}, /* File descriptor in bad state */
    {"EBADHANDLE",521}, /* Illegal NFS file handle */
    {"EBADMSG",74}, /* Not a data message */
    {"EBADR",53}, /* Invalid request descriptor */
    {"EBADRQC",56}, /* Invalid request code */
    {"EBADSLT",57}, /* Invalid slot */
    {"EBADTYPE",527}, /* Type not supported by server */
    {"EBFONT",59}, /* Bad font file format */
    {"ECANCELED", 125},
    {"ECANCELED",125}, /* Operation Canceled */
    {"ECHRNG",44}, /* Channel number out of range */
    {"ECOMM",70}, /* Communication error on send */
    {"ECONNABORTED",103}, /* Software caused connection abort */
    {"ECONNREFUSED",111}, /* Connection refused */
    {"ECONNRESET",104}, /* Connection reset by peer */
    {"EDEADLK",35}, /* Resource deadlock would occur */
    {"EDESTADDRREQ",89}, /* Destination address required */
    {"EDOM",33}, /* Math argument out of domain of function.  */
    {"EDOTDOT",73}, /* RFS specific error */
    {"EDQUOT",122}, /* Quota exceeded */
    {"EHOSTDOWN",112}, /* Host is down */
    {"EHOSTUNREACH",113}, /* No route to host */
    {"EIDRM",43}, /* Identifier removed */
    {"EILSEQ",84}, /* Illegal byte sequence */
    {"EILSEQ",84}, /* Illegal byte sequence.  */
    {"EINPROGRESS",115}, /* Operation now in progress */
    {"EIOCBQUEUED",529}, /* iocb queued, will get completion event */
    {"EIOCBRETRY",530}, /* iocb queued, will trigger a retry */
    {"EISCONN",106}, /* Transport endpoint is already connected */
    {"EISNAM",120}, /* Is a named type file */
    {"EJUKEBOX",528}, /* Request initiated, but will not complete before timeout */
    {"EKEYEXPIRED",127}, /* Key has expired */
    {"EKEYREJECTED",129}, /* Key was rejected by service */
    {"EKEYREVOKED",128}, /* Key has been revoked */
    {"EL2HLT",51}, /* Level 2 halted */
    {"EL2NSYNC",45}, /* Level 2 not synchronized */
    {"EL3HLT",46}, /* Level 3 halted */
    {"EL3RST",47}, /* Level 3 reset */
    {"ELIBACC",79}, /* Can not access a needed shared library */
    {"ELIBBAD",80}, /* Accessing a corrupted shared library */
    {"ELIBEXEC",83}, /* Cannot exec a shared library directly */
    {"ELIBMAX",82}, /* Attempting to link in too many shared libraries */
    {"ELIBSCN",81}, /* .lib section in a.out corrupted */
    {"ELNRNG",48}, /* Link number out of range */
    {"ELOOP",40}, /* Too many symbolic links encountered */
    {"EMEDIUMTYPE",124}, /* Wrong medium type */
    {"EMSGSIZE",90}, /* Message too long */
    {"EMULTIHOP",72}, /* Multihop attempted */
    {"ENAMETOOLONG",36}, /* File name too long */
    {"ENAVAIL",119}, /* No XENIX semaphores available */
    {"ENETDOWN",100}, /* Network is down */
    {"ENETRESET",102}, /* Network dropped connection because of reset */
    {"ENETUNREACH",101}, /* Network is unreachable */
    {"ENOANO",55}, /* No anode */
    {"ENOBUFS",105}, /* No buffer space available */
    {"ENOCSI",50}, /* No CSI structure available */
    {"ENODATA",61}, /* No data available */
    {"ENOIOCTLCMD",515}, /* No ioctl command */
    {"ENOKEY",126}, /* Required key not available */
    {"ENOLCK",37}, /* No record locks available */
    {"ENOLINK",67}, /* Link has been severed */
    {"ENOMEDIUM",123}, /* No medium found */
    {"ENOMSG",42}, /* No message of desired type */
    {"ENONET",64}, /* Machine is not on the network */
    {"ENOPKG",65}, /* Package not installed */
    {"ENOPROTOOPT",92}, /* Protocol not available */
    {"ENOSR",63}, /* Out of streams resources */
    {"ENOSTR",60}, /* Device not a stream */
    {"ENOSYS",38}, /* Function not implemented */
    {"ENOTCONN",107}, /* Transport endpoint is not connected */
    {"ENOTEMPTY",39}, /* Directory not empty */
    {"ENOTNAM",118}, /* Not a XENIX named type file */
    {"ENOTRECOVERABLE",131}, /* State not recoverable */
    {"ENOTSOCK",88}, /* Socket operation on non-socket */
    {"ENOTSUPP",524}, /* Operation is not supported */
    {"ENOTSYNC",522}, /* Update synchronization mismatch */
    {"ENOTUNIQ",76}, /* Name not unique on network */
    {"EOPNOTSUPP",95}, /* Operation not supported on transport endpoint */
    {"EOWNERDEAD",130}, /* Owner died */
    {"EPFNOSUPPORT",96}, /* Protocol family not supported */
    {"EPROTO",71}, /* Protocol error */
    {"EPROTONOSUPPORT",93}, /* Protocol not supported */
    {"EPROTOTYPE",91}, /* Protocol wrong type for socket */
    {"ERANGE",34}, /* Math result not representable.  */
    {"EREMCHG",78}, /* Remote address changed */
    {"EREMOTE",66}, /* Object is remote */
    {"EREMOTEIO",121}, /* Remote I/O error */
    {"ERESTART",85}, /* Interrupted system call should be restarted */
    {"ERESTARTNOHAND",514}, /* restart if no handler.. */
    {"ERESTART_RESTARTBLOCK",516}, /* restart by calling sys_restart_syscall */
    {"ESERVERFAULT",526}, /* An untranslatable error occurred */
    {"ESHUTDOWN",108}, /* Cannot send after transport endpoint shutdown */
    {"ESOCKTNOSUPPORT",94}, /* Socket type not supported */
    {"ESRMNT",69}, /* Srmount error */
    {"ESTALE",116}, /* Stale NFS file handle */
    {"ESTRPIPE",86}, /* Streams pipe error */
    {"ETIME",62}, /* Timer expired */
    {"ETIMEDOUT",110}, /* Connection timed out */
    {"ETOOMANYREFS",109}, /* Too many references: cannot splice */
    {"ETOOSMALL",525}, /* Buffer or request is too small */
    {"EUCLEAN",117}, /* Structure needs cleaning */
    {"EUNATCH",49}, /* Protocol driver not attached */
    {"EUSERS",87}, /* Too many users */
    {"EWOULDBLOCK",EAGAIN}, /* Operation would block */
    {"EXFULL",54}, /* Exchange full */
    {NULL, -1}
};

int getErrNum(char *err) {
    
    // if a number, parse & return it
    if (err[0] >= '0' && err[0] <= '9') {
        return atoi(err);
    }
    
    // otherwise, good ol' if-block
    for (int i=0; errList[i].name != NULL; i++) {
        if (strncmp(errList[i].name, err, strlen(errList[i].name)) == 0) {
            return errList[i].number;
        }
    }

    // not found - any error will do
    printf("[ERROR] Error '%s' not recognized: you are making it up\n", err);
    return -1;
}
