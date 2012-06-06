/**
 * @file util.c
 * Funciones auxiliares miscelaneas.
 *
 * @author Juan Navarro Moreno
 * @author Claudio Jesus Sanchez Casanueva
 * @date 05-05-2007
 */

#include <stdlib.h>
#include <time.h>

#include "util.h"


/********************************************
 * Implementacion de las funciones publicas *
 ********************************************/

/**
 * Inicia el manejo de numeros aleatorios.
 */
void randomInit(void)
{
    srand(time(NULL));
}


/**
 * Genera un numero entero aleatorio.
 * @param begin limite inferior.
 * @param end limite superior.
 * @return numero entero aleatorio comprendido entre 'begin' y 'end' (ambos incluidos).
 */
int randomInteger(int begin, int end)
{
    int random = begin + (int) ((end - begin + 1.0) * (rand() / (RAND_MAX + 1.0)));
    return random;
}


/**
 * Genera un numero real aleatorio.
 * @param begin limite inferior.
 * @param end limite superior.
 * @return numero real aleatorio comprendido entre 'begin' y 'end' (ambos incluidos).
 */
double randomDouble(double begin, double end)
{
    double random = begin + ((end - begin) * (rand() / (RAND_MAX * 1.0)));
    return random;
}

/**
* intercambia dos punteros.
* @param p1 puntero 1.
* @param p2 puntero 2.
 */
void swapPuntero(void **p1, void **p2) {
    void *paux;
    
    paux = *p1;
    *p1 = *p2;
    *p2 = paux;
}
