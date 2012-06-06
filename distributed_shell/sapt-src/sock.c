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
 *  
 *  Changelog:
 *  2003-12-01: created file
 *  2004-10-21: added better pollIn behaviour
 *  2005-09-30: added DEBUG and muestraDatos
 */

/*
 * sock.c
 *      implementacion de una sencilla libreria de funciones para facilitar la
 *      el uso de sockets TCP
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/timeb.h>
#include <sys/unistd.h>
#include <time.h>
#include <stdio.h>
#include <sys/poll.h>
#include <errno.h>
#include <strings.h>
#include <netdb.h>

#include "sock.h"

/*
 * preparaEscucha
 *      Prepara un "socket" de servidor
 *      para recibir conexiones por un puerto dado
 *
 * entra: el identificador del puerto, num. max. de conexiones pendientes
 * sale: el descriptor de socket de escucha obtenido
 *       -1 si no se establece conexion
 */
int preparaEscucha(int puerto, int maxClientes) {
    struct sockaddr_in server;
    int s_id;
    int rc;
    int numRaro;

    // consigue un socket de servidor
    rc = socket(PF_INET, SOCK_STREAM, 0);
    if (rc == -1) return -1;
    s_id = rc;

    // evita problemas de 'bind'
    rc = setsockopt(s_id, SOL_SOCKET,SO_REUSEADDR,&numRaro,sizeof(int));
    if (rc == -1) return -1;

    // le asocia un puerto de escucha
    server.sin_family = AF_INET;         // familia = IP
    server.sin_port   = htons(puerto);   // puerto de escucha, en orden de red
    server.sin_addr.s_addr = INADDR_ANY; // direcciones aceptadas = todas

    rc = bind(s_id, (struct sockaddr *)&server, sizeof(server));
    if (rc == -1) return -1;

    // y le pone a escuchar
    rc = listen(s_id, maxClientes);

    if (rc == -1) return -1;
    else return s_id;
}

/*
 * aceptaConexion
 *      Realiza un "accept" bloqueante sobre el socket de escucha dado
 *
 * entra: el socket sobre el que se esta escuchando
 * sale: el descriptor de socket de conexion obtenido
 *       -1 si no se establece conexion alguna.
 */
int aceptaConexion(int sockEscucha) {
    int ns;
    struct sockaddr_in cliente;  // para recibir la direccion del cliente
    int namelen;

    namelen = sizeof(cliente);
    ns = accept(sockEscucha, (struct sockaddr *)&cliente, (socklen_t *)&namelen);

    return ns;
}

/*
 * recibe
 *      Recibe datos procedentes de un socket conectado.
 *
 * entra: el socket de conexion,
 *      buffer con espacio para los datos a recibir,
 *      el numero de bytes que, como maximo, se leeran del socket
 * sale: el numero de bytes recibidos
 *      -1 si error
 */
int recibe(int sockConexion, void *datos, int numDatos) {
    return recibeXtra(sockConexion, datos, numDatos, 0);
}

/*
 * recibeXtra
 *      Recibe datos con opciones extra. Por ejemplo, se puede usar
 *		MSG_PEEK.
 *
 * entra: el socket de conexion,
 *      buffer con los datos a enviar,
 *      el numero de bytes que, como maximo, se leeran del socket
 *      un entero para especificar opciones adicionales
 * sale: el numero de bytes recibidos
 *      -1 si error
 */
int recibeXtra(int sockConexion, void *datos, int numDatos, int xtra) {
    int rc = recv(sockConexion, datos, numDatos, xtra);

    #ifdef DEBUG_SOCK
        printf("\nReceiving %d bytes of %d requested:\n", rc, numDatos);
        muestraDatos(datos, (rc<numDatos)?rc:numDatos);
    #endif
    return rc;
}

/*
 * envia
 *      Envia datos a traves de un socket conectado
 *
 * entra: el socket de conexion,
 *      buffer donde almacenar lo recibido,
 *      la longitud de mensaje a enviar
 * sale: el numero de bytes enviados
 *      -1 si error
 */
int envia(int sockConexion, void *datos, int nDatos) {
    #ifdef DEBUG_SOCK
        printf("\nSending %d bytes:\n", nDatos);
        muestraDatos(datos, nDatos);
    #endif
    return send(sockConexion, datos, nDatos, 0);
}

/*
 * conecta
 *      Se conecta a un socket abierto por un servidor. La direccion se puede
 *      proporcionar en formato numerico o con nombre que se resuelve via DNS.
 *
 * entra: 
 *      el nombre del servidor, y el puerto de conexiones
 * sale:
 *      el socket obtenido, o -1 si error
 */
int conecta(char *server, int port) {
    
    struct hostent* hp;
    struct sockaddr_in socketInfo;
    int socketfd;
    int result;
    
    /* crea el socket */
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    
    /* informacion del host; si no lo encuentra, error */
    hp = (struct hostent *)gethostbyname((const char *)server);
    if( !hp ) {        
        return -1;
    }    
    
    /* incorpora datos del puerto, dir. del servidor, y tipo de direccion */
    socketInfo.sin_port = htons(port);
    
    bcopy((char *)hp->h_addr, (char *)&socketInfo.sin_addr,hp->h_length);
    socketInfo.sin_family = hp->h_addrtype;
    
    /* conectamos */ 
    result = connect(socketfd,
		(struct sockaddr *)&socketInfo, 
		sizeof(socketInfo));
		
    if( result == -1 ) {        
        return -1;
    }
       
    return socketfd;
}

/*
 * pollIn
 *      Espera a que haya algo que leer de un socket dado, bloqueando
 *      el hilo en el punto desde el que se llama a esta funcion.
 *
 *
 * entra: el socket de conexion,
 *      timeout maximo en milisegundos
 * sale: 0 si algo recibido,
 *       1 si interrupcion recibida o tiempo expirado
 *      -1 si otro tipo de error
 */
int pollIn(int sock_num, int timeout) {
    int rc;
    struct pollfd poll_sock;
    poll_sock.fd = sock_num;
    poll_sock.events = POLLIN;
    rc = poll(&poll_sock, 1, timeout);

    if (rc == 1) return 0;
	if ((rc == -1 && errno == EINTR) || rc == 0) return 1;
	else return -1;
}

/*
 * pollOut
 *      Espera a que se pueda escribir a un socket dado, bloqueando
 *      el hilo en el punto desde el que se llama a esta funcion.
 *
 * entra: el socket de conexion,
 *      timeout maximo en milisegundos
 * sale: 0 si listo para enviar,
 *       1 si interrupcion recibida
 *      -1 si otro tipo de error
 */
int pollOut(int sock_num, int timeout) {
    int rc;
    struct pollfd poll_sock;
    poll_sock.fd = sock_num;
    poll_sock.events = POLLOUT;
    rc = poll(&poll_sock, 1, timeout);

    if (rc == 1) return 0;
	if ((rc == -1 && errno == EINTR) || rc == 0) return 1;
	else return -1;
}

/*
 * muestraDatos
 *      Muestra una serie de bytes en 2 columnas de a 8, una en formato 
 *      hexadecimal y otra como texto
 * entra:
 *      un array con los datos
 *      el numero de bytes a representar dentro de este array
 * sale:
 *      nada, pero imprime cosas por stdout
 */
void muestraDatos(void *datos, int nDatos) {
    int i, j;
    unsigned char *D = (unsigned char *)datos;
    for (i=0; i<nDatos; i += 8) {
        printf("%3.3d ", i);
        for (j=0; j<8; j++) {
            if (i+j<nDatos) printf("%2.2x ", D[i+j]);
            else printf("   ");
        }
        printf(" ");
        for (j=0; j<8; j++) {
            if (i+j<nDatos) printf("%c", (D[i+j]>=32 && D[i+j]<=128)?D[i+j]:'.');
            else printf(" ");
        }
        printf("\n");
    }
}
