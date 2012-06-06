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

/** @file libdclient.h
 * Cabeceras de las primitivas cliente de manejo de ficheros en un sistema distribuido.
 *
 * @author Juan Navarro Moreno
 * @author Antonio Gonzalez Huete
 * @date 10-12-2006
 */

#ifndef __LIBDCLIENT_H_
#define __LIBDCLIENT_H_

#define MAX_LEN_RUTA 255 ///< Maxima longitud de una ruta en el sistema de ficheros.

int r_open(char* ip, int port, char* nombre, char* modo);
int r_ls(char* ip, int port, char* directory);
int r_close(int fd);
long r_seek(int fd, long offset, int pos_origen);
int r_read(int fd, char* buffer, int n_bytes);
int r_write(int fd, char* buffer, int n_bytes);

int d_open(char* ip_y_puerto, char* nombre, char* modo);
int d_ls(char* ip_y_puerto, char* directorio);
int d_close(int fd);
long d_seek(int fd, long offset, int whence);
int d_read(int fd, char *buffer, int number);
int d_write(int fd, char* buffer, int number);

int d_find(char* nombre_fichero, char* ip_y_puerto, char* resultado);
int d_exec(char* ip_y_puerto, char* nombre, char* argv[]);
int d_register(char *ip_y_puerto, char *ip_y_puerto_a_registrar, char **direcciones_obtenidas);
int d_unregister(char *ip_y_puerto, char *ip_y_puerto_a_eliminar);


#endif
