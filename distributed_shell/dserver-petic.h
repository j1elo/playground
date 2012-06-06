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

/** @file dserver-petic.h
 * Cabeceras del modulo de atencion a peticiones del servidor distribuido.
 *
 * @author Juan Navarro Moreno
 * @author Antonio Gonzalez Huete
 * @date 27-11-2006
 */

#ifndef __DSERVER_PETIC_H_
#define __DSERVER_PETIC_H_

int peticiones(int sockCliente, void *estado);

FILE *petic_ropen(int sockCliente, char *path, char *mode, unsigned int numCopia, int *modoAcceso);
FILE *petic_rls(int sockCliente, char *path);
int petic_rclose(int sockCliente, FILE *pFich, char *nombreFich, unsigned int numCopia);
int petic_rseek(int sockCliente, FILE *pFich, char *argOffset, char *argOrigen);
int petic_rread(int sockCliente, FILE *pFich, char *argLong);
int petic_rwrite(int sockCliente, FILE *pFich, char *argLong, void *datos, int modoAcceso);

int petic_rexec();
int petic_dfind(int sockCliente, char* path);
FILE *petic_dls(int sockCliente, char *path);
int petic_dregister(int sockCliente, char* serv);
int petic_dunregister(int sockCliente, char* serv);

int petic_nuevalista(int sockCliente, char *numServers, char *nbIPs, char *nbPuertos, char *datos);
void *fComCluster(void *argumentos);


/* Funciones auxiliares, extraidas de diversos ficheros. */
//dserver-ls.c
int lsopen(char *nombre);

//dserver-r_exec.c
int consigueDireccionLocal(char *direccionLocal, int s);
void manejaSigUsr1(int numSignal);
int ejecutaHijo(char *cadena, char *direccionLocal, int fd);


#endif
