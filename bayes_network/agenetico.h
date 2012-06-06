/**
 * @file agenetico.h
 * Cabeceras para el manejo algoritmos geneticos.
 *
 * @author Juan Navarro Moreno
 * @author Claudio Jesus Sanchez Casanueva
 * @date 01-05-2007
 */

#ifndef _AGENETICO_H
#define _AGENETICO_H

#include "patrones.h"

typedef struct{
    char *cromosoma;
    double fitness;
} individuo;

int agenetico(int tamPoblacion, int tamCromosoma, int numEpocas, set *train, set * test);

#endif
