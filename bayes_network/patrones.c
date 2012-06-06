/**
 * @file patrones.c
 * Manejo basico de ficheros de patrones mediante estructuras del tipo "set".
 * Funciones de creacion, impresion y liberacion de recursos.
 *
 * @author Juan Navarro Moreno
 * @author Claudio Jesus Sanchez Casanueva
 * @date 21-02-2007
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "patrones.h"
#include "hash.h"

#define ERR_MEMORIA "fallo al reservar memoria"
#define ERR_FORMATO "formato de fichero incorrecto"

#define ERR printf("hito\n")
// Prototipos de las funciones privadas.
void *errorCrearSet(FILE *f, Tabla_Hash **tablas, set *s, char *msj);



/********************************************
 * Implementacion de las funciones publicas *
 ********************************************/

/**
 * Genera una estructura de datos de tipo set.
 * A partir de un fichero de entrada con formato estandar, almacena la
 * informacion de las clases, ejemplos y atributos que contenga.
 * @param fichero nombre del fichero de datos.
 * @return puntero a una estructura de tipo set con toda la informacion.
 * @return NULL si error.
 * @warning liberar el puntero devuelto mediante PATR_LiberarSet().
 */
set *PATR_CrearSet(char *fichero) {
    FILE *fIn;
    set *patrones;
    unsigned numAtributos, numClases;
    unsigned i, j, k;
    int rc; //Para comprobar retornos de funciones.
    char c[2]; //Para comprobar fin de linea.
    Tabla_Hash **valoresHash=NULL;

    //Abrimos el fichero para comprobar que existe.
    fIn = fopen(fichero, "r");
    if (fIn == NULL) {
        perror("==> Error: leerPatrones");
        return NULL;
    }

    //Reservamos la estructura para almacenar los datos.
    patrones = (set *)calloc(1, sizeof(set));
    if (patrones == NULL)
        return errorCrearSet(fIn, valoresHash, patrones, ERR_MEMORIA);

    //Inicia el numero total a 0.
    patrones->numTotal = 0;

    //Lee el numero de atributos
    if (fscanf(fIn, "%u", &numAtributos) != 1)
        return errorCrearSet(fIn,  valoresHash, patrones, ERR_FORMATO);
    patrones->numAtributos = numAtributos;
    
    //Prepara la memoria para almacenar el numero de posibles valores de cada atributos
    patrones->valoresAtributo = (unsigned *) calloc(numAtributos, sizeof(unsigned));
    if(patrones->valoresAtributo == NULL)
        return errorCrearSet(fIn, valoresHash, patrones, ERR_MEMORIA);
    //Crea las tablas hash para necesarias
    valoresHash = (Tabla_Hash **) calloc(numAtributos, sizeof(Tabla_Hash*));
    if(valoresHash == NULL)
        return errorCrearSet(fIn, valoresHash, patrones, ERR_MEMORIA);
    
    for(i = 0; i < numAtributos; i++) {
        valoresHash[i] = Hash_crear();
        if(valoresHash[i] == NULL) {
            return errorCrearSet(fIn, valoresHash, patrones, ERR_MEMORIA);
        }
    }

    //Prepara memoria para leer los nombres de cada atributo.
    patrones->nombresAtributo = (char **)calloc(numAtributos, sizeof(char *));
    if (patrones->nombresAtributo == NULL)
        return errorCrearSet(fIn, valoresHash, patrones, ERR_MEMORIA);

    //Lee los nombres de cada atributo.
    for (i=0; i<numAtributos; i++)
        fscanf(fIn, "%as", &(patrones->nombresAtributo[i]));

    //Prepara memoria para leer el numero de ejemplos de cada clase.
    if (fscanf(fIn, "%u", &numClases) != 1)
        return errorCrearSet(fIn, valoresHash, patrones, ERR_FORMATO);

    patrones->numClases = numClases;
    patrones->numEjemplos = (unsigned *)calloc(numClases, sizeof(unsigned));
    if (patrones->numEjemplos == NULL)
        return errorCrearSet(fIn, valoresHash, patrones, ERR_MEMORIA);

    //Lee el numero de ejemplos de cada clase.
    for (i=0; i<numClases; i++) {
        rc = fscanf(fIn, "%u", &(patrones->numEjemplos[i]));
        if (rc != 1)
            return errorCrearSet(fIn, valoresHash, patrones, ERR_FORMATO);
        patrones->numTotal += patrones->numEjemplos[i];
    }

    //Prepara memoria para leer los ejemplos de cada clase.
    patrones->datos = (double ***)calloc(numClases, sizeof(double **));
    if (patrones->datos == NULL)
        return errorCrearSet(fIn, valoresHash, patrones, ERR_MEMORIA);
    for (i=0; i<numClases; i++) {
        patrones->datos[i] = (double **)calloc(patrones->numEjemplos[i], sizeof(double *));
        if (patrones->datos[i] == NULL)
        return errorCrearSet(fIn, valoresHash, patrones, ERR_MEMORIA);
        for (j=0; j<patrones->numEjemplos[i]; j++) {
            patrones->datos[i][j] = (double *)calloc(numAtributos, sizeof(double));
            if (patrones->datos[i][j] == NULL)
                return errorCrearSet(fIn, valoresHash, patrones, ERR_MEMORIA);
        }
    }

    //Lee los ejemplos de cada clase.
    for (i=0; i<numClases; i++) {
        for (j=0; j<patrones->numEjemplos[i]; j++) {
            for (k=0; k<numAtributos; k++) {
                rc = fscanf(fIn, "%lf", &(patrones->datos[i][j][k]));
                if (rc != 1)
                    return errorCrearSet(fIn, valoresHash, patrones, ERR_FORMATO);
                rc = Hash_insertar(valoresHash[k], &(patrones->datos[i][j][k]));
                if(rc == -2)
                    return errorCrearSet(fIn, valoresHash, patrones, ERR_MEMORIA);
            }

            /* En este punto deberiamos haber leido una linea completa.
             * Comprueba si el proximo caracter es un retorno de carro,
             * precedido (o no) por otros espacios o tabuladores. */
            fscanf(fIn, "%*[ \r\t\f\v]");
            if (fscanf(fIn, "%[\n]", c) != 1)
                return errorCrearSet(fIn, valoresHash, patrones,ERR_FORMATO);
        }
    }
    fclose(fIn);
    for(i=0; i < patrones->numAtributos; i++) {
        patrones->valoresAtributo[i] = tamanioHash(valoresHash[i]);
        Hash_destruir(valoresHash[i]);
    }
    free(valoresHash);
    
    return patrones;
}


/**
 * Libera toda la informacion de una estructura set.
 * @param s conjunto de clases y ejemplos.
 */
void PATR_LiberarSet(set *s) {
    unsigned i, j;

    if (s == NULL)
        return;

    for (i=0; (i<s->numClases) && (s->datos[i] != NULL); i++) {        
        for (j=0; j<s->numEjemplos[i]; j++) {
            free(s->datos[i][j]);
        }
        free(s->datos[i]);
    }
    free(s->datos);

    free(s->numEjemplos);
            

    
    if(s->nombresAtributo != NULL) {
        for (i=0; i<s->numAtributos; i++)
            free(s->nombresAtributo[i]);
    }
    free(s->nombresAtributo);
            
    free(s->valoresAtributo);

    free(s);
}


/**
 * Imprime toda la informacion en el formato estandar de un fichero de patrones.
 * @param s conjunto de clases y ejemplos.
 * @param fichero indica el destino de la impresion:
 *     Si fichero == NULL, imprime por pantalla.
 *     Si fichero != NULL, imprime en un fichero de ese nombre.
 */
int PATR_ImprimirSet(set *s, char *fichero) {
    FILE *fOut;
    unsigned i, j, k;
    unsigned numAtributos, numClases;
    unsigned *numEjemplos;

    //Abre el fichero.
    if (fichero == NULL)
        fOut = stdout;
    else {
        fOut = fopen(fichero, "w");
        if (fOut == NULL) {
            perror("==> Error: imprimirSet");
            return -1;
        }
    }

    numAtributos = s->numAtributos;
    numClases = s->numClases;
    numEjemplos = s->numEjemplos;

    //Imprime el numero de atributos y los nombres de los atributos.
    fprintf(fOut, "%d\n", numAtributos);
    for (i=0; i<numAtributos; i++)
        fprintf(fOut, "%s ", s->nombresAtributo[i]);
    fprintf(fOut, "\n");

    //Imprime el numero de clases y el numero de ejemplos de cada una.
    fprintf(fOut, "%d", numClases);
    for (i=0; i<numClases; i++)
        fprintf(fOut, " %d", numEjemplos[i]);
    fprintf(fOut, "\n");

    //Imprime todos los ejemplos.
    for (i=0; i<numClases; i++) {
        for (j=0; j<numEjemplos[i]; j++) {
            for (k=0; k<numAtributos; k++)
                fprintf(fOut, "%.2f ", s->datos[i][j][k]);
            fprintf(fOut, "\n");
        }
    }

    if (fichero != NULL)
        fclose(fOut);

    return 0;
}



/********************************************
 * Implementacion de las funciones privadas *
 ********************************************/

/**
 * Maneja los errores ocurridos en PATR_CrearSet().
 * Cierra ficheros y libera memora reservada.
 * @param f stream de datos a cerrar.
 * @param s conjunto de clases y ejemplos.
 * @param msj mensaje de texto a mostrar por stderr.
 */
void *errorCrearSet(FILE *f, Tabla_Hash **tablas, set *s, char *msj) {
    int i;
    fprintf(stderr, "==> Error: crearSet: ");
    fprintf(stderr, "%s\n", msj);
    fclose(f);
    if(tablas!=NULL) {
        for(i=0; i < s->numAtributos; i++)
            Hash_destruir(tablas[i]);
        
        free(tablas);
    }
    PATR_LiberarSet(s);
    return NULL;
}
