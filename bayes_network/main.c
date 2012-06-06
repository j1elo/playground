/**
 * @file main.c
 * Practica 4 de Reconocimiento de Patrones.
 *
 * @author Juan Navarro Moreno
 * @author Claudio Jesus Sanchez Casanueva
 * @date 12-03-2007
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bayes.h"
#include "patrones.h"
#include "agenetico.h"
#include "util.h"

#define MAXCADENA 128


int main(int argc, char **argv) {
	FILE *fIn;
    set *test = NULL;
    set *train = NULL;
	
	int tamPoblacion, tamCromosoma, numEpocas;

    if (argc != 5) {
        printf("Uso:\n   %s <fichero train> <fichero test> <tamanio poblacion> <# epocas sin cambios>\n", argv[0]);
        return 0;
    }
	
	randomInit();
    if (fIn == NULL) {
        perror("==> Error: error al abrir el fichero");
        PATR_LiberarSet(train);
        PATR_LiberarSet(test);
        return -1;
    }	
    train = PATR_CrearSet(argv[1]);
    test = PATR_CrearSet(argv[2]);
    if ((train == NULL) || (test==NULL)) {
        fprintf(stderr, "==> Error durante la lectura de los sets\n");
        PATR_LiberarSet(train);
        PATR_LiberarSet(test);
        return -1;
    }

    if ( (train->numAtributos != test->numAtributos)
            || (train->numClases != test->numClases) ) {
        perror("==> Error: set incompatibles, distinto numero de clases o atributos");
        return -1;
    }
    
	sscanf(argv[3], "%d", &tamPoblacion);
	sscanf(argv[4], "%d", &numEpocas);

	tamCromosoma = train->numAtributos * (train->numAtributos+1) / 2;
   	agenetico(tamPoblacion, tamCromosoma, numEpocas, train, test);

    PATR_LiberarSet(train);
    PATR_LiberarSet(test);

    return 0;
}
