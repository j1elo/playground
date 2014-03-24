/** @file dtac.c
 * Programa para escribir un fichero remoto.
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

#define HELP "Uso: dtac ruta\n"
#define MAX_BUFF 1024


int main(int argc, char *argv[])
{
	int fd, rc;
	char buffer[MAX_BUFF]="", maquina_y_puerto[255]="", ruta[255]="", *entorno;

	// comprueba numero de parametros
	if (argc!=2) {
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


	//escribe la entrada de stdin
	fd = d_open(maquina_y_puerto, ruta, "w+");
	if (fd == -1)
      	return -1;

 	printf(":q para terminar\n");
	while(1) {
		bzero(buffer, MAX_BUFF);
		fgets(buffer, MAX_BUFF, stdin);
		if (strcmp(buffer, ":q\n") == 0)
			break;
		rc = d_write(fd, buffer, strlen(buffer));
        (void)rc; // Supress "unused value" warning
	}

	d_close(fd);

	return 0;
}
