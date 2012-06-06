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

/** @file dserver.h
 * Cabeceras del modulo de servidor distribuido para la creacion de un cluster de servidores.
 *
 * @author Juan Navarro Moreno
 * @author Antonio Gonzalez Huete
 * @date 27-11-2006
 */

#ifndef __DSERVER_H_
#define __DSERVER_H_

#define MAXDATOS 5120 ///< Buffer de 5KB para datos.
#define MAXCADENA 300 ///< 300B para paths, comandos, etc.

#define MAXUSERS 5 ///< Maximos clientes simultaneos para el servidor.
#define MAX_DIRS_CLUSTER 128 ///< Maximo numero de servidores que pueden formar un cluster.
#define MAX_LEN_DIR 32 ///< Longitud de una direccion IP.


/** Tipo de una funcion de servicio de las que usa servidorDistribuido().
 * Se ejecuta en su propio hilo, y debe llamar con cierta frecuencia a
 * pthread_testcancel() para asegurarse de finalizar cuando el servidor
 * se lo solicita.
 * @param sockCliente un socket abierto para comunicarse con el cliente.
 * @param estado el ultimo argumento proporcionado a servidorDistribuido.
 * @return 0 si el servidor debe continuar, -1 si el servidor finaliza.
 */
typedef int(*fserv_t)(int sockCliente, void *estado);


/** Implementa un servidor distribuido.
 * Este servidor intenta unirse a un cluster de servidores ya existentes,
 * si se le proporciona la direccion IP y el puerto de uno de ellos.
 * @param puerto numero de puerto local para escuchar conexiones.
 * @param maxClientes numero maximo de clientes a atender.
 * @param f funcion de servicio a usar.
 * @param estado informacion de estado que pasarle a esa funcion de servicio.
 * @return 0 si salida a peticion del usuario, -1 si salida por otra causa.
 */
int servidorDistribuido(int puerto, int maxClientes, fserv_t f, void *estado);

#endif
