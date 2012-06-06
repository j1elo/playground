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
 */

/** @file dserver-petic.c
 * Modulo de atencion a peticiones del servidor distribuido.
 * Este fichero implementa las funciones que se van a ejecutar
 * en un hilo distinto por cada peticion recibida en el servidor.
 *
 * @author Juan Navarro Moreno
 * @author Antonio Gonzalez Huete
 * @date 27-11-2006
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h> //Uso general.
#include <sys/types.h> //Uso general.
#include <sys/stat.h> //Para stat().
#include <sys/wait.h> //Espera de procesos. Para funcion waitpid().
#include <pthread.h> //Manejo de hilos. Para funciones pthread_*().
#include <signal.h> //Manejo de signals. Para funcion kill().
#include <errno.h> //Acceso a la variable errno.

//Definicion de longitud de buffer, IPs, numero de servidores en cluster, etc.
#include "dserver.h"

#include "sock.h"
#include "semaphore.h"
#include "dserver-petic.h"
#include "libdclient.h"

#define SIN_FICH 0
#define CON_FICH 1
#define LECTURA 0
#define ESCRITURA 1

#define MAXCOM 5 ///< Maximo numero de comandos a ejecutar con fComCluster().
#define TIMEOUT 3000 ///< Timeout de las conexiones, en milisegundos.

// tipos de datos

///Estructura de argumentos para la funcion fComCluster().
typedef struct {
	Semaphore *semArgs; ///< Semaforo de control para no pisar argumentos de la funcion.
	int numServ; ///< Numero de servidor (en las tablas del cluster) al que mandar los comandos.
	char datos[MAXCOM][MAXDATOS]; ///< Array de comandos.
	int ndatos[MAXCOM]; ///< Numero de bytes en cada comando.
	int ncom; ///< Numero de comandos en los arrays.
} fComClusterArgs_t;

extern int gNumServers; ///< Numero de entradas almacenadas en los arrays de informacion del cluster.
extern char gClusterIPs[][MAX_LEN_DIR]; ///< Direcciones IP de los servidores pertenecientes al cluster.
extern int gClusterPuertos[]; ///< Puertos de escucha de los servidores pertenecientes al cluster.

unsigned int gNumCopia = 0; ///< Cuenta el numero de aperturas de un mismo fichero.

Semaphore gSemGlobales; ///< Controla el acceso a variables globales.


//funciones privadas



/** Funcion de atencion de peticiones en el servidor.
 * Se ejecuta en su propio hilo gracias a la funcion servidorGenerico(), la
 * cual tambien se encarga de cerrar el socket de conexion.
 * @param sockCliente el descriptor del socket por el que se comunica con el cliente.
 * @param estado un indicador de estado del servidor.
 * @return 0 si el servidor debe continuar su ejecucion. -1 si debe finalizar.
 */
int peticiones(int sockCliente, void *estado) {
	int rc = 0, error = 0;
	char datos[MAXDATOS] = ""; //Buffer para la recepcion de comandos. OJO con strtok().
	char cadena[MAXCADENA] = ""; //Buffer para el envio de respuestas por errores.
	char nombreFich[MAXCADENA] = ""; //Almacena el nombre del fichero abierto.
	char direccionLocal[MAXCADENA];

	FILE *pFich = NULL;
	unsigned int numCopia = 0;
	int estadoFich = SIN_FICH;
	int modoAcceso = LECTURA;

	char *token = NULL;
	short int banderaFin = 0;

	semaphore_init(&gSemGlobales, 1);

	rc = consigueDireccionLocal(direccionLocal, sockCliente);
    if (rc != 0) {
        fprintf(stderr, "peticiones(): consigueDireccionLocal(): error\n");
    }

	//fprintf(stderr, "Estado interno: \"Sin fichero abierto\"\n");

	//while(no recibas un close)
	while (!banderaFin) {
		//Comprueba si estan intentando matarnos (como hilo ;-).
		pthread_testcancel();

		//Desconecta por timeout de los clientes que no han abierto un fichero.

		if (estadoFich==SIN_FICH) {
			rc = pollIn(sockCliente, TIMEOUT);
			//fprintf(stderr, "rc pollIn = %d\n", rc);
			if (rc == 1) {
				fprintf(stderr, "Timeout: cerrar conexion con cliente por inactividad.\n");
				return 0;
			}
		}

		rc = 0;
		while (rc == 0) {
			rc = recibe(sockCliente, datos, MAXDATOS);
			if (rc == -1) {
				perror("peticiones(): recibe()");
				return 0;
			}
		}

		//Comprueba el ultimo byte de los datos (deberia ser '\n').
		if (datos[rc-1] == '\n') {
			datos[rc-1] = 0;
			rc--;
		}
		else
			fprintf(stderr, "Los datos recibidos no acaban en '\\n'\n");

		//Pruebas con telnet: limpia el "\r".
		//Comentar esta parte cuando se use el cliente apropiado.
		if (datos[rc-1] == '\r') {
			datos[rc-1] = 0;
			rc--;
		}


		fprintf(stderr, "datos recibidos:\n");
		//muestraDatos((void *)datos, rc); //muestra informacion detallada (en hex.)
		fprintf(stderr, "[%s]\n", datos);

		token = strtok(datos, " ");

		//Si en los datos solo habia un "Intro", no hay tokens, pero tampoco es un comando incorrecto.
		if (token!=NULL && estadoFich==SIN_FICH) {
			if (!strcmp(token, "r_open")) {
				char *token2;
				token = strtok(NULL, " "); //Lee el fichero que se debe abrir.
				token2 = strtok(NULL, " "); //Lee el modo de apertura.

				//Averigua el numero de copia que nos toca usar.
				//Guarda el valor en memoria local para que otros hilos no lo cambien.
				numCopia = gNumCopia++;

				pFich = petic_ropen(sockCliente, token, token2, numCopia, &modoAcceso);
				if (pFich == NULL) {
					fprintf(stderr, "Error en petic_ropen()\n");
				}
				else {
					//Guarda el nombre de fichero para poder cerrarlo luego.
					strncpy(nombreFich, token, MAXCADENA-1);
					estadoFich = CON_FICH;
					fprintf(stderr, "Comando r_open\n");
					fprintf(stderr, "Estado interno: \"Con fichero abierto\"\n");
				}
			}
			else if (!strcmp(token, "r_ls")) {
				token = strtok(NULL, " "); //Lee la ruta a listar.
				pFich = petic_rls(sockCliente, token);
				if (pFich == NULL) {
					fprintf(stderr, "Error en petic_rls()\n");
				}
				else {
					//Borra los contenidos de nombreFich para que petic_rclose() no guarde una copia.
					nombreFich[0] = 0;
					estadoFich = CON_FICH;
					modoAcceso = LECTURA; //El fichero resultado de petic_rls() debe ser de solo lectura.
					fprintf(stderr, "Comando r_ls\n");
					fprintf(stderr, "Estado interno: \"Con fichero abierto\"\n");
				}
			}
			else if (!strcmp(token, "r_close")) {
				//El r_close sin fichero abierto, sirve para desconectar.
				if (petic_rclose(sockCliente, NULL, NULL, 0) == -1) {
					//No podemos hacer mucho si petic_rclose() falla.
				}
				banderaFin = 1;
				fprintf(stderr, "Comando r_close\n");
			}
			else if (!strcmp(token, "r_exec")) {
				token = strtok(NULL, " "); //Apunta a la ruta del ejecutable y sus argumentos.
				if (petic_rexec(sockCliente, direccionLocal, token) == -1) {
					fprintf(stderr, "Error en petic_rexec()\n");
				}
				banderaFin = 1;
				fprintf(stderr, "Comando r_exec\n");
			}
			else if (!strcmp(token, "d_find")) {
				token = strtok(NULL, " "); //Lee la ruta al fichero.
				if (petic_dfind(sockCliente, token) == -1) {
					fprintf(stderr, "Error en petic_dfind()\n");
				}
				banderaFin = 1;
				fprintf(stderr, "Comando d_find\n");
			}
			else if (!strcmp(token, "d_ls")) {
				token = strtok(NULL, " "); //Lee la ruta a listar.
				pFich = petic_dls(sockCliente, token);
				if (pFich == NULL) {
					fprintf(stderr, "Error en petic_dls()\n");
				}
				else {
					//Borra los contenidos de nombreFich para que petic_rclose() no guarde una copia.
					nombreFich[0] = 0;
					estadoFich = CON_FICH;
					modoAcceso = LECTURA; //El fichero resultado de petic_dls() debe ser de solo lectura.
					fprintf(stderr, "Comando d_ls\n");
					fprintf(stderr, "Estado interno: \"Con fichero abierto\"\n");
				}
			}
			else if (!strcmp(token, "d_register")) {
				token = strtok(NULL, " "); //Lee el par ip:puerto.
				if (petic_dregister(sockCliente, token) == -1) {
					fprintf(stderr, "Error en petic_dregister()\n");
				}
				banderaFin = 1;
				fprintf(stderr, "Comando d_register\n");
			}
			else if (!strcmp(token, "d_unregister")) {
				token = strtok(NULL, " "); //Lee el par ip:puerto.
				if (petic_dunregister(sockCliente, token) == -1) {
					fprintf(stderr, "Error en petic_dunregister()\n");
				}
				banderaFin = 1;
				fprintf(stderr, "Comando d_unregister\n");
			}
			else if (!strcmp(token, "nueva_lista")) {
				char *token2, *token3, *token4;
				token = strtok(NULL, " "); //Lee el num. de servidores.
				token2 = strtok(NULL, " "); //Lee el num. bytes de IPs.
				token3 = strtok(NULL, " "); //Lee el num. bytes de puertos.
				token4 = token3 + strlen(token3) + 1; //Apunta a los datos.
				if (petic_nuevalista(sockCliente, token, token2, token3, token4) == -1) {
					fprintf(stderr, "Error en petic_nuevalista()\n");
				}
				banderaFin = 1;
				fprintf(stderr, "Comando nueva_lista\n");
			}
			else if (!strcmp(token, "r_seek")
				|| !strcmp(token, "r_read")
				|| !strcmp(token, "r_write")) {
				fprintf(stderr, "El comando '%s' no puede ejecutarse sin un fichero abierto\n", token);
				error = EBADF;
				sprintf(cadena, "%d\n", error);
				envia(sockCliente, cadena, strlen(cadena));
			}
			else {
				fprintf(stderr, "Comando desconocido o incorrecto\n");
				error = EBADF;
				sprintf(cadena, "%d\n", error);
				envia(sockCliente, cadena, strlen(cadena));
			}
		}
		else if (token!=NULL && estadoFich==CON_FICH) {
			if (!strcmp(token, "r_seek")) {
				char *token2;
				token = strtok(NULL, " "); //Lee el offset
				token2 = strtok(NULL, " "); //Lee la posicion de origen

				if (petic_rseek(sockCliente, pFich, token, token2) == -1) {
					fprintf(stderr, "Error en petic_rseek()\n");
				}
				else {
					fprintf(stderr, "Comando r_seek\n");
				}
			}
			else if (!strcmp(token, "r_read")) {
				token = strtok(NULL, " "); //Lee el num. de bytes solicitados.
				if (petic_rread(sockCliente, pFich, token) == -1) {
					fprintf(stderr, "Error en petic_rread()\n");
				}
				else {
					fprintf(stderr, "Comando r_read\n");
				}
			}
			else if (!strcmp(token, "r_write")) {
				char *token2=NULL;
				token = strtok(NULL, " "); //Lee el num. de bytes a escribir.
				if (token)
					token2 = token + strlen(token) + 1; //Apunta a los datos.
				petic_rwrite(sockCliente, pFich, token, (void *)token2, modoAcceso);
				fprintf(stderr, "Comando r_write\n");
			}
			else if (!strcmp(token, "r_close")) {
				if (petic_rclose(sockCliente, pFich, nombreFich, numCopia) == -1) {
					//No podemos hacer mucho si petic_rclose() falla.
				}
				banderaFin = 1;
				fprintf(stderr, "Comando r_close\n");
			}
			else if (!strcmp(token, "r_open")
				|| !strcmp(token, "r_ls")
				|| !strcmp(token, "r_exec")
				|| !strcmp(token, "d_find")
				|| !strcmp(token, "d_ls")
				|| !strcmp(token, "d_register")
				|| !strcmp(token, "d_unregister")) {
				fprintf(stderr, "El comando '%s' no puede ejecutarse cuando hay un fichero abierto\n", token);
				error = EBADF;
				sprintf(cadena, "%d\n", error);
				envia(sockCliente, cadena, strlen(cadena));
			}
			else {
				fprintf(stderr, "Comando desconocido o incorrecto\n");
				error = EBADF;
				sprintf(cadena, "%d\n", error);
				envia(sockCliente, cadena, strlen(cadena));
			}
		}

		//Limpia el buffer de datos
		bzero(datos, MAXDATOS);
	}

	//TODO decidir donde se deberia destruir este semaforo.
	//Aqui no es buen sitio porque podrian haber mas hilos utilizandolo.
	//semaphore_destroy(&gSemGlobales);
	return 0;
}


/** Abre el fichero solicitado.
 * Crea una copia del fichero y abre la copia; de este modo se logra
 * mantener la semantica de sesion, por la que cada hilo tiene acceso
 * a sus propios cambios del fichero, pero no a los de los demas hilos.
 * @param sockCliente socket de comunicacion con el cliente.
 * @param path ruta al fichero que se debe abrir.
 * @param mode modo de apertura del fichero.
 * @param numCopia numero de copia que le corresponde al fichero.
 * @param[out] modoAcceso sale el modo de acceso del fichero.
 * @return el puntero FILE* al fichero abierto. NULL si error.
 * @return Respuesta al cliente: "<errno>\n".
 */
FILE *petic_ropen(int sockCliente, char *path, char *mode, unsigned int numCopia, int *modoAcceso) {
	int error;
	char cadena[MAXCADENA] = "";
	char nombreFich[MAXCADENA] = "";
	struct stat statFich;
	FILE *pFich;

	if ((path==NULL) || (mode==NULL)) {
		error = EINVAL;
		fprintf(stderr, "petic_ropen(): esperaba 2 argumentos: ruta al fichero y modo de apertura\n");
		sprintf(cadena, "%d\n", error);
		envia(sockCliente, cadena, strlen(cadena));
		return NULL;
	}

	if (strcmp(mode, "r+")==0 || mode[0]=='w') *modoAcceso=ESCRITURA;
	else *modoAcceso = LECTURA;

	/* Comprueba si path apunta a un fichero existente.
	 * Si no existe, y el modo es != r, entonces asigna el nombreFich
	 * para su posterior creacion. Si el modo es r o r+, hay un eror. */
	if (stat(path, &statFich) == -1) {
		if ((errno == ENOENT) && (mode[0] != 'r')) {
			sprintf(nombreFich, "%s.%.3u", path, numCopia);
		}
		else {
			error = errno;
			perror("petic_ropen(): stat()");
			sprintf(cadena, "%d\n", error);
			envia(sockCliente, cadena, strlen(cadena));
			return NULL;
		}
	}
	//Comprueba que sea un fichero normal.
	else {
		if (S_ISREG(statFich.st_mode)) {
			fprintf(stderr, "Es un fichero normal: voy a tratarlo\n");
		}
		else if (S_ISDIR(statFich.st_mode)) {
			//Es un directorio; posible ampliacion: que lo trate petic_rls().
			error = EINVAL;
			fprintf(stderr, "petic_ropen(): no implementada la apertura de directorios\n");
			sprintf(cadena, "%d\n", error);
			envia(sockCliente, cadena, strlen(cadena));
			return NULL;
		}
		else {
			fprintf(stderr, "No es ni un fichero normal, ni un directorio! Error.\n");
			error = EINVAL;
			fprintf(stderr, "petic_ropen(): la ruta no apunta a un fichero valido\n");
			sprintf(cadena, "%d\n", error);
			envia(sockCliente, cadena, strlen(cadena));
			return NULL;
		}

		/* Crea una copia del fichero para su apertura: "cp nombre nombre.001"
		 * Esta sintaxis aparece de nuevo en petic_rclose(). */
		sprintf(nombreFich, "%s.%.3u", path, numCopia);
		sprintf(cadena, "cp %s %s\n", path, nombreFich);
		if (system(cadena) == -1) {
			error = errno;
			perror("petic_ropen(): system()");
			sprintf(cadena, "%d\n", error);
			envia(sockCliente, cadena, strlen(cadena));
			return NULL;
		}
	}

	pFich = fopen(nombreFich, mode);
	if (pFich == NULL) {
		error = errno;
		perror("petic_ropen(): fopen()");
		sprintf(cadena, "%d\n", error);
		envia(sockCliente, cadena, strlen(cadena));
		return NULL;
	}

	error = 0;
	sprintf(cadena, "%d\n", error);
	envia(sockCliente, cadena, strlen(cadena));
	return pFich;
}


/** Crea un listado con los contenidos de un directorio.
 * Abre un nuevo fichero temporal para guardar el resultado de hacer "ls <path>".
 * El nombre del fichero temporal lo elige el sistema, y este sera borrado cuando
 * se le aplique un fclose().
 * @param sockCliente socket de comunicacion con el cliente.
 * @param path ruta al directorio que se debe listar.
 * @return el puntero FILE* al fichero abierto. NULL si error.
 * @return Respuesta al cliente: "<errno>\n".
 */
FILE *petic_rls(int sockCliente, char *path) {
	int rc, error, fd;
	char cadena[MAXCADENA] = "";
	char datos[MAXDATOS] = "";
	FILE *pFich;

	if (path == NULL) {
		error = EINVAL;
		fprintf(stderr, "petic_rls(): esperaba 1 argumento: directorio que listar\n");
		sprintf(cadena, "%d\n", error);
		envia(sockCliente, cadena, strlen(cadena));
		return NULL;
	}

	pFich = tmpfile();
	if (pFich == NULL) {
		error = errno;
		perror("petic_rls(): tmpfile()");
		sprintf(cadena, "%d\n", error);
		envia(sockCliente, cadena, strlen(cadena));
		return NULL;
	}

	fd = lsopen(path);
	if (fd == -1) {
		fclose(pFich);
		error = errno;
		perror("petic_rls(): lsopen()");
		sprintf(cadena, "%d\n", error);
		envia(sockCliente, cadena, strlen(cadena));
		return NULL;
	}

	//En el descriptor de fichero fd tenemos los resultados del 'ls'.
	while ((rc = read(fd, datos, MAXDATOS - 1)) > 0) {
		datos[rc] = '\0';
		fprintf(pFich, "%s", datos);
	}
	close(fd);

	if (rc == -1) {
		fclose(pFich);
		error = errno;
		perror("petic_rls(): read()");
		sprintf(cadena, "%d\n", error);
		envia(sockCliente, cadena, strlen(cadena));
		return NULL;
	}

	rewind(pFich); //Situa el puntero de acceso al principio para posteriores lecturas.
	error = 0;
	sprintf(cadena, "%d\n", error);
	envia(sockCliente, cadena, strlen(cadena));
	return pFich;
}


/** Cierra el fichero solicitado.
 * En caso de ser un fichero creado en un petic_ropen() anterior, nombreFich debe
 * contener un nombre de fichero valido, y se procede a sustituirlo por
 * el fichero pFich.numCopia y cerrar este ultimo.
 * @param sockCliente socket de comunicacion con el cliente.
 * @param pFich puntero al fichero "copia".
 * @param nombreFich nombre del fichero original.
 * @param numCopia numero de copia correspondiente al fichero "copia".
 * @return 0 = OK, -1 = Error.
 * @return Respuesta al cliente: "<errno>\n".
 */
int petic_rclose(int sockCliente, FILE *pFich, char *nombreFich, unsigned int numCopia) {
	int error;
	char cadena[MAXCADENA] = "";

	if (pFich!=NULL) fclose(pFich);
	if (nombreFich!=NULL && strlen(nombreFich)>0) {
		if (remove(nombreFich) == -1)
			perror("petic_rclose(): remove()");

		//Procede a renombrar el archivo temporal.
		sprintf(cadena, "%s.%.3u", nombreFich, numCopia);
		//Por igualdad de estado si antes existia un archivo igual, ahora no habra error.
		rename(cadena, nombreFich);
	}

	error = 0;
	sprintf(cadena, "%d\n", error);
	envia(sockCliente, cadena, strlen(cadena));
	return 0;
}


/** Modifica el indicador de posicion de fichero.
 * @param sockCliente socket de comunicacion con el cliente.
 * @param pFich puntero al fichero.
 * @param argOffset argumento indicando el offset a desplazar a partir del origen.
 * @param argOrigen argumento indicando la posicion de origen del desplazamiento.
 * Debe ser uno de los tres valores definidos en stdio.h:
 * @arg SEEK_SET para el inicio del fichero.
 * @arg SEEK_CUR para la posicion actual del fichero.
 * @arg SEEK_END para el final del fichero.
 * @return 0 = OK, -1 = Error.
 * @return Respuesta al cliente: "<errno> <posicion actual>\n".
 */
int petic_rseek(int sockCliente, FILE *pFich, char *argOffset, char *argOrigen) {
	int error;
	char cadena[MAXCADENA];
	long offset, limitOffset;
	int origen;
	unsigned long actual;

	//Comprueba que haya fichero abierto.
	if (fileno(pFich) == -1) {
		//Segun el bucle principal de peticiones(), nunca deberia entrar aqui.
		fprintf(stderr, "petic_rseek(): no hay fichero abierto!\n");
		return -1;
	}
	else if ((argOffset==NULL) || (argOrigen==NULL)
			|| (!sscanf(argOffset, "%ld", &offset)) || (!sscanf(argOrigen, "%d", &origen))) {
		error = EINVAL;
		fprintf(stderr, "petic_rseek(): esperaba 2 argumentos: offset de avance y posicion de origen\n");
		sprintf(cadena, "%d %ld\n", error, ftell(pFich));
		envia(sockCliente, cadena, strlen(cadena));
		return -1;
	}

	//Comprueba que no se avance (o retroceda) mas del limite del fichero.
	if (offset < 0) {
		limitOffset = ftell(pFich) * -1;
		if (offset < limitOffset)
			offset = limitOffset;
	}
	else {
		actual = ftell(pFich);
		fseek(pFich, 0L, SEEK_END);
		limitOffset = ftell(pFich) - actual;
		fseek(pFich, actual, SEEK_SET); //Recupera la posicion original.
		if (offset > limitOffset)
			offset = limitOffset;
	}

	//Desplaza el puntero de fichero.
	if (fseek(pFich, offset, origen) == -1) {
		error = errno;
		perror("petic_rseek(): fseek()");
		sprintf(cadena, "%d %ld\n", error, ftell(pFich));
		envia(sockCliente, cadena, strlen(cadena));
		return -1;
	}

	error = 0;
	sprintf(cadena, "%d %ld\n", error, ftell(pFich));
	envia(sockCliente, cadena, strlen(cadena));
	return 0;
}


/** Lee y envia los contenidos del fichero abierto.
 * @param sockCliente socket de comunicacion con el cliente.
 * @param pFich puntero al fichero.
 * @param argLong argumento indicando el numero de bytes a leer.
 * @return 0 = OK, -1 = Error.
 * @return Respuesta al cliente: "<errno> <longitud> <datos>".
 */
int petic_rread(int sockCliente, FILE *pFich, char *argLong) {
	int error;
	char cadena[MAXCADENA] = "";
	char datos[MAXDATOS] = "";
	char respuesta[MAXCADENA+MAXDATOS];
	int longCabecera; //Longitud de la cabecera en la respuesta.
	unsigned long actual=0, maxLongitud=0, longitud=0;

	//Comprueba que haya fichero abierto.
	if (fileno(pFich) == -1) {
		//Segun el bucle principal de peticiones(), nunca deberia entrar aqui.
		fprintf(stderr, "petic_rread(): no hay fichero abierto!\n");
		return -1;
	}
	else if ((argLong == NULL) || (!sscanf(argLong, "%lu", &longitud))) {
		error = EINVAL;
		fprintf(stderr, "petic_rread(): esperaba 1 argumento: bytes que leer\n");
		sprintf(cadena, "%d\n", error);
		envia(sockCliente, cadena, strlen(cadena));
		return -1;
	}

	//Comprueba que no se pidan mas bytes de los que hay disponibles.
	actual = ftell(pFich);
	fseek(pFich, 0L, SEEK_END);
	maxLongitud = ftell(pFich) - actual;
	fseek(pFich, actual, SEEK_SET); //Recupera la posicion original.
	if (longitud > maxLongitud)
		longitud = maxLongitud;

	//Sabemos por convenio con el cliente, que longitud <= MAXDATOS asi que no hay problemas.
	if (fread((void *)datos, longitud, 1, pFich) == 0) {
		//fread() no distingue entre fin-de-fichero y error, asi que lo comprobamos nosotros.
		if (ferror(pFich)) {
			error = errno;
			perror("petic_rread(): fread()");
			sprintf(cadena, "%d 0\n", error);
			envia(sockCliente, cadena, strlen(cadena));
			return -1;
		}
	}

	//Prepara y envia los datos.
	sprintf(respuesta, "0 %lu ", longitud);
	longCabecera = strlen(respuesta);
	memcpy((void *)(respuesta+longCabecera), (void *)datos, longitud);

	if (envia(sockCliente, respuesta, longCabecera+longitud) == -1) {
		fprintf(stderr, "Error al enviar datos\n");
		return -1;
	}

	return 0;
}


/** Escribe datos en el fichero.
 * @param sockCliente socket de comunicacion con el cliente.
 * @param pFich puntero al fichero.
 * @param argLong argumento indicando el numero de bytes a escribir.
 * @param datos buffer conteniendo los argLong datos a escribir en el fichero.
 * @param modoAcceso flag indicando si el acceso al fichero es de solo lectura o de lectura y escritura.
 * @return 0 = OK, -1 = Error.
 * @return Respuesta al cliente: "<errno>\n".
 */
int petic_rwrite(int sockCliente, FILE *pFich, char *argLong, void *datos, int modoAcceso) {
	int error;
	char cadena[MAXCADENA];
	unsigned long longitud;

	//Comprueba que haya fichero abierto.
	if (fileno(pFich) == -1) {
		//Segun el bucle principal de peticiones(), nunca deberia entrar aqui.
		fprintf(stderr, "petic_rwrite(): no hay fichero abierto!\n");
		return -1;
	}
	else if ((argLong == NULL) || (datos == NULL)
			|| (!sscanf(argLong, "%lu", &longitud))) {
		error = EINVAL;
		fprintf(stderr, "petic_rwrite(): esperaba 2 argumentos: bytes que escribir y datos\n");
		sprintf(cadena, "%d\n", error);
		envia(sockCliente, cadena, strlen(cadena));
		return -1;
	}

	//Comprueba que sea modo de escritura.
	if (modoAcceso != ESCRITURA) {
		error = EPERM;
		fprintf(stderr, "petic_rwrite(): el fichero no esta abierto en modo escritura\n");
		sprintf(cadena, "%d\n", error);
		envia(sockCliente, cadena, strlen(cadena));
		return -1;
	}

	//Escribe los datos recibidos en el fichero abierto con anterioridad.
	if (fwrite(datos, longitud, 1, pFich) == 0) {
		error = errno;
		perror("petic_rwrite(): fwrite()");
		sprintf(cadena, "%d\n", error);
		envia(sockCliente, cadena, strlen(cadena));
		return -1;
	}

	//Prepara y envia la respuesta.
	error = 0;
	sprintf(cadena, "%d\n", error);
	envia(sockCliente, cadena, strlen(cadena));
	return 0;
}


/** Lanza el ejecutable en la maquina destino.
 * Una vez devuelto el valor de retorno, el socket usado se convierte en
 * la entrada y salida del ejecutable.
 * @return Respuesta al cliente: "<errno> <pid_hijo>\n".
 */
int petic_rexec(int sockCliente, char *direccionLocal, char *comando) {
	int rc;
	int pidHijo;
	char cadena[MAXCADENA] = "";

	if (comando == NULL) {
		fprintf(stderr, "petic_rexec(): esperaba 1 argumento: comando a ejecutar\n");
		sprintf(cadena, "%d -1\n", EINVAL);
		envia(sockCliente, cadena, strlen(cadena));
		return -1;
	}

	rc = ejecutaHijo(comando, direccionLocal, sockCliente);
	if (rc <= 0) {
		fprintf(stderr, "petic_rexec(): ejecutaHijo(): error\n");
		sprintf(cadena, "%d -1\n", EINVAL);
		envia(sockCliente, cadena, strlen(cadena));
		return -1;
	}
	pidHijo = rc;

	//Devuelve el PID del hijo y cierra la conexion; el hijo sigue conectado.
	sprintf(cadena, "0 %d\n", pidHijo);
	envia(sockCliente, cadena, strlen(cadena));

	//Desbloquea el hijo.
	kill(pidHijo, SIGUSR1);

	//Espera hasta que el hijo acabe.
	if (waitpid(pidHijo, &rc, 0) == -1)
		perror("petic_rexec(): waitpid()");

	if (WIFEXITED(rc)) {
		fprintf(stderr, "petic_rexec(): exited, status=%d\n", WEXITSTATUS(rc));
	} else if (WIFSIGNALED(rc)) {
		fprintf(stderr, "petic_rexec(): killed by signal %d\n", WTERMSIG(rc));
	}
	return 0;
}


/** Busca un fichero en el cluster de servidores.
 * Devuelve la ip y el puerto de un dserver del cluster donde existe el fichero solicitado.
 * @return 0 = OK, -1 = Error.
 * @return Respuesta al cliente: "<errno> <ip:puerto>\n".
 */
int petic_dfind(int sockCliente, char *path) {
	int i, numServers, error, responder;
	char cadena[MAXCADENA] = "";
	int *retornos;

	Semaphore semArgs; // para evitar pisar los args. de la f. de mensajes.

	semaphore_down(&gSemGlobales);
	pthread_t hilos[gNumServers];
	numServers = gNumServers;
	semaphore_up(&gSemGlobales);

	pthread_attr_t atribs;
	fComClusterArgs_t args;

	//Envia peticiones r_open a todos los servidores.
	// Y sus correspondientes r_close.
	args.ncom = 2;
	sprintf(args.datos[0], "r_open %s r\n", path);
	args.ndatos[0] = strlen(args.datos[0]);
	sprintf(args.datos[1], "r_close\n");
	args.ndatos[1] = strlen(args.datos[1]);

	// para crear nuevos hilos accesibles mediante pthread_join().
 	pthread_attr_init(&atribs);
 	pthread_attr_setdetachstate(&atribs, PTHREAD_CREATE_JOINABLE);

	args.semArgs = &semArgs; //Para que la f. de mensajes lo pueda liberar.
	semaphore_init(&semArgs, 1);

	for (i=0; i<numServers; i++) {
		semaphore_down(&semArgs);
		args.numServ = i;
		pthread_create(hilos+i, &atribs, fComCluster, (void *)&args);
	}

	//Recibe las respuestas de los servidores.
	responder = 1;
	for (i=0; i<numServers; i++) {
		if (pthread_join(hilos[i], (void **)&retornos) != 0) {
			error = errno;
			perror("petic_dfind(): pthread_join()");
			sprintf(cadena, "%d\n", error);
			envia(sockCliente, cadena, strlen(cadena));
			return -1;
		}

		if (responder && retornos!=NULL && retornos[0]==0) {
			fprintf(stderr, "petic_dfind(): encontrado! en servidor=%d\n", i);
			//El servidor i tiene el fichero solicitado.
			error = 0;
			sprintf(cadena, "%d %s:%d\n", error, gClusterIPs[i], gClusterPuertos[i]);
			envia(sockCliente, cadena, strlen(cadena));
			responder = 0;
		}
	}

	//Si no se ha respondido es porque el fichero no se ha encontrado.
	if (responder == 1) {
		sprintf(cadena, "%d\n", ENOENT);
		envia(sockCliente, cadena, strlen(cadena));
	}

 	return 0;
}


/** Crea un listado con los contenidos de un directorio.
 * Similar a petic_rls(), pero realiza sucesivos petic_rls() sobre
 * todos los servidores que componen el cluster.
 * @param sockCliente socket de comunicacion con el cliente.
 * @param path ruta al directorio que se debe listar.
 * @return el puntero FILE* al fichero abierto. NULL si error.
 * @return Respuesta al cliente: "<errno>\n".
 */
FILE *petic_dls(int sockCliente, char *path) {
	int i, rc, fd, numServers, error;
	char cadena[MAXCADENA] = "";
	char datos[MAXDATOS] = "";
	FILE *pFich;

	numServers = gNumServers;

	if (path == NULL) {
		error = EINVAL;
		fprintf(stderr, "petic_dls(): esperaba 1 argumento: directorio que listar\n");
		sprintf(cadena, "%d\n", error);
		envia(sockCliente, cadena, strlen(cadena));
		return NULL;
	}

	pFich = tmpfile();

	if (pFich == NULL) {
		error = errno;
		perror("petic_dls(): tmpfile()");
		sprintf(cadena, "%d\n", error);
		envia(sockCliente, cadena, strlen(cadena));
		return NULL;
	}

	//Para cada servidor del cluster.
	for (i=0; i<numServers; i++) {
		//1. envia una peticion r_ls.
		fd = r_ls(gClusterIPs[i], gClusterPuertos[i], path);
		if (fd == -1) {
			//Si falla un servidor, se pasa al siguiente.
			perror("petic_dls(): petic_rls()");
			continue;
		}

		//2. realiza los r_read que sean necesarios hasta acabar el fichero remoto.
		while ((rc = r_read(fd, datos, MAXDATOS - 1)) > 0) {
			//3. escribe en el fichero local los resultados del r_read.
			datos[rc] = '\0';
			fprintf(pFich, "%s", datos);
		}

		fprintf(pFich, "\n");

		//4. envia un r_close.
		r_close(fd);
	}

	rewind(pFich); //Situa el puntero de acceso al principio para posteriores lecturas.
	error = 0;
	sprintf(cadena, "%d\n", error);
	envia(sockCliente, cadena, strlen(cadena));
	return pFich;
}


/** Notifica a todas las maquinas del cluster de que hay un nuevo miembro.
 * El nuevo dserver es el que tiene la IP y puerto suministrados como argumento.
 * Les envia a todos los servidores, en paralelo (usando un hilo de ejecucion
 * para cada uno), una nueva lista de servidores registrados en el cluster. Para
 * ello emplea la funcion fComCluster() y el comando "nueva_lista".
 * @param sockCliente socket de comunicacion con el cliente.
 * @param serv servidor nuevo en el cluster.
 * @return 0 = OK, -1 = Error.
 * @return Respuesta al cliente: servidores registrados (sin incluir al nuevo)
 * y sus IPs y puertos: "<errno> <N> <ip1:puerto1> ... <ipN:puertoN>\n".
 */
int petic_dregister(int sockCliente, char *serv) {
	int i;
	char datos[MAXDATOS] = "";
	char ipServ[MAX_LEN_DIR] = "";
	int puertoServ = 0;
	int numServers = gNumServers; //Para usar el valor de gNumServers sin necesitar semaforo.

	Semaphore semArgs; // para evitar pisar los args. de la f. de mensajes.
	pthread_t hilos[gNumServers];
	pthread_attr_t atribs;
	fComClusterArgs_t args;
	char *p; //Posicion dentro de la cadena de datos del mensaje remoto.
	int longIPs, longPuertos; //Tamanio en bytes de los respectivos arrays.

	sscanf(serv, "%[^:]:%d", ipServ, &puertoServ);
	semaphore_down(&gSemGlobales);

	//Miro que el serv no este ya en la lista.
	for (i=0; i<numServers; i++) {
		if (!strncmp(gClusterIPs[i], ipServ, MAX_LEN_DIR) && (gClusterPuertos[i]==puertoServ)) {
			//Respondo al serv.
			sprintf(datos, "%d 0\n", EINVAL);
			envia(sockCliente, datos, strlen(datos));
			return -1;
		}
	}

	//Miro si el servidor cabe en la lista.
	if (numServers >= MAX_DIRS_CLUSTER) {
		sprintf(datos, "%d 0\n", EINVAL);
		envia(sockCliente, datos, strlen(datos));
		return -1;
	}

	//1- Respondo al serv con la lista de servidores en el cluster.
	sprintf(datos, "%d %d", 0, numServers);
	for (i=0; i<numServers; i++) {
		sprintf(datos+strlen(datos), " %s:%d", gClusterIPs[i], gClusterPuertos[i]);
	}
	sprintf(datos+strlen(datos), "\n");

	//fprintf(stderr, "La respuesta al d_register es:\n[%s]\n", datos);
	envia(sockCliente, datos, strlen(datos));


	//2- Inserto el serv en la lista de servidores local.
	strncpy(gClusterIPs[numServers], ipServ, MAX_LEN_DIR);
	gClusterPuertos[numServers] = puertoServ;
	gNumServers++;
	numServers++;

	//3- Les envio a todos la nueva lista de servidores.
	longIPs = numServers*MAX_LEN_DIR*sizeof(char);
	longPuertos = numServers*sizeof(int);

	args.ncom = 1;
	args.ndatos[0] = 0;
	p = args.datos[0];
	sprintf(p, "nueva_lista %d %d %d ", numServers, longIPs, longPuertos);
	args.ndatos[0] += strlen(p);
	p += strlen(p);
	memcpy((void *)p, (void *)gClusterIPs, longIPs);
	p += longIPs;
	memcpy((void *)p, (void *)gClusterPuertos, longPuertos);

	//Levanta el semaforo tan pronto como deja de ser necesario.
	semaphore_up(&gSemGlobales);

	args.ndatos[0] += longIPs + longPuertos;
	sprintf(args.datos[0]+args.ndatos[0], "\n");
	args.ndatos[0]++;

	// para crear nuevos hilos accesibles (joinable).
 	pthread_attr_init(&atribs);
	pthread_attr_setdetachstate(&atribs, PTHREAD_CREATE_JOINABLE);

	args.semArgs = &semArgs; //Para que la f. de mensajes lo pueda liberar.
	semaphore_init(&semArgs, 1);

	for (i=0; i<numServers; i++) {
		semaphore_down(&semArgs);
		args.numServ = i;
		pthread_create(hilos+i, &atribs, fComCluster, (void *)&args);
	}

	/* Los hilos necesitan acceder a la variable semArgs; si el hilo padre termina antes que
	 * los hijos, semArgs sera destruida y ocurrira un fallo de segmentacion.
	 * Por eso el hilo padre espera a que los hijos terminen. */
	for (i=0; i<numServers; i++)
		pthread_join(hilos[i], NULL);

	return 0;
}


/** Elimina un servidor del cluster.
 * Elimina del cluster el servidor con la IP y puerto especificados.
 * Lo contrario a petic_dregister().
 * Les envia a todos los servidores, en paralelo (usando un hilo de ejecucion
 * para cada uno), una nueva lista de servidores registrados en el cluster. Para
 * ello emplea la funcion fComCluster() y el comando "nueva_lista".
 * @return 0 = OK, -1 = Error.
 * @return Respuesta al cliente: "<errno>\n".
 */
int petic_dunregister(int sockCliente, char *serv) {
	int i;
	char cadena[MAXCADENA] = "";
	char ipServ[MAX_LEN_DIR] = "";
	int puertoServ;
	int numServers = gNumServers;

	Semaphore semArgs; // para evitar pisar los args. de la f. de mensajes.
	pthread_t hilos[gNumServers];
	pthread_attr_t atribs;
	fComClusterArgs_t args;
	char *p; //Posicion dentro de la cadena de datos del mensaje remoto.
	int longIPs, longPuertos; //Tamanio en bytes de los respectivos arrays.

	sscanf(serv, "%[^:]:%d", ipServ, &puertoServ);
	semaphore_down(&gSemGlobales);

	//Elimina el serv de la lista de servidores local.
	for (i=0; i<numServers; i++) {
		if (!strncmp(gClusterIPs[i], ipServ, MAX_LEN_DIR) && (gClusterPuertos[i]==puertoServ))
			break;
	}
	if (i >= numServers) {
		//El serv no existe en la lista de servidores. Le respondo.
		sprintf(cadena, "%d\n", EINVAL);
		envia(sockCliente, cadena, strlen(cadena));
		return -1;
	}

	//Respondo al serv.
	sprintf(cadena, "%d\n", 0);
	envia(sockCliente, cadena, strlen(cadena));

	//El servidor a eliminar esta en la posicion i.
	if (i == numServers-1) {
		gNumServers--;
		numServers--;
	}
	else {
		strncpy(gClusterIPs[i], gClusterIPs[numServers-1], MAX_LEN_DIR);
		gClusterPuertos[i] = gClusterPuertos[numServers-1];
		gNumServers--;
		numServers--;
	}

	//Les envia a todos la nueva lista de servidores.
	longIPs = numServers*MAX_LEN_DIR*sizeof(char);
	longPuertos = numServers*sizeof(int);

	args.ncom = 1;
	args.ndatos[0] = 0;
	p = args.datos[0];
	sprintf(p, "nueva_lista %d %d %d ", numServers, longIPs, longPuertos);
	args.ndatos[0] += strlen(p);
	p += strlen(p);
	memcpy((void *)p, (void *)gClusterIPs, longIPs);
	p += longIPs;
	memcpy((void *)p, (void *)gClusterPuertos, longPuertos);

	semaphore_up(&gSemGlobales);

	args.ndatos[0] += longIPs + longPuertos;
	sprintf(args.datos[0]+args.ndatos[0], "\n");
	args.ndatos[0]++;

	// para crear nuevos hilos accesibles (joinable)
 	pthread_attr_init(&atribs);
	pthread_attr_setdetachstate(&atribs, PTHREAD_CREATE_JOINABLE);

	args.semArgs = &semArgs; //Para que la f. de mensajes lo pueda liberar.
	semaphore_init(&semArgs, 1);

	for (i=0; i<numServers; i++) {
		semaphore_down(&semArgs);
		args.numServ = i;
		pthread_create(hilos+i, &atribs, fComCluster, (void *)&args);
	}

	/* Los hilos necesitan acceder a la variable semArgs; si el hilo padre termina antes que
	 * los hijos, semArgs sera destruida y ocurrira un fallo de segmentacion.
	 * Por eso el hilo padre espera a que los hijos terminen. */
	for (i=0; i<numServers; i++)
		pthread_join(hilos[i], NULL);

	return 0;
}


/** Actualiza la lista de servidores registrados en el cluster.
 * Sustituye la lista local de servidores registrados en el cluster,
 * con la nueva lista recibida como argumento.
 * @return 0 = OK, -1 = Error.
 * @return Respuesta al cliente: "<errno>\n".
 */
int petic_nuevalista(int sockCliente, char *numServers, char *nbIPs, char *nbPuertos, char *datos) {
	int num, longIPs, longPuertos;
	int i;
	char cadena[MAXCADENA];

	if ((numServers==NULL) || (nbIPs==NULL) || (nbPuertos==NULL)
			|| (!sscanf(numServers, "%d", &num))
			|| (!sscanf(nbIPs, "%d", &longIPs))
			|| (!sscanf(nbPuertos, "%d", &longPuertos))) {
		fprintf(stderr, "petic_nuevalista(): esperaba 4 argumentos: numero de servidores, bytes de IPs, bytes de Puertos y datos\n");
		return -1;
	}

	//Copia los datos recibidos a memoria local.
	semaphore_down(&gSemGlobales);
	gNumServers = num;
	memcpy((void *)gClusterIPs, (void *)datos, longIPs);
	memcpy((void *)gClusterPuertos, (void *)datos+longIPs, longPuertos);

	fprintf(stderr, "El cluster queda formado asi:\n");
	for (i=0; i<gNumServers; i++) {
		fprintf(stderr, "%s:%d\n", gClusterIPs[i], gClusterPuertos[i]);
	}

	semaphore_up(&gSemGlobales);

	//Envia la respuesta.
	sprintf(cadena, "%d\n", 0);
	envia(sockCliente, cadena, strlen(cadena));
	return 0;
}


/** Envia una serie de comandos a un servidor del cluster.
 * La idea de esta funcion es ser ejecutada en paralelo usando varios hilos de
 * ejecucion, y poder mandar asi un conjunto de comandos a todos los servidores
 * del cluster simultaneamente.
 */
void *fComCluster(void *argumentos) {
	int i, rc, r_errno, socket;
	char cadena[MAXCADENA];
	fComClusterArgs_t args;
	int retornos[MAXCOM];

	memcpy(&args, argumentos, sizeof(fComClusterArgs_t));
	semaphore_up(args.semArgs);

	//printf("%d: conectar a %s:%d\n", args.numServ, gClusterIPs[args.numServ], gClusterPuertos[args.numServ]);

	socket = conecta(gClusterIPs[args.numServ], gClusterPuertos[args.numServ]);
	if (socket == -1) {
		perror("fComCluster(): conecta()");
		pthread_exit(NULL);
	}

	//Envia los comandos dados en los argumentos.
	r_errno = 0;
	for (i=0; i<args.ncom; i++) {
		if (envia(socket, args.datos[i], args.ndatos[i]) == -1) {
			perror("fComCluster(): envia()");
			pthread_exit(NULL);
		}

		//Recibimos respuesta. Controla que no se exceda el tiempo.
		rc = pollIn(socket, TIMEOUT);
		if (rc == 1) {
			pthread_exit(NULL);
		}

		rc = 0;
		while (rc == 0) {
			rc = recibe(socket, cadena, MAXCADENA);
			if (rc == -1) {
				perror("fComCluster(): recibe()");
				pthread_exit(NULL);
			}
		}

		//Guardamos el codigo de respuesta.
		sscanf(cadena, "%d", &r_errno);
		retornos[i] = r_errno;
	}

	close(socket);
	pthread_exit((void *)retornos);
}
