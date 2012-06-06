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
 *  2004-08-04: Manuel Freire Moran
 */

/** @file dserver.c
 * Modulo de servidor distribuido para la creacion de un cluster de servidores.
 * Implementa un servidor multihilo. Cada conexion recibida por el servidor
 * se atendera por la funcion correspondiente en un hilo distinto.
 *
 * @author Juan Navarro Moreno
 * @author Antonio Gonzalez Huete
 * @date 27-11-2006
 *
 * TODO controlar que no se intente registrar un servidor que ya esta registrado.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/timeb.h>
#include <netinet/in.h>

#include <time.h>
#include <ctype.h>
#include <signal.h>

#include <pthread.h> //Manejo de hilos. Para funciones pthread_*().
#include <errno.h> //Acceso a la variable errno.

#include "sock.h"
#include "semaphore.h"
#include "dserver.h"
#include "dserver-petic.h"

// tipos de datos
typedef struct {
	int sockCliente;
	fserv_t f;
	void *estado;
	Semaphore *semParametros;
	pthread_t *hilo;
	int numHilo; //Indica el numero de hilo en la tabla de hilos del servidor. Util para pruebas.
} argsCliente_t;

// globales
int r_errno; ///< Valor de errno remoto, devuelto por el servidor.
int gNumServers=0; ///< Numero de entradas almacenadas en los arrays de informacion del cluster.
char gClusterIPs[MAX_DIRS_CLUSTER][MAX_LEN_DIR]; ///< Direcciones IP de los servidores pertenecientes al cluster.
int gClusterPuertos[MAX_DIRS_CLUSTER]; ///< Puertos de escucha de los servidores pertenecientes al cluster.

char gIPRem[MAX_LEN_DIR]; ///< IP del servidor al que solicitar ingreso en el cluster.
int gPuertoRem; ///< Puerto del servidor al que solicitar ingreso en el cluster.

int gBanderaFin = 0; ///< Bandera para indicar solicitud de fin.
int gSocketEscucha; ///< Socket a cerrar si se recibe sigint.
Semaphore gSemClientes; ///< Semaforo para limitar el # de usuarios.

// prototipos internos
void finaliza(int numSignal);
void *funcionServicio(void *argumentos);
void cierraSocket(void *numSocket);
int inicializaTablaHilos(pthread_t **hilos, int maxHilos);
int buscaHiloLibre(pthread_t *hilos, int maxHilos);
void cancelaHilos(pthread_t *hilos, int maxHilos);

int clusterRegistro(char *ipRem, int puertoRem);
int clusterBaja();


int main(int argc, char **argv) {
	int puertoLoc=0;
	int posParam;

	//Comprueba argumentos.
	if (argc>1 && strcmp(argv[1], "-v")==0) {
		argc--;
		posParam = 2;
	}
	else {
		fclose(stderr);
		posParam = 1;
	}

	if (argc==2 && sscanf(argv[posParam], "%d", &puertoLoc)==1) {
		fprintf(stdout, "Iniciar como primer servidor del cluster, puerto %d\n", puertoLoc);
		bzero(gIPRem, MAX_LEN_DIR);
		gPuertoRem = 0;
		servidorDistribuido(puertoLoc, MAXUSERS, peticiones, NULL);
	}
	else if (argc==3
		&& sscanf(argv[posParam], "%d", &puertoLoc)==1
		&& sscanf(argv[posParam+1], "%[^:]:%d", gIPRem, &gPuertoRem)==2) {
		fprintf(stdout, "Nuevo sevidor en el cluster, puerto %d, insercion en servidor %s:%d\n",
			puertoLoc, gIPRem, gPuertoRem);
		servidorDistribuido(puertoLoc, MAXUSERS, peticiones, NULL);
	}
	else {
		fprintf(stdout, "Uso:\n   d_server [-v] <puerto de escucha> [<ip:puerto de otro d_server>]\n");
		fprintf(stdout, "Opciones:\n   -v verbose: activa los mensajes detallados de estado y error (stderr).\n");
		return -1;
	}

	return 0;
}


// funciones internas

/*
 * finaliza (interna)
 *      pone una bandera global a "true", y rompe posibles puntos de bloqueo
 */
void finaliza(int numSignal) {
	fprintf(stderr, "-- finalizando (signal %d recibido) --\n", numSignal);
	gBanderaFin = 1;
	//Se da de baja en el cluster
	if (clusterBaja(gIPRem, gPuertoRem) == -1)
		perror("finaliza(): clusterBaja()");
	close(gSocketEscucha);
	semaphore_up(&gSemClientes);
}


/*
 * cierraSocket (interna)
 *      cierra un socket
 */
void cierraSocket(void *numSocket) {
	close(*((int *)numSocket));
}


/*
 * funcionServicio (interna)
 *      usada para envolver la funcion de atencion a peticiones y liberarla de
 *      tener que cerrar el socket, proporcionarla sus argumentos, y tal.
 */
void *funcionServicio(void *argumentos) {

	argsCliente_t args;
	sigset_t mascaraSigInt;

	// copia argumentos a mem. local, evitando que otro hilo los sobreescriba
	memcpy(&args, argumentos, sizeof(argsCliente_t));
	semaphore_up(args.semParametros);

	// ignora interrupciones por SIGINT en este hilo y prepara la limpieza
	sigemptyset(&mascaraSigInt);
	sigaddset(&mascaraSigInt, SIGINT);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_sigmask(SIG_BLOCK, &mascaraSigInt, NULL);
	pthread_cleanup_push(cierraSocket, (void *)&args.sockCliente);

	// lanza la funcion de atencion a clientes
	if (args.f(args.sockCliente, args.estado) != 0) {
		gBanderaFin = 1;
	}

	// finaliza la ejecucion del hilo y libera el puerto
	pthread_cleanup_pop(1);
	*(args.hilo) = 0;
	fprintf(stderr, "> hilo #%d finalizado. Queda espacio para %d clientes.\n",
		args.numHilo, gSemClientes.v+2);
	semaphore_up(&gSemClientes);
	pthread_exit(NULL);
}


/*
 * inicializaTablaHilos (interna)
 *      inicializa la tabla de hilos, poniendolos todos a 0
 */
int inicializaTablaHilos(pthread_t **hilos, int maxHilos) {
	int i;
	*hilos = (pthread_t *)malloc(sizeof(pthread_t) * maxHilos);
	if (*hilos == NULL) return -1;
	for (i=0; i<maxHilos; i++) {
		(*hilos)[i] = 0;
	}
	return 0;
}


/*
 * buscaHiloLibre (interna)
 *       devuelve la siguiente posicion libre en la tabla de hilos
 */
int buscaHiloLibre(pthread_t *hilos, int maxHilos) {
	int i;
	for (i=0; i<maxHilos; i++) {
		if (hilos[i] == 0) return i;
	}
	return -1;
}


/*
 * cancelaHilos (interna)
 *       cancela todos los hilos no-0 del array pasado como argumento
 */
void cancelaHilos(pthread_t *hilos, int maxHilos) {
	int i;
	for (i=0; i<maxHilos; i++) {
		if (hilos[i] != 0) {
			fprintf(stderr, "cancelando hilo #%d... ", i);
			fflush(stdout);
			pthread_cancel(hilos[i]);
			fprintf(stderr, "OK\n");
		}
	}
}


/* entradas: ip y puerto remotos. */
int clusterRegistro(char *ipRem, int puertoRem) {
	int rc, socket;
	int puertoLoc = 0;
	char ipLoc[MAX_LEN_DIR] = "";

	int i;
	char cadena[MAXCADENA] = "";
	char *token; //puntero auxiliar.

	if (consigueDireccionLocal(cadena, gSocketEscucha) == -1) {
		fprintf(stderr, "clusterRegistro(): consigueDireccionLocal(): error\n");
		return -1;
	}

	// parsea el par direccionIP:puerto
	token = strchr(cadena, ':');
	*token = '\0';
	sscanf(++token, "%d", &puertoLoc);
	strncpy(ipLoc, cadena, MAX_LEN_DIR);

	if ((ipRem[0]==0 || puertoRem==0)) {
		//Nos auto-insertamos en la tabla de servidores.
		strncpy(gClusterIPs[0], ipLoc, MAX_LEN_DIR);
		gClusterPuertos[0] = puertoLoc;
		gNumServers = 1;
		return 0;
	}

	if ((socket=conecta(ipRem, puertoRem)) == -1) {
		perror("clusterRegistro(): conecta()");
		return -1;
	}

	//Enviamos la solicitud de registro en el cluster de servidores.
	sprintf(cadena, "d_register %s:%d\n", ipLoc, puertoLoc);
	if (envia(socket, cadena, strlen(cadena)) == -1) {
		perror("clusterRegistro(): envia()");
		return -1;
	}

	//Recibimos respuesta del servidor
	rc = 0;
	while (rc == 0) {
		rc = recibe(socket, cadena, MAXCADENA);
		if (rc == -1) {
			perror("clusterRegistro(): recibe()");
			return -1;
		}
	}

	//Comprobamos el codigo de respuesta.
	//De paso preparamos el troceado de la respuesta.
	token = strtok(cadena, " ");
	sscanf(token, "%d", &r_errno);
	if (r_errno != 0) {
		errno = r_errno;
		return -1;
	}

	//Leemos el numero de servidores y sus pares direccion:puerto.
	bzero(gClusterIPs, MAX_DIRS_CLUSTER*MAX_LEN_DIR*sizeof(char));
	bzero(gClusterPuertos, MAX_DIRS_CLUSTER*sizeof(int));
	token = strtok(NULL, " ");
	sscanf(token, "%d", &gNumServers);
	token = strtok(NULL, " ");
	for (i=0; i<gNumServers && token!=NULL; i++) {
		sscanf(token, "%[^:]:%d", gClusterIPs[i], &(gClusterPuertos[i]));
		token = strtok(NULL, " ");
	}

	//Nos auto-insertamos en la tabla de servidores.
	strncpy(gClusterIPs[gNumServers], ipLoc, MAX_LEN_DIR);
	gClusterPuertos[gNumServers] = puertoLoc;
	gNumServers++;

	return 0;
}


int clusterBaja() {
	int rc, r_errno, socket, i;
	int puertoLoc, puertoRem;
	char ipLoc[MAX_LEN_DIR], ipRem[MAX_LEN_DIR];
	char cadena[MAXCADENA] = "";
	char *token; //puntero auxiliar.

	//Si solo hy un servidor, somos nosotros y no hay que hacer nada.
	if (gNumServers <= 1)
		return 0;

	if (consigueDireccionLocal(ipLoc, gSocketEscucha) == -1) {
		fprintf(stderr, "clusterBaja(): Error al conseguir la direccion local\n");
		return -1;
	}

	// parsea el par direccionIP:puerto
	token = strchr(ipLoc, ':');
	*token = '\0';
	sscanf(++token, "%d", &puertoLoc);

	//printf("iploc: %s, puertoloc: %d\n", ipLoc, puertoLoc);

	//Escoge al primer servidor de la lista para mandarle nuestra baja del cluster.
	i = 0;
	if (!strncmp(gClusterIPs[i], ipLoc, MAX_LEN_DIR) && gClusterPuertos[i]==puertoLoc)
		i++; //Que no seamos nosotros mismos!
	strncpy(ipRem, gClusterIPs[i], MAX_LEN_DIR);
	puertoRem = gClusterPuertos[i];

	if ((socket=conecta(ipRem, puertoRem)) == -1) {
		perror("clusterBaja(): conecta()");
		return -1;
	}

	//Enviamos la solicitud de baja en el cluster de servidores.
	sprintf(cadena, "d_unregister %s:%d\n", ipLoc, puertoLoc);
	if (envia(socket, cadena, strlen(cadena)) == -1) {
		perror("clusterBaja(): envia()");
		return -1;
	}

	//Recibimos respuesta del servidor
	rc = 0;
	while (rc == 0) {
		rc = recibe(socket, cadena, MAXCADENA);
		if (rc == -1) {
			perror("clusterBaja(): recibe()");
			return -1;
		}
	}

	//Comprobamos el codigo de respuesta.
	sscanf(cadena, "%d", &r_errno);
	if (r_errno != 0) {
		errno = r_errno;
		return -1;
	}
	return 0;
}


// funciones exportadas

/** Implementa un servidor distribuido.
 * Este servidor intenta unirse a un cluster de servidores ya existentes,
 * si se le proporciona la direccion IP y el puerto de uno de ellos.
 * @param puerto numero de puerto local para escuchar conexiones.
 * @param maxClientes numero maximo de clientes a atender.
 * @param f funcion de servicio a usar.
 * @param estado informacion de estado que pasarle a esa funcion de servicio.
 * @return 0 si salida a peticion del usuario, -1 si salida por otra causa.
 */
int servidorDistribuido(int puerto, int maxClientes, fserv_t f, void *estado) {

	Semaphore semParametros; // para evitar pisar los args. de la f. de atencion
	argsCliente_t argsCliente;

	int	socketEscucha;  // los clientes solicitan establecer conexion aqui
    int rc;

	pthread_t *hilos; // solo para conexiones ya establecidas; 0 == libre
    pthread_attr_t atribs;

	// reserva espacio para tabla de hilos
	if (inicializaTablaHilos(&hilos, maxClientes) == -1) {
		perror("> imposible crear tabla de hilos, abortando");
		return -1;
	}

    // abre puerto (y usa una cola de espera de, a lo sumo, maxClientes)
    rc = preparaEscucha(puerto, maxClientes);
    if (rc == -1) {
        perror("servidorDistribuido(): preparaEscucha()");
        return -1;
    } else {
        printf("Escuchando en puerto %d...\n", puerto);
        socketEscucha = rc;
    }

    // se prepara para recibir Ctrl+C y salir limpiamente
	gSocketEscucha = socketEscucha;
    signal(SIGINT, finaliza);

    // Se registra en el cluster de servidores.
    if (clusterRegistro(gIPRem, gPuertoRem) == -1) {
    	fprintf(stderr, "Error al registrarse en el cluster de servidores\n");
    	close(socketEscucha);
    	return -1;
    }

    // prepara un semaforo para controlar num. hilos y otro para parametros
    semaphore_init(&gSemClientes, maxClientes);
    semaphore_init(&semParametros, 1);

	// para crear nuevos hilos independientes (detached)
 	pthread_attr_init(&atribs);
	pthread_attr_setdetachstate(&atribs, PTHREAD_CREATE_DETACHED);

	// argumentos de funcionServicio (solo cambian el socket y el num. hilo)
	argsCliente.semParametros = &semParametros;
	argsCliente.estado = estado;
	argsCliente.f = f;

    // bucle principal: recibe peticiones y crea hilos para procesarlas
    while(!gBanderaFin) {

		// se bloquea si hay muchos clientes (puede ser interrumpido aqui)
        semaphore_down(&gSemClientes);
		if (gBanderaFin) break;

        // recibe una nueva conexion
        rc = aceptaConexion(socketEscucha);
        if (gBanderaFin || rc == -1) {
            if (gBanderaFin) fprintf(stderr, "> interrumpido, abortando\n");
			else perror("> error en 'accept', abortando");
            break;
        }
        fprintf(stderr, "> conectado! quedan %d conexiones libres\n", gSemClientes.v);

		// prepara parametros y crea el hilo
        semaphore_down(&semParametros);
        argsCliente.sockCliente = rc;
        argsCliente.numHilo = buscaHiloLibre(hilos, maxClientes);
    	fprintf(stderr, ">> preparado para lanzar hilo #%d\n", argsCliente.numHilo);
		argsCliente.hilo = hilos + argsCliente.numHilo;

        rc = pthread_create(argsCliente.hilo,
			&atribs,
			funcionServicio,
			(void *)&argsCliente);

        if (rc == -1) {
            perror("> error creando thread, abortando");
            break;
        }
		else {
			fprintf(stderr, ">> hilo lanzado\n");
		}
    }

	// limpieza (cada hilo limpia lo suyo cuando acaba o es cancelado)
	printf(">> saliendo... ");
	fflush(stdout);
	cancelaHilos(hilos, maxClientes);
	free(hilos);
	semaphore_destroy(&semParametros);
	semaphore_destroy(&gSemClientes);

	printf("OK\n");
	if (gBanderaFin) return 0;
    else return -1;
}
