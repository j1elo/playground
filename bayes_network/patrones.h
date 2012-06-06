/**
 * @file patrones.h
 * Cabeceras para el manejo basico de ficheros de patrones.
 *
 * @author Juan Navarro Moreno
 * @author Claudio Jesus Sanchez Casanueva
 * @date 21-02-2007
 */

#ifndef _PATRONES_H
#define _PATRONES_H

///Estructura de almacenamiento de los patrones.
typedef struct {
    ///Numero de atributos que tienen los ejemplos.
    unsigned numAtributos;
    ///Nombres de cada uno de los atributos.
    char **nombresAtributo;
    ///Numero de posibles valores tomados por cada atributo.
    unsigned *valoresAtributo;
    ///Numero de clases entre las que se reparten los ejemplos.
    unsigned numClases;
    ///Numero de ejemplos de cada clase.
    unsigned *numEjemplos;
    ///Numero total de ejemplos.
    unsigned numTotal;
    ///datos[clase][ejemplo][atributo].
    double ***datos;
}
set;


// Prototipos de las funciones publicas.
set *PATR_CrearSet(char *fichero);
void PATR_LiberarSet(set* s);
int PATR_ImprimirSet(set *s, char *fichero);

#endif
