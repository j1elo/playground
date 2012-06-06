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

/** @file libdclient.c
 * Primitivas cliente de manejo de ficheros en un sistema distribuido.
 *
 * @author Juan Navarro Moreno
 * @author Antonio Gonzalez Huete
 * @date 10-12-2006
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/timeb.h>
#include <sys/select.h>

#include <netinet/in.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h> //Acceso a la variable errno.

//Definicion de longitud de buffer, IPs, numero de servidores en cluster, etc.
#include "dserver.h"

#include "sock.h"
#include "libdclient.h"

#define TIMEOUT 3000 ///< Timeout de las conexiones, en milisegundos.

///Valor de errno remoto, devuelto por el servidor.
int r_errno;


/** Abre un fichero localizado en un servidor remoto.
 * @param ip direccion ip del servidor.
 * @param port puerto donde esta escuchando el servidor de ficheros.
 * @param ruta ruta o nombre del fichero que se quiere abrir.
 * @param modo modo de apertura del fichero.
 * @return descriptor para manejar el fichero remoto, -1 si error.
 */
int r_open(char* ip, int port, char* ruta, char* modo) {
	int rc, r_errno, socket;
	char cadena[MAXCADENA] = "";

	//Comprobacion del parametro "ruta"
	if ((ruta == NULL) || (sizeof(ruta)>=MAX_LEN_RUTA)) {
		errno = EINVAL;
		fprintf(stderr, "r_open(): ruta incorrecta\n");
		return -1;
	}

	//Comprobacion del resto de parametros
	if ((ip == NULL) || (modo == NULL)) {
		errno = EINVAL;
		fprintf(stderr, "r_open():  parametros incorrectos\n");
		return -1;
	}

	//Abrimos el socket.
	if ((socket=conecta(ip, port)) == -1) {
		perror("r_open(): conecta()");
		return -1;
	}

	//Enviamos la orden de apertura de listado de directorio
	sprintf(cadena, "r_open %s %s\n", ruta, modo);
	if (envia(socket, cadena, strlen(cadena)) == -1) {
		perror("r_open(): envia()");
		return -1;
	}

	//Recibimos respuesta. Controla que no se exceda el tiempo.
	rc = pollIn(socket, TIMEOUT);
	if (rc == 1) {
		fprintf(stderr, "r_open(): pollIn(): timeout\n");
		return -1;
	}
	rc = 0;
	while (rc == 0) {
		rc = recibe(socket, cadena, MAXCADENA);
		if (rc == -1) {
			perror("r_open(): recibe()");
			return -1;
		}
	}

	//Comprobamos el codigo de respuesta.
	sscanf(cadena, "%d", &r_errno);
	if (r_errno != 0) {
		errno = r_errno;
		return -1;
	}
	return socket;
}


/** Abre un fichero remoto con el resultado de ejecutar el comando "ls" en el servidor.
 * @param ip direccion ip del servidor.
 * @param port puerto donde esta escuchando el servidor de ficheros.
 * @param ruta ruta o nombre del fichero que se quiere listar.
 * @return descriptor para manejar el fichero remoto, -1 si error.
 */
int r_ls(char* ip, int port, char* ruta) {
	int rc, r_errno, socket;
	char cadena[MAXCADENA] = "";

	//Comprobacion del parametro "ruta"
	if ((ruta == NULL) || (sizeof(ruta)>MAX_LEN_RUTA)) {
		errno = EINVAL;
		fprintf(stderr, "r_ls(): ruta incorrecta\n");
		return -1;
	}

	//Abrimos el socket
	if ((socket=conecta(ip, port)) == -1) {
		perror("r_ls(): conecta()");
		return -1;
	}

	//Enviamos la orden de apertura de listado de directorio
	sprintf(cadena, "r_ls %s\n", ruta);
	if (envia(socket, cadena, strlen(cadena)) == -1) {
		perror("r_ls(): envia()");
		close(socket);
		return -1;
	}

	//Recibimos respuesta. Controla que no se exceda el tiempo.
	rc = pollIn(socket, TIMEOUT);
	if (rc == 1) {
		fprintf(stderr, "r_ls(): pollIn(): timeout\n");
		close(socket);
		return -1;
	}
	rc = 0;
	while (rc == 0) {
		rc = recibe(socket, cadena, MAXCADENA);
		if (rc == -1) {
			perror("r_ls(): recibe()");
			close(socket);
			return -1;
		}
	}

	//Comprobamos el codigo de respuesta.
	sscanf(cadena, "%d", &r_errno);
	if (r_errno != 0) {
		errno = r_errno;
		close(socket);
		return -1;
	}
	return socket;
}


/** Cierra un fichero abierto remotamente.
 * @param fd descriptor del fichero remoto (realmente es el descriptor de un socket).
 * @return 0 = OK, -1 = Error.
 */
int r_close(int fd) {
	int rc, r_errno;
	char cadena[MAXCADENA]="";

	sprintf(cadena, "r_close\n");
	if (envia(fd, cadena, strlen(cadena)) == -1) {
		perror("r_close(): envia()");
		return -1;
	}

	//Recibimos respuesta. Controla que no se exceda el tiempo.
	rc = pollIn(fd, TIMEOUT);
	if (rc == 1) {
		fprintf(stderr, "r_seek(): pollIn(): timeout\n");
		return -1;
	}
	rc = 0;
	while (rc == 0) {
		rc = recibe(fd, cadena, MAXCADENA);
		if (rc == -1) {
			perror("r_seek(): recibe()");
			return -1;
		}
	}

	//Comprobamos el codigo de respuesta.
	sscanf(cadena, "%d", &r_errno);
	if (r_errno != 0) {
		errno = r_errno;
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}


/** Posiciona el puntero de un fichero remoto en el offeset especificado.
 * @param fd descriptor del fichero remoto (realmente es el descriptor de un socket).
 * @param offset offset que aplicar a la posicion de referencia en el fichero.
 * @param pos_origen posicion de referencia:
 * @arg SEEK_SET para el principio del fichero.
 * @arg SEEK_CUR para la posicion actual.
 * @arg SEEK_END para el final del fichero.
 * @return posicion actual en el fichero (puede no ser la pedida, en caso de error), -1 si error.
 */
long r_seek(int fd, long offset, int pos_origen) {
	int rc, r_errno;
	long pos_actual;
	char cadena[MAXCADENA] = "";

	//Enviamos la orden de lectura de datos
	sprintf(cadena, "r_seek %ld %d\n", offset, pos_origen);
	if (envia(fd, cadena, strlen(cadena)) == -1) {
		perror("r_seek(): envia()");
		return -1;
	}

	//Recibimos respuesta. Controla que no se exceda el tiempo.
	rc = pollIn(fd, TIMEOUT);
	if (rc == 1) {
		fprintf(stderr, "r_seek(): pollIn(): timeout\n");
		return -1;
	}
	rc = 0;
	while (rc == 0) {
		rc = recibe(fd, cadena, MAXCADENA);
		if (rc == -1) {
			perror("r_seek(): recibe()");
			return -1;
		}
	}

	//Comprobamos el codigo de respuesta.
	sscanf(cadena, "%d %ld", &r_errno, &pos_actual);
	if (r_errno != 0) {
		errno = r_errno;
		return -1;
	}

	return pos_actual;
}


/** Lee de un fichero remoto el numero de bytes solicitado.
 * @param fd descriptor del fichero remoto (realmente es el descriptor de un socket).
 * @param n_bytes numero maximo de bytes a leer del fichero.
 * @param[out] outBuffer buffer donde almacenar los datos leidos.
 * @return numero de bytes leidos, -1 si error.
 */
int r_read(int fd, char *outBuffer, int n_bytes) {
	int rc, bytes_leidos;
	char cadena[MAXCADENA] = "";
	char respuesta[MAXCADENA+MAXDATOS] = "";
	char *pRespuesta;

	//Comprobacion de parametros
	if ((outBuffer == NULL) || (n_bytes > MAXDATOS)) {
		errno = EINVAL;
		return -1;
	}

	//Inicializamos buffer
	bzero(outBuffer, n_bytes);

	//Enviamos la orden de lectura de datos
	sprintf(cadena, "r_read %d\n", n_bytes);
	if (envia(fd, cadena, strlen(cadena)) == -1) {
		perror("r_read(): envia()");
		return -1;
	}

	//Recibimos respuesta. Controla que no se exceda el tiempo.
	rc = pollIn(fd, TIMEOUT);
	if (rc == 1) {
		fprintf(stderr, "r_read(): pollIn(): timeout\n");
		return -1;
	}
	rc = 0;
	while (rc == 0) {
		rc = recibe(fd, respuesta, MAXCADENA+n_bytes);
		if (rc == -1) {
			perror("r_read(): recibe()");
			return -1;
		}
	}

	//Comprobamos el codigo de respuesta.
	sscanf(respuesta, "%d %d", &r_errno, &bytes_leidos);
	if (r_errno != 0) {
		errno = r_errno;
		return -1;
	}

	/* Leemos los datos recibidos.
	 * Busca el segundo espacio, que separa el numero de bytes leidos
	 * de los datos a leer. */
	pRespuesta = strchr(respuesta, ' ');
	pRespuesta++;
	pRespuesta = strchr(pRespuesta, ' ');
	pRespuesta++;

	if (pRespuesta == NULL) {
		//Caso de una respuesta mal formada.
		fprintf(stderr, "respuesta recibida invalida\n");
		errno = EBADMSG;
		return -1;
	}

	//Sabemos por convenio con el servidor, que bytes_leidos <= MAXDATOS asi que no hay problemas.
	if (memcpy(outBuffer, pRespuesta, bytes_leidos) == NULL) {
		perror("r_read(): memcpy()");
		return -1;
	}

	return bytes_leidos;
}


/** Escribe en un fichero remoto el numero de bytes solicitado del buffer especificado.
 * @param fd descriptor del fichero remoto (realmente es el descriptor de un socket).
 * @param buffer buffer donde se encuentran los datos a escribir.
 * @param n_bytes numero de bytes a escribir.
 * @return numero de bytes escritos, -1 si error.
 */
int r_write(int fd, char* buffer, int n_bytes){
	int rc, longCabecera; //Longitud de la cabecera en la respuesta.
	char cadena[MAXCADENA] = "";
	char trama[MAXCADENA+MAXDATOS] = "";

	//Comprobacion de parametros.
	if ((buffer == NULL) || (n_bytes > MAXDATOS)) {
		errno = EINVAL;
		return -1;
	}

	//Enviamos la orden de escritura de datos.
	sprintf(trama, "r_write %d ", n_bytes);
	longCabecera = strlen(trama);
	if (memcpy(trama+longCabecera, buffer, n_bytes) == NULL) {
		perror("r_write(): memcpy()");
		return -1;
	}
	trama[longCabecera+n_bytes] = '\n';

	if (envia(fd, trama, longCabecera+n_bytes+1) == -1) {
		perror("r_write(): envia()");
		return -1;
	}

	//Recibimos respuesta. Controla que no se exceda el tiempo.
	rc = pollIn(fd, TIMEOUT);
	if (rc == 1) {
		fprintf(stderr, "r_write(): pollIn(): timeout\n");
		return -1;
	}
	rc = 0;
	while (rc == 0) {
		rc = recibe(fd, cadena, MAXCADENA);
		if (rc == -1) {
			perror("r_write(): recibe()");
			return -1;
		}
	}

	//Comprobamos el codigo de respuesta.
	sscanf(cadena, "%d", &r_errno);
	if (r_errno != 0) {
		errno = r_errno;
		return -1;
	}

	return n_bytes;
}


/** Abre un fichero localizado en un cluster de servidores remotos.
 * @param ip_y_puerto direccion ip y puerto de un servidor en formato "ip:puerto".
 * @param nombre ruta o nombre del fichero que se quiere abrir.
 * @param modo modo de apertura del fichero.
 * @return descriptor para manejar el fichero remoto, -1 si error.
 */
int d_open(char* ip_y_puerto, char* nombre, char* modo) {
	int fd;
	char buffer[MAX_LEN_RUTA] = "";
	char ipRem[MAX_LEN_RUTA] = "";
	int puertoRem;

	//Comprobacion de parametros.
	if (ip_y_puerto == NULL || nombre == NULL || modo == NULL
	   || sscanf(ip_y_puerto, "%[^:]:%d", ipRem, &puertoRem) != 2) {
		errno = EINVAL;
		return -1;
	}

	//Obtenemos la direccion IP y el puerto de la maquina destino que tiene el fichero.
	if (d_find(nombre, ip_y_puerto, buffer) == -1) {
		//Si el fichero no se encuentra, prepara su creacion en caso de modo escritura.
		if (strcmp(modo, "r+")==0 || modo[0]=='w')
			sscanf(ip_y_puerto, "%[^:]:%d", ipRem, &puertoRem);
		else
			return -1;
	}
	else
		sscanf(buffer, "%[^:]:%d", ipRem, &puertoRem);

	//Abrimos el socket con la maquina destino.
	fd = r_open(ipRem, puertoRem, nombre, modo);
	if (fd == -1) {
      	return -1;
 	}

	return fd;
}


/** Abre un fichero remoto con el resultado de ejecutar el comando "ls" en
 * los servidores del cluster.
 * @param ip_y_puerto direccion ip y puerto de un servidor en formato "ip:puerto".
 * @param directorio ruta o nombre del fichero que se quiere listar.
 * @return descriptor para manejar el fichero remoto, -1 si error.
 */
int d_ls(char* ip_y_puerto, char* directorio) {
	int rc, r_errno, socket;
	char cadena[MAXCADENA] = "";
	char ipRem[MAX_LEN_DIR] = "";
	int puertoRem;

	//Comprobacion de parametros.
	if (ip_y_puerto == NULL || sscanf(ip_y_puerto, "%[^:]:%d", ipRem, &puertoRem) != 2
	   || directorio == NULL || sizeof(directorio)>MAX_LEN_RUTA) {
		errno = EINVAL;
		return -1;
	}

	//Abrimos el socket.
	if ((socket=conecta(ipRem, puertoRem)) == -1) {
		perror("d_ls(): conecta()");
		return -1;
	}

	//Enviamos la orden de listado de directorio remoto.
	sprintf(cadena, "d_ls %s\n", directorio);
	if (envia(socket, cadena, strlen(cadena)) == -1) {
		perror("d_ls(): envia()");
		close(socket);
		return -1;
	}

	//Recibimos respuesta. Controla que no se exceda el tiempo.
	rc = pollIn(socket, TIMEOUT);
	if (rc == 1) {
		fprintf(stderr, "d_ls(): pollIn(): timeout\n");
		close(socket);
		return -1;
	}
	rc = 0;
	while (rc == 0) {
		rc = recibe(socket, cadena, MAXCADENA);
		if (rc == -1) {
			perror("d_ls(): recibe()");
			close(socket);
			return -1;
		}
	}

	//Comprobamos el codigo de respuesta.
	sscanf(cadena, "%d", &r_errno);
	if (r_errno != 0) {
		errno = r_errno;
		close(socket);
		return -1;
	}
	return socket;
}


/** Cierra un fichero abierto remotamente.
 * @param fd descriptor del fichero remoto (realmente es el descriptor de un socket).
 * @return 0 = OK, -1 = Error.
 */
int d_close(int fd) {
	return r_close(fd);
}


/** Posiciona el puntero de un fichero remoto en el offeset especificado.
 * @param fd descriptor del fichero remoto (realmente es el descriptor de un socket).
 * @param offset offset que aplicar a la posicion de referencia en el fichero.
 * @param whence posicion de referencia:
 * @arg SEEK_SET para el principio del fichero.
 * @arg SEEK_CUR para la posicion actual.
 * @arg SEEK_END para el final del fichero.
 * @return posicion actual en el fichero (puede no ser la pedida, en caso de error), -1 si error.
 */
long d_seek(int fd, long offset, int whence) {
	return r_seek(fd, offset, whence);
}


/** Lee de un fichero remoto el numero de bytes solicitado.
 * @param fd descriptor del fichero remoto (realmente es el descriptor de un socket).
 * @param number numero maximo de bytes a leer del fichero.
 * @param[out] buffer buffer donde almacenar los datos leidos.
 * @return numero de bytes leidos, -1 si error.
 */
int d_read(int fd, char *buffer, int number) {
	return r_read(fd, buffer, number);
}


/** Escribe en un fichero remoto el numero de bytes solicitado del buffer especificado.
 * @param fd descriptor del fichero remoto (realmente es el descriptor de un socket).
 * @param buffer buffer donde se encuentran los datos a escribir.
 * @param number numero de bytes a escribir.
 * @return numero de bytes escritos, -1 si error.
 */
int d_write(int fd, char* buffer, int number){
	return r_write(fd, buffer, number);
}


/** Busca un fichero en el cluster de servidores.
 * Devuelve la ip y el puerto de un dserver del cluster donde existe el fichero solicitado.
 * @param nombre_fichero nombre del fichero a buscar.
 * @param ip_y_puerto direccion ip y puerto del servidor en formato "ip:puerto".
 * @param[out] resultado la cadena "ip:puerto" de un servidor que tiene el fichero.
 * @return 0 = OK, -1 = Error.
 */
int d_find(char *nombre_fichero, char *ip_y_puerto, char *resultado) {
	int rc, fd;
	char cadena[MAXCADENA] = "";
	char respuesta[MAXCADENA+MAX_LEN_DIR] = "";
	char ipRem[MAX_LEN_RUTA] = "";
	int puertoRem;

	//Comprobacion de los parametros.
	if (nombre_fichero == NULL || sizeof(nombre_fichero)>MAX_LEN_RUTA
		   || ip_y_puerto == NULL || resultado == NULL
		   || sscanf(ip_y_puerto, "%[^:]:%d", ipRem, &puertoRem) != 2) {

		errno = EINVAL;
		fprintf(stderr, "d_find(): parametros incorrectos\n");
		return -1;
	}

	//Abrimos el socket.
	if ((fd=conecta(ipRem, puertoRem)) == -1) {
		perror("d_find(): conecta()");
		return -1;
	}

	//Enviamos la orden d_find.
	sprintf(cadena, "d_find %s\n", nombre_fichero);
	if (envia(fd, cadena, strlen(cadena)) == -1) {
		close(fd);
		perror("d_find(): envia()");
		return -1;
	}

	//Recibimos respuesta. Controla que no se exceda el tiempo.
	rc = pollIn(fd, TIMEOUT);
	if (rc == 1) {
		close(fd);
		fprintf(stderr, "d_find(): pollIn(): timeout\n");
		return -1;
	}
	rc = 0;
	while (rc == 0) {
		rc = recibe(fd, respuesta, MAXCADENA+MAX_LEN_DIR);
		if (rc == -1) {
			r_close(fd);
			perror("d_find(): recibe()");
			return -1;
		}
	}

	//Comprobamos el codigo de respuesta.
	sscanf(respuesta, "%d", &r_errno);
	if (r_errno != 0) {
		close(fd);
		errno = r_errno;
		return -1;
	}

	//Almacena el resultado devuelto por el servidor.
	sscanf(respuesta, "%*d %s", resultado);
	close(fd);
	return 0;
}


/** Lanza el ejecutable en la maquina destino.
 * @param ip_y_puerto direccion ip y puerto del servidor en formato "ip:puerto".
 * @param nombre nombre del fichero que se quiere ejecutar.
 * @param argv[]  parametros de la orden a ejecutar.
 * @return si todo va bien, la funcion nunca llega a retornar. En caso contrario retorna -1.
 */
int d_exec(char* ip_y_puerto, char* nombre, char* argv[]) {
	int i, rc, fd, r_pid, parada;
	char cadena[MAXCADENA] = "";
	char ipRem[MAX_LEN_DIR] = "";
	int puertoRem;

	//Variables para hacer el select().
	fd_set rfds;
	struct timeval tv;

	//Comprobacion del parametro "directorio"
	if (ip_y_puerto == NULL || sscanf(ip_y_puerto, "%[^:]:%d", ipRem, &puertoRem) != 2
		   || nombre == NULL || sizeof(nombre)>MAX_LEN_RUTA) {
		errno = EINVAL;
		return -1;
	}

	//Abrimos el socket
	if ((fd=conecta(ipRem, puertoRem)) == -1) {
		perror("d_exec(): conecta()");
		return -1;
	}

	//Enviamos la orden de exec
	sprintf(cadena, "r_exec %s", nombre);
	if (argv != NULL) {
		i = 0;
		while (argv[i] != NULL) {
			sprintf(cadena+strlen(cadena), " %s", argv[i]);
			i++;
		}
	}
	sprintf(cadena+strlen(cadena), "\n");

	if (envia(fd, cadena, strlen(cadena)) == -1) {
		r_close(fd);
		perror("d_exec(): envia()");
		return -1;
	}

	//Recibimos respuesta. Controla que no se exceda el tiempo.
	rc = pollIn(fd, TIMEOUT);
	if (rc == 1) {
		close(fd);
		fprintf(stderr, "d_exec(): pollIn(): timeout\n");
		return -1;
	}
	rc = 0;
	while (rc == 0) {
		rc = recibe(fd, cadena, MAXCADENA);
		if (rc == -1) {
			r_close(fd);
			perror("d_exec(): recibe()");
			return -1;
		}
	}

	//Comprobamos el codigo de respuesta.
	sscanf(cadena, "%d %d", &r_errno, &r_pid);
	if (r_errno != 0) {
		close(fd);
		errno = r_errno;
		return -1;
	}


	//Bucle que primero lee entrada del teclado y la manda por el socket,
	// y segundo lee mensajes del socket y los pasa por pantalla.
	//Admite hasta 3 mensajes de 0 bytes en el socket, antes de dar por
	// terminada la conexion.
	parada = 3;
	while (parada != 0) {
		bzero(cadena, MAXCADENA);

		//Atiende entradas de stdin y del socket.
		FD_ZERO(&rfds);
    	FD_SET(0, &rfds);
		FD_SET(fd, &rfds);

		//Espera hasta 5 segundos.
		tv.tv_sec = 5;
		tv.tv_usec = 0;

		rc = select(fd+1, &rfds, NULL, NULL, &tv);

		if (rc == -1)
			perror("select()");
		else if (rc) {
			if (FD_ISSET(0, &rfds)) {
				//Hay datos en stdin;
				rc = read(0, cadena, MAXCADENA);
				envia(fd, cadena, rc);
			}
			else if (FD_ISSET(fd, &rfds)) {
				//Hay datos en el socket.
				rc = recibe(fd, cadena, MAXCADENA);
				if (rc == 0) parada--;
				else printf("%s", cadena);
			}
		}
		else {
			//printf("Han pasado 5 segundos sin datos.\n");
		}
	}

	exit(0);
}


/** Registra un nuevo servidor en el cluster de servidores.
 * @param ip_y_puerto ip y puerto de un servidor del cluster, en formato "ip:puerto".
 * @param ip_y_puerto_a_registrar ip y puerto de la maquina a registrar en el cluster, en formato "ip:puerto" .
 * @param[out] direcciones_obtenidas direcciones de los servidores que forman el cluster, en formato "ip:puerto",
 * excluyendo los propios de la maquina a registrar en el cluster.
 * @return 0 = OK, -1 = Error.
 */
int d_register(char *ip_y_puerto, char *ip_y_puerto_a_registrar, char **direcciones_obtenidas) {
	return 0;
}


/** Elimina un servidor del cluster de servidores.
 * @param ip_y_puerto ip y puerto de un servidor del cluster, en formato "ip:puerto".
 * @param ip_y_puerto_a_eliminar ip y puerto de la maquina a eliminar del cluster, en formato "ip:puerto".
 * @return 0 = OK, -1 = Error.
 */
int d_unregister(char *ip_y_puerto, char *ip_y_puerto_a_eliminar) {
	return 0;
}
