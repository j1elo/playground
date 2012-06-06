/**
 * @file bayes.h
 * Cabeceras para el manejo de las redes de bayes
 *
 * @author Juan Navarro Moreno
 * @author Claudio Jesus Sanchez Casanueva
 * @date 28-03-2007
 */

#ifndef _BAYES_H
#define _BAYES_H

#include "patrones.h"

///Estructura de almacenamiento de los patrones.
typedef struct {
    unsigned numAtributos;
    unsigned *atributos;
    unsigned **dependencias;
}
red_bayes;


// Prototipos de las funciones publicas.
red_bayes *BAY_CrearRedBayes(char *cadena, int N_Buena);
int BAY_CorregirRepresentacionBayes(char *cadena);
int BAY_ImprimirRedBayes(red_bayes *red, char *fichero);
void BAY_LiberarRedBayes(red_bayes *red);

double BAY_ClasificarBayes(red_bayes *red, set *train, set *test);

#endif
