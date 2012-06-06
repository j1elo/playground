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
 
/*
 * s_echo.c
 *      servidor de 'echo' que hace uso de la libreria 'serv', 'semaphore'
 *      y 'sock'
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "serv.h"
#include "semaphore.h"
#include "sock.h"

#define RECV_TIMEOUT 5000
#define MAX_BUFF 1024

/*
 * atiendeEcho
 *      Maneja una peticion de "echo" por parte de un cliente
 *
 * entra: 
 *	    el socket de conexion a atender (ya conectado)
 *      datos adicionales (en este caso, ninguno).
 *
 * sale: nada
 */
int atiendeEcho(int sockConexion, void *estado) {
    int rc;
    int id;
    char buffer[MAX_BUFF];

    id = sockConexion;
    printf("%d> levantado!\n", id);
  
    // recibe datos (hasta que se reciba un mensaje con 0 datos)
    while (1) {
		
		// comprueba cond. de final y espera recibir algo
		pthread_testcancel();        
		rc = pollIn(sockConexion, RECV_TIMEOUT);
        if (rc != 0) {
            if (rc == 1)
                printf("%d> nada en los últimos %d ms\n", id, RECV_TIMEOUT);
            else if (rc == -1)
                printf("%d> interrupción recibida!\n", id);

            continue;
        }

		// hay algo para recibir - lo recibe
        rc = recibe(sockConexion, buffer, MAX_BUFF);
        if (rc == -1) {
            sprintf(buffer, "%d> error recibiendo:", id);
            perror(buffer);
            break;
        } else if (rc == 0) {
            printf("%d> rompiendo conexion: recibidos 0 bytes\n", id);
            break;
        }
		pthread_testcancel();

        // lo muestra
        printf("%d> recibidos %d bytes:\n", id, rc);
        muestraDatos((void *)buffer, rc);

        // los envia de vuelta
        rc = envia(sockConexion, buffer, rc);
        if (rc == -1) {
            sprintf(buffer, "%d> error enviando, abortando: ", id);
            perror(buffer);
            break;
        }
    }

	return 0;
}

/*
 *
 * main
 *
 */
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
    servidorGenerico(puerto, 2, atiendeEcho, NULL);

    // cuando finaliza el servidor, fin del programa
    printf("El servidor ha finalizado\n");
    return 0;
}
