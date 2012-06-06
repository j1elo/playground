/**
 * @file util.h
 * Cabeceras de funciones auxiliares miscelaneas.
 *
 * @author Juan Navarro Moreno
 * @author Claudio Jesus Sanchez Casanueva
 * @date 05-05-2007
 */

#ifndef _UTIL_H
#define _UTIL_H

/* Macros para trabajar con doubles. */
#define Igual(x, y) \
(((x) - (y) < 0.000001) && ((x) - (y) > -0.000001) ? 1 : 0)

#define MenorQue(x, y) \
((Igual((x), (y)) == 0) && ((x) < (y)) ? 1 : 0)

#define IgualMenorQue(x, y) \
(Igual((x), (y)) || ((x) < (y)) ? 1 : 0)

#define MayorQue(x, y) \
((Igual((x), (y)) == 0) && ((x) > (y)) ? 1 : 0)

#define IgualMayorQue(x, y) \
(Igual((x), (y)) || ((x) > (y)) ? 1 : 0)

#define ERR printf("aqui\n");


// Prototipos de las funciones publicas.
void randomInit(void);
int randomInteger(int begin, int end);
double randomDouble(double begin, double end);
void swapPuntero(void **p1, void **p2);

#endif
