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
 *  2005-09-30: added DEBUG_SOCK flag and muestraDatos
 */

/*
 * sock.h
 *      cabeceras de una sencilla libreria de funciones para facilitar la
 *      el uso de sockets TCP
 */

#ifndef __SOCK_H_
#define __SOCK_H_

// si DEBUG_SOCK esta definido, todos los 'send' y 'receive' mostraran
// lo que envian o reciben por stdout, usando muestraDatos
//#define DEBUG_SOCK

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/timeb.h>
#include <sys/unistd.h>
#include <time.h>
#include <stdio.h>
#include <sys/poll.h>

/*
 * preparaEscucha
 *      Prepara un "socket" de servidor
 *      para recibir conexiones por un puerto dado
 *
 * entra: el identificador del puerto, num. max. de conexiones pendientes
 * sale: el descriptor de socket de escucha obtenido
 *       -1 si no se establece conexion
 */
int preparaEscucha(int puerto, int maxClientes);

/*
 * aceptaConexion
 *      Realiza un "accept" bloqueante sobre el socket de escucha dado
 *
 * entra: el socket sobre el que se esta escuchando
 * sale: el descriptor de socket de conexion obtenido
 *       -1 si no se establece conexion alguna.
 */
int aceptaConexion(int sockEscucha);

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
int recibe(int sockConexion, void *datos, int numDatos);

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
int recibeXtra(int sockConexion, void *datos, int numDatos, int xtra);

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
int envia(int sockConexion, void *datos, int nDatos);
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
int conecta(char *server, int port);

/*
 * pollIn
 *      Espera a que haya algo que leer de un socket dado, bloqueando
 *      el hilo en el punto desde el que se llama a esta funcion.
 *
 *
 * entra: el socket de conexion,
 *      timeout maximo en milisegundos
 * sale: 0 si algo recibido,
 *       1 si interrupcion recibida
 *      -1 si otro tipo de error
 */
int pollIn(int sock_num, int timeout);

/*
 * pollOut
 *      Espera a que se pueda escribir a un socket dado, bloqueando
 *      el hilo en el punto desde el que se llama a esta funcion.
 *
 * entra: el socket de conexion,
 *      timeout maximo en milisegundos
 * sale: 0 si algo recibido,
 *       1 si interrupcion recibida
 *      -1 si otro tipo de error
 */
int pollOut(int sock_num, int timeout);

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
void muestraDatos(void *datos, int nDatos);

#endif // __SOCK_H_
