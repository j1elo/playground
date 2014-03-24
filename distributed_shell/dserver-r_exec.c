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
 *  2004-11-09: Manuel Freire Moran
 */

/** @file dserver-r_exec.c
 * Servidor que entiende peticiones del tipo 'r_exec'.
 * Servidor de la P1 que solamente entiende la peticion
 * <pre>
 *    'r_exec <nombre de programa y argumentos>\n'
 *    a la que responde con
 *    '<errno> <pid_programa>\n'
 * </pre>
 *
 * NOTA:
 * <pre>
 *    si se ejecuta 'telnet' contra este servidor, hay que tener en cuenta que
 *    telnet envia '\r\n' como final de linea, y no '\n'. Esto hace que muchas
 *    aplicaciones (pej, '/bin/bash') no reconozcan bien
 *    las lineas.
 *    En el caso de 'bash', se puede postponer ';' a todos los comandos.
 * </pre>
 *
 * @author Juan Navarro Moreno
 * @author Antonio Gonzalez Huete
 * @date 27-11-2006
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/wait.h>

#include "dserver.h"
#include "semaphore.h"
#include "sock.h"

#define MAX_BUFF 1024
#define MAX_CADENA 1024

// prototipos de las funciones que se deberian incluir en el 'dserver' (o en librerias)
void manejaSigUsr1(int numSignal);
int ejecutaHijo(char *cadena, char *direccionLocal, int fd);
int consigueDireccionLocal(char *direccionLocal, int s);
//int ejecuta(char *buffer, int *nDatos, char *direccionLocal, int id, int sockConexion);

/*
 * manejaSigUsr1
 *   Lanzada para desbloquear un hijo creado con ejecutaHijo. No hace nada:
 *   el hijo se desbloquea el solo tras receibir el SIGUSR1 y llamar a esto.
 */
void manejaSigUsr1(int numSignal) {
        // no hace nada
}

/*
 * ejecutaHijo
 *    Lanza un hijo que ejecuta el proceso solicitado con los argumentos
 *    suministrados. Pasa al hijo la ip_y_puerto_propios por si las necesita
 *    en la variable de entorno DSERVER. El hijo se queda bloqueado esperando
 *    SIGUSR1 para permitir enviar informacion por el socket antes de lanzarlo
 *    de verdad.
 *    Conecta stderr, stdin, y stdout con el socket suministrado.
 * entra:
 *    el nombre del archivo a ejecutar, con todos sus argumentos
 *    (se asume que los espacios dividen entre argumentos).
 *    la direccionLocal que se debe usar para la variable de entorno DSERVER del hijo
 *    el descriptor del socket que se usara para stdin, stdout y stderr del proceso
 * sale:
 *    el descriptor (se debe cerrar con 'close()' una vez se acaba de operar con el)
 *    -1 si error
 */
int ejecutaHijo(char *cadena, char *direccionLocal, int fd) {
  int childpid;
  int rc;
  int i;
  char s[MAX_CADENA];
  char *pos;
  char *argv[MAX_CADENA];
  char *envv[MAX_CADENA];
  char envv0[MAX_CADENA];

  // Consigue argumentos (si, uso 'strtok*', pero ojo: strtok_r y sobre una copia)
  strncpy(s, cadena, MAX_CADENA-1);
  argv[0] = strtok_r(s, " ", &pos);
  for (i=1; (argv[i] = strtok_r(NULL, " ", &pos)) != NULL; i++) /* nada */;

  // prepara variables de entorno
  sprintf(envv0, "DSERVER=%s", direccionLocal);
  envv[0] = envv0;
  envv[1] = NULL;

  // Depuracion - eliminar si se considera apropiado
  /*
  for (i=0; argv[i] != NULL; i++) {
	  printf("\targv[%d] = '%s'\n", i, argv[i]);
  }
  */

  // Realiza el fork
  childpid = fork ();
  switch (childpid) {
    case 0:
      // prepara un manejador de 'SIGUSR1'
      signal(SIGUSR1, manejaSigUsr1);

      // espera a recibir un 'SIGUSR1' para continuar
      pause();

      // conecta la e/s del proceso hijo al socket
      close(0); // in
      dup2(fd, 0);
      close(1); // out
      dup2(fd, 1);
      close(2); // err- tambien lo manda por el mismo socket
      dup2(fd, 2);
      // ejecuta la instruccion
      rc = execve(argv[0], argv, envv);
      exit (rc); // (no se alcanza)
    case -1:
      return -1;
    default:
      break;
  }

  // devuelve el pid del hijo
  return childpid;
}

/*
 * consigueDireccionLocal
 *    devuelve el par ip:puerto del extremo local de un socket. Ojo, porque si
 *    la conexion se establece via 'localhost', devuelve 127.0.0.1 como IP.
 * entra:
 *    un puntero a cadena (ya reservada) donde escribir el resultado (max 24 caracteres)
 *    el socket de donde sacar la direccion
 * sale:
 *    0 si todo bien
 *   -1 si error
 */
int consigueDireccionLocal(char *direccionLocal, int s) {
    struct sockaddr_in self;
    int len = sizeof(struct sockaddr_in);
    long ip;
    unsigned char *ipc = (unsigned char *)&ip;

    if (getsockname(s, (struct sockaddr*)&self, (socklen_t*)&len) != 0) {
        return -1;
    }

    ip = self.sin_addr.s_addr;
    sprintf(direccionLocal, "%d.%d.%d.%d:%d",
        ipc[0], ipc[1], ipc[2], ipc[3], ntohs(self.sin_port));

    return 0;
}


/*
 * ejecuta
 * 	  lanza el programa especificado, con sus argumentos, como hijo.
 *    Si tiene exito, devuelve el PID del hijo, y espera a que acabe.
 *    Toda la entrada y salida del hijo va a traves del socket suministrado.
 * entra:
 *    buffer con la solicitud (se usa tambien para enviar resultados)
 *    puntero a entero con el numero de datos recibidos / a enviar
 *    puntero a descriptor de archivo
 *    la direccionLocal (ip:puerto) a usar para la v. de entorno DSERVER del hijo
 *    identificador del hilo (para incluir en mensajes)
 *    socket de conexion (a usar para responder / conectar al hijo)
 * sale:
 *    0 si todo bien (y se debe finalizar el hilo, sin cerrar nada ni enviar nada)
 *   -1 si error (y se debe enviar un mensaje de error y repetir bucle ppal)
 */
 /*
int ejecuta(char *buffer, int *nDatos, char *direccionLocal, int id, int sockConexion) {
    char args[MAX_BUFF];
    int pidHijo;
    int rc;

    if (sscanf(buffer, "r_exec %[^\r\n]", args) != 1) {
		printf("%d> peticion erronea\n", id);
		*nDatos = sprintf(buffer, "%d -1\n", EINVAL);
		return -1;
	}

	// ejecuta
	rc = ejecutaHijo(args, direccionLocal, sockConexion);
	if (rc <= 0) {
		printf("%d> error lanzando hijo: %d\n", id, rc);
		*nDatos = sprintf(buffer, "%d -1\n", EINVAL);
		return -1;
	}
    pidHijo = rc;

	// devuelve el PID del hijo y cierra la conexion; el hijo sigue conectado
	printf("%d> he sido padre! se llama '%d'\n", id, pidHijo);
	*nDatos = sprintf(buffer, "0 %d\n", pidHijo);
	envia(sockConexion, buffer, *nDatos);

    // desbloquea el hijo
    kill(pidHijo, SIGUSR1);

	// espera hasta que el hijo acabe
	if (waitpid(pidHijo, &rc, 0) == -1) perror("Error waiting");
	if (WIFEXITED(rc)) {
		printf("%d> mi hijo se va, diciendo '%d'\n", id, WEXITSTATUS(rc));
	}
	else if (WIFSIGNALED(rc)) {
		printf("%d> han echado a mi hijo, con un '%d'\n", id, WTERMSIG(rc));
	}

	// finaliza ejecucion del hilo
	return 0;
}
*/

/*
 * atiendePeticion
 *      Maneja una peticion.
 *
 * entra:
 *	    el socket de conexion a atender (ya conectado)
 *      datos adicionales (en este caso, ninguno).
 *
 * sale: nada
 */
 /*
int atiendePeticion(int sockConexion, void *estado) {
    int rc;
    int id;
    char buffer[MAX_BUFF];
    int nDatos;
    char direccionLocal[MAX_CADENA];

    id = sockConexion;

    rc = consigueDireccionLocal(direccionLocal, sockConexion);
    if (rc != 0) {
        printf("%d> error obteniendo dir. local: %s\n", id, strerror(rc));
    }
    else {
        printf("%d> dir. local es '%s'\n", id, direccionLocal);
    }

    printf("%d> levantado!\n", id);

    while (1) {

		rc = recibe(sockConexion, buffer, MAX_BUFF);
		if (rc == -1) break;

		if (strstr(buffer, "r_exec ") != NULL) {
            if (ejecuta(buffer, &nDatos, direccionLocal, id, sockConexion) == 0) {
				return 0;
			}
        }
        else {
            printf("%d> Peticion no reconocida: '%s'\n", id, buffer);
            nDatos = sprintf(buffer, "%d\n", EPROTO);
        }

		rc = envia(sockConexion, buffer, nDatos);
		if (rc == -1) break;
    }

    return 0;
}
*/

/*
 *
 * main
 *
 */
 /*
int main(int argc, char **argv) {

    int puerto;

    // comprueba paraetros
    if (argc != 2) {
        printf("Esperaba 1 argumento: puerto en el que escuchar\n");
        return -1;
    } else if (sscanf(argv[1], "%d", &puerto) != 1) {
        printf("Argumento erroneo: esperaba un numero\n");
    }

    // lanza el servidor
    servidorGenerico(puerto, 3, atiendePeticion, NULL);

    // cuando finaliza el servidor, fin del programa
    printf("El servidor ha finalizado\n");
    return 0;
}
*/
