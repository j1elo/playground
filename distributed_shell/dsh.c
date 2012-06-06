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

/** @file dsh.c
 * Implementacion de una shell sencilla que hace uso de libdclient para
 * conseguir una cierta transparencia de localizacion en la ejecucion de procesos.
 *
 * Requiere la existencia de una variable de entorno "DSERVER" que indique la
 * direccion y puerto, en formato "IP:puerto", de algun servidor del cluster
 * al que hacer las peticiones.
 *
 * @author Juan Navarro Moreno
 * @author Antonio Gonzalez Huete
 * @date 09-12-2006
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h> //Espera de procesos. Para funcion waitpid().

//Definicion de longitud de buffer, IPs, etc.
#include "dserver.h"

#include "libdclient.h"

#define MAX_ARGS 5

int main(int argc, char *argv[]) {
	char *var, *token;
	char comando[MAXCADENA]; //Comando a ejecutar. OJO con strtok().
	char ipRem[MAX_LEN_DIR];
	int i, j, puertoRem;

	int rc;
	pid_t pidHijo;
	char *args[MAX_ARGS];

	var = getenv("DSERVER");
	if (var == NULL || sscanf(var, "%[^:]:%d", ipRem, &puertoRem)!=2) {
		fprintf(stderr, "%s\n   %s\n",
				"Error interpretando la variable de entorno DSERVER.",
				"Comprueba que sea de la forma \"IP:puerto\"");
		return 0;
	}

	printf("dsh$ "); fflush(stdout);
	bzero(comando, MAXCADENA);
	fgets(comando, MAXCADENA, stdin);
	if (comando[strlen(comando)-1] == '\n') {
		comando[strlen(comando)-1] = '\0';
	}

	while (strcmp(comando, "exit")) {
		//No usamos la cadena para nada mas, asi que usamos strtok().
		token = strtok(comando, " ");
		if (token != NULL)
			token = strtok(NULL, " "); //Extrae el primer argumento.

		i = 0;
		while (token != NULL) {
			//Extrae los argumentos.
			if (i >= MAX_ARGS-1) {
				fprintf(stderr, "Superado el maximo de argumentos para un comando.\n");
				token = NULL;
				continue;
			}
			args[i++] = token;
			token = strtok(NULL, " ");
		}
		args[i] = NULL;

		printf("Comando: [%s]\n", comando);
		for (j=0; j<=i; j++) {
			printf("Argumento %d: [%s]\n", j, args[j]);
		}

		pidHijo = fork();
		switch (pidHijo) {
			case 0:
				//El hijo lanza la funcion d_exec().
				rc = d_exec(var, comando, args);
				if (rc == -1) {
					printf("hijo: d_exec() ha fallado\n");
				}
				exit(rc); //No se deberia legar aqui.
				break;
			case -1:
				printf("Error al crear un proceso para lanzar un d_exec()\n");
				break;
			default:
				break;
		}

		//Espera hasta que el hijo acabe.
		if (waitpid(pidHijo, &rc, 0) == -1)
			perror("dsh: waitpid()");

		if (WIFEXITED(rc)) {
			fprintf(stderr, "dsh: exited, status=%d\n", WEXITSTATUS(rc));
		} else if (WIFSIGNALED(rc)) {
			fprintf(stderr, "dsh: killed by signal %d\n", WTERMSIG(rc));
		}

		//Lee un nuevo comando.
		printf("dsh$ "); fflush(stdout);
		bzero(comando, MAXCADENA);
		fgets(comando, MAXCADENA, stdin);
		if (comando[strlen(comando)-1] == '\n') {
			comando[strlen(comando)-1] = '\0';
		}
	}

	return 0;
}
