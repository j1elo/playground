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
 *  2004-08-09: Manuel Freire Moran
 */

/*
 * ls.c
 *      una funcion para generar listados como ficheros
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/dir.h>
#include <errno.h>

/*#define MAX_BUFF 1024*/

/*
 * lsopen (version 2.1: corrige fallos de la 2)
 *    lanza un 'ls' de un directorio y devuelve un descriptor de fichero a
 *    traves del cual se puede leer la salida que produce.
 * entra:
 *    el nombre del directorio
 * sale:
 *    el descriptor (se puede leer con 'read()'; se debe cerrar con 'close()'),
 *    -1 si error
 */
int
lsopen (char *nombre)
{
  int pdes[2];
  int childpid;
  int rc;
  DIR *d;

  // Comprueba legibilidad (util para dar valor a ERRNO)
  if ((d = opendir (nombre)) == NULL)
    return -1;
  closedir (d);
  errno = 0;

  // Crea un 'pipe' para recibir el resultado
  pipe (pdes);
  childpid = fork ();
  switch (childpid)
    {
    case 0:
      // conecta el extremo de entrada a la salida estandar del proceso
      close(1);
      dup2 (pdes[1], 1); // escritura; stdout pasa a ser la entrada del pipe
      close(pdes[0]);    // cierro lectura en este proceso
      close(pdes[1]);    // y tambien la escritura (stdout esta escribiendo ahora)
      // ejecuta la instruccion
      rc = execl ("/bin/ls", "/bin/ls", "-l", nombre, (char *)NULL);
      exit (rc); // (no se alcanza)
    case -1:
      return -1;
    default:
      break;
    }

  // devuelve el extremo de salida
  close(pdes[1]);
  return pdes[0];
}

/*
 *
 * main
 *
 */
/*
int
main (int argc, char **argv)
{

  int fd, rc;
  char buffer[MAX_BUFF];

  // comprueba parametros
  if (argc != 2)
    {
      printf ("Esperaba 1 argumento: directorio que listar.\n");
      return -1;
    }

  // muestra el listado por stdout
  fd = lsopen (argv[1]);
  if (fd == -1)
    {
      printf ("Error.\n");
      return 0;
    }
  while ((rc = read (fd, buffer, MAX_BUFF - 1)) > 0)
    {
      buffer[rc] = 0;
      printf(buffer);
    }

  close (fd);

  return 0;
}
*/
