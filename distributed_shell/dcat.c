/** @file dcat.c
 * Programa para listar un fichero remoto.
 * Lee el fichero remoto pasado como argumento, y lo muestra por stdout.
 *
 * @author Juan Navarro Moreno
 * @author Antonio Gonzalez Huete
 * @date 10-12-2006
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/dir.h>
#include <errno.h>

#include "libdclient.h"

#define HELP "Uso: dcat ruta\n"
#define MAX_BUFF 1024


int main(int argc, char *argv[])
{
	int fd, rc;
	char buffer[MAX_BUFF]="", maquina_y_puerto[255]="", ruta[255]="", *entorno;

	// comprueba numero de parametros
	if (argc != 2) {
		printf ("%s", HELP);
      	return -1;
	}

	if ((entorno = getenv("DSERVER"))==NULL)
		fprintf(stderr, "Variable de entorno DSERVER no definida\n");

	strncpy(maquina_y_puerto, entorno, strlen(entorno));

	if (sscanf(argv[1], "%s", ruta) != 1) {
		printf ("%s", HELP);
		return -1;
	}

	// muestra el listado por stdout
	fd = d_open(maquina_y_puerto, ruta, "r");
	if (fd == -1)
      	return -1;

	while ((rc = d_read(fd, buffer, MAX_BUFF - 1)) > 0) {
		buffer[rc] = 0;
		printf("%s", buffer);
	}

	d_close(fd);

	return 0;
}
