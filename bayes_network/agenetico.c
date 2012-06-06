/**
 * @file agenetico.c
 * Manejo de un algoritmo genetico.
 * Funciones de creacion, impresion y liberacion de recursos.
 *
 * @author Juan Navarro Moreno
 * @author Claudio Jesus Sanchez Casanueva
 * @date 28-03-2007
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bayes.h"
#include "util.h"
#include "agenetico.h"

// definicion de probabilidades
#define PROB_ALELO 0.1
#define PROB_RECOM 0.85

//definiciones generales
#define MAX_EPOCAS 1000
/* protipos de funciones privadas */
int crearCromosoma(char *outCr, int longitud, double prob);
individuo* crearPoblacion(int tamPoblacion, int tamCromosoma);
individuo* crearPoblacionAleatoria(int tamPoblacion, int tamCromosoma, double probAlelo);
void extinguirPoblacion(individuo *poblacion, int tamPoblacion);
int recombinarCromosomas(const char *inCr1, const char *inCr2, char *outCr1, char *outCr2, double pRecomb);
int mutarCromosoma(const char *inCr, char *outCr, double pMuta);
individuo* torneo(individuo * in1, individuo *in2);
int reconstruirCromosoma(char *cromosoma);
double calcularFitness(individuo *in, set *train, set * test);
void clonarIndividuo(individuo *original, individuo *clon);

int agenetico(int tamPoblacion, int tamCromosoma, int numEpocas, set *train, set * test) {
    int i, j;
    int epoca = 1, sinCambios = 0;
    // poblacion de mutacion
    double probMut;
    individuo *p1, *p2;
    //indiviudo con mejor fitness
    individuo *best;
	unsigned mejora;
    individuo *prog1, *prog2;
	individuo *desc1, *desc2;

    if((tamPoblacion < 4) || (tamPoblacion%2 != 0)) {
        printf("La poblacion debe ser par y con un minimo de 4 individuos\n");
        return -1;
    }
    else if(tamCromosoma < 1) {
        printf("El tamaño de cromosoma debe ser como minimo 1\n");
        return -1;
    }
    else if(numEpocas < 1) {
        printf("Tiene que haber como minimo 1 epoca\n");
        return -1;
    }
    else if((train == NULL) || (test == NULL)) {
        return -1;
    }

	probMut = 1.0/tamCromosoma;

    // crea la poblacion inicial
    p1 = crearPoblacionAleatoria(tamPoblacion, tamCromosoma, PROB_ALELO);
    // crea la poblacion de descendientes
    p2 = crearPoblacion(tamPoblacion, tamCromosoma);
    // crea la poblacion de descendientes
    best = crearPoblacion(1, tamCromosoma);

    // obtiene el mejor individuo
    p1[0].fitness = calcularFitness(p1, train, test);
    clonarIndividuo(p1,best);
    for(i=1; i < tamPoblacion; i++) {
        p1[i].fitness = calcularFitness(p1 + i, train, test);
        if(MayorQue(p1[i].fitness, best->fitness)) {
            clonarIndividuo( p1 + i, best);
        }
    }

    printf("epoca: %3d\n", 0);
    printf("   red: %s - error train: %5.2lf %% - error test: %5.2lf %%\n", best->cromosoma,  100.0*(1.0-calcularFitness(best, train, train)), 100.0*(1.0-best->fitness));

    while((sinCambios < numEpocas) && (epoca <= MAX_EPOCAS)) {
        printf("epoca: %3d  --  sin cambios: %3d epocas\n", epoca, sinCambios);
		mejora = 0;
		for(i = 0, j = 0; i < (tamPoblacion/2); i++, j+=2) {
			// torneos para seleccionar los progenitor
			prog1 = torneo(p1 + randomInteger(0, tamPoblacion-1), p1 + randomInteger(0, tamPoblacion-1));
			prog2 = torneo(p1 + randomInteger(0, tamPoblacion-1), p1 + randomInteger(0, tamPoblacion-1));

			// seleccion de los descendientes
			desc1 = p2 + j;
			desc2 = p2 + j+1;

			// recombinacion de de individuos
			recombinarCromosomas(prog1->cromosoma, prog2->cromosoma, desc1->cromosoma, desc2->cromosoma, PROB_RECOM);

			//mutar cromosoma
			mutarCromosoma(desc1->cromosoma, desc1->cromosoma, probMut);
			mutarCromosoma(desc2->cromosoma, desc2->cromosoma, probMut);

			// reconstruir los individuos
			reconstruirCromosoma(desc1->cromosoma);
			reconstruirCromosoma(desc2->cromosoma);

			// calcular el fitness de los nuevos individuos y ver si hay mejoras
            desc1->fitness = calcularFitness(desc1, train, test);
            desc2->fitness = calcularFitness(desc2, train, test);
			if(MayorQue(desc1->fitness, best->fitness)) {
                mejora = 1;
                clonarIndividuo( desc1, best);
			}
			if(MayorQue(desc2->fitness, best->fitness)) {
                mejora = 1;
                clonarIndividuo( desc2, best);
			}
		}

		if(mejora == 1) {
			sinCambios = 0;
            printf("   red: %s - error train: %5.2lf %% - error test: %5.2lf %%\n", best->cromosoma, 100.0*(1.0-calcularFitness(best, train, train)), 100.0*(1.0-best->fitness));
		}
		else {
			sinCambios++;
		}

		epoca++;

        // seleccion de supervivientes: sustitucion de los progenitores
        swapPuntero((void *) &p1, (void *) &p2);
    }

    printf("\n\n");
    printf("tamanio poblacion: %3d, # epocas sin cambios: %d, mejor individuo: %s - error train: %5.2lf %% - error test: %5.2lf %%\n\n", tamPoblacion, numEpocas, best->cromosoma,  100.0*(1.0-calcularFitness(best, train, train)), 100.0*(1.0-best->fitness));
	extinguirPoblacion(p1, tamPoblacion);
	extinguirPoblacion(p2, tamPoblacion);
    extinguirPoblacion(best, 0);


	return 0;
}


/* Implemetacion de funciones privadas **/

/**
 * Crea una cadena que representa un cromosoma aleatorio.
 * @param outCr buffer donde almacenar la cadena del cromosoma creado.
 *   Debe tener espacio para 'longitud' + 1 caracteres (por el '/0' terminador de cadena).
 * @param longitud longitud deseada del cromosoma.
 * @param prob probabilidad de que un alelo tome el valor 1
 * @return 0 OK.
 * @return -1 error: valor de 'longitud' invalido.
 */
int crearCromosoma(char *outCr, int longitud, double prob)
{
    int i;
    double random;

    if (longitud < 1)
        return -1;

    for (i=0; i<longitud; i++) {
        random = randomDouble(0.0, 1.0);
        if(IgualMenorQue(random, prob))
            outCr[i] = '1';
        else
            outCr[i] = '0';
    }
    outCr[longitud] = '\0';

    return 0;
}

/**
* Crea una poblacion de individuos.
* @param tamPoblacion numero de individuos de la poblacion
* @param tamCromosoma tamaño de los cromosmas
* @return NULL si errores
* @return poblacion
*/
individuo* crearPoblacion(int tamPoblacion, int tamCromosoma) {
    individuo* poblacion;
    int i;

    poblacion = (individuo *) calloc(tamPoblacion, sizeof(individuo));
    if(poblacion == NULL)
        return NULL;

    for(i = 0; i < tamPoblacion; i++) {
        poblacion[i].cromosoma = (char *) calloc(tamCromosoma, sizeof(char));
        if(poblacion[i].cromosoma == NULL) {
            for(i--; i < tamPoblacion; i--) {
                free(poblacion[i].cromosoma);
            }
            free(poblacion);

            return NULL;
        }
    }
    return poblacion;
}

/**
* Crea una poblacion de individuos con los cromosmas iniciados aleatoriamente.
* @param tamPoblacion numero de individuos de la poblacion
* @param tamCromosoma tamaño de los cromosmas
* @param probAlelo probabilidad de que un alelo tomo el valor '1'.
* @return NULL si errores
* @return poblacion
*/
individuo* crearPoblacionAleatoria(int tamPoblacion, int tamCromosoma, double probAlelo){
    individuo* poblacion;
    int i;

    poblacion = (individuo *) calloc(tamPoblacion, sizeof(individuo));
    if(poblacion == NULL)
        return NULL;

    for(i = 0; i < tamPoblacion; i++) {
        poblacion[i].cromosoma = (char *) calloc(tamCromosoma, sizeof(char));
        if(poblacion[i].cromosoma == NULL) {
            for(i--; i < tamPoblacion; i--) {
                free(poblacion[i].cromosoma);
            }
            free(poblacion);

            return NULL;
        }
        crearCromosoma(poblacion[i].cromosoma, tamCromosoma, probAlelo);
        BAY_CorregirRepresentacionBayes(poblacion[i].cromosoma);
    }
    return poblacion;
}

/**
* Libera la informacion deuna poblacion.
* @param poblacion poblacion a liberar
*/
void extinguirPoblacion(individuo *poblacion, int tamPoblacion) {
    int i;

    if(poblacion == NULL)
        return;

    for(i = 0; i < tamPoblacion; i++) {
        free(poblacion[i].cromosoma);
    }

    free(poblacion);
}

/**
 * Recombina dos cromosomas dados.
 * @param inCr1 cadena del cromosoma progenitor 1.
 * @param inCr2 cadena del cromosoma progenitor 2. Debe ser de la misma longitud que inCr1.
 * @param[out] outCr1 buffer donde almacenar la cadena del cromosoma descendiente 1.
 *   Debe tener espacio para 'strlen(inCr1)' + 1 caracteres (por el '/0' terminador de cadena).
 * @param[out] outCr2 buffer donde almacenar la cadena del cromosoma descendiente 2.
 *   Debe tener espacio para 'strlen(inCr1)' + 1 caracteres (por el '/0' terminador de cadena).
 * @param pRecomb probabilidad en el intervalo [0.0, 1.0] de que los cromosomas progenitores
 *   sean recombinados.
 * @return 0 OK. Los progenitores no se han recombinado.
 * @return >=1 posicion a partir de la cual se han recombinado los progenitores.
 * @return -1 error: parametros de entrada incorrectos.
 */
int recombinarCromosomas(const char *inCr1, const char *inCr2, char *outCr1, char *outCr2, double pRecomb)
{
    int i;
    int separador;
    double random;

    if (!inCr1 || !inCr2 || !outCr1 || !outCr2
         || strlen(inCr1) < 1 || strlen(inCr2) < 1 || strlen(inCr1) != strlen(inCr2)
         || pRecomb < 0.0 || pRecomb > 1.0) {
        return -1;
    }

    random = randomDouble(0.0, 1.0);
    //Ojo nunca se debe meter la llamada a una funcion en una macro.
    if (IgualMenorQue(random, pRecomb)) {
        //Recombina los progenitores.
        separador = randomInteger(1, strlen(inCr1)-1);

        for (i=0; i<separador; i++) {
            outCr1[i] = inCr1[i];
            outCr2[i] = inCr2[i];
        }
        for (i=separador; i<strlen(inCr1); i++) {
            outCr1[i] = inCr2[i];
            outCr2[i] = inCr1[i];
        }
        outCr1[strlen(inCr1)] = '\0';
        outCr2[strlen(inCr1)] = '\0';
    }
    else {
        //Los progenitores se devuelven intactos.
        separador = 0;
        strcpy(outCr1, inCr1);
        strcpy(outCr2, inCr2);
    }

    return separador;
}


/**
 * Muta un cromosoma.
 * @param inCr cadena del cromosoma a mutar.
 * @param[out] outCr buffer donde almacenar la cadena del cromosoma mutado.
 *   Debe tener espacio para 'strlen(inCr)' + 1 caracteres (por el '/0' terminador de cadena).
 * @param pMuta probabilidad en el intervalo [0.0, 1.0] de que cada bit del cromosoma mute.
 * @return 0 OK.
 * @return -1 error: parametros de entrada incorrectos.
 */
int mutarCromosoma(const char *inCr, char *outCr, double pMuta)
{
    int i;
    double random;

    if (!inCr || !outCr
         || strlen(inCr) < 1
         || pMuta < 0.0 || pMuta > 1.0) {
        return -1;
    }

    for (i=0; i<strlen(inCr); i++) {
        random = randomDouble(0.0, 1.0);
        //Ojo nunca se debe meter la llamada a una funcion en una macro.
        if (IgualMenorQue(random, pMuta)) {
            //Muta el bit del cromosoma.
            if (inCr[i] == '0')
                outCr[i] = '1';
            else
                outCr[i] = '0';
        }
        else {
            outCr[i] = inCr[i];
        }
    }
    outCr[strlen(inCr)] = '\0';

    return 0;
}

/**
* Realiza un torneo entre dos individuos
* @param in1 individuo 1.
* @param in2 individuo 2.
* @return individuo ganador.
 */
individuo* torneo(individuo *in1, individuo *in2) {
    if(Igual(in1->fitness, in2->fitness)) {
        if(randomInteger(0,1) == 0)
            return in1;
        else
            return in2;
    }
    else if(MayorQue(in1->fitness, in2->fitness))
        return in1;
    else
        return in2;
}

/**
* Reconstruye un cromosoma para que sea valido en la representacion actual
* @param cromosoma cromosoma a corregir
* @return 0 OK.
 * @return -1 error: parametros de entrada incorrectos o errores internos.
 */
int reconstruirCromosoma(char *cromosoma) {
    return BAY_CorregirRepresentacionBayes(cromosoma);
}

/**
* Calcular el fitness de un individuo
* @param cromosoma cromosoma del individo
* @return -1 error.
* @return fitness;
*/
double calcularFitness(individuo* in, set *train, set * test) {
    red_bayes * red;
    double fitness;

    red = BAY_CrearRedBayes(in->cromosoma, 0);
    if(red == NULL)
        return -1;

    fitness = 1.0 - BAY_ClasificarBayes(red, train, test);

    BAY_LiberarRedBayes(red);

    return fitness;
}

void clonarIndividuo(individuo *original, individuo *clon) {
    strcpy(clon->cromosoma, original->cromosoma);
    clon->fitness = original->fitness;
}
