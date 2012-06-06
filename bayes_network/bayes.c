/**
 * @file bayes.c
 * Manejo de las redes de Bayes mediante la estructura "red_bayes".
 * Funciones de creacion, impresion y liberacion de recursos.
 *
 * @author Juan Navarro Moreno
 * @author Claudio Jesus Sanchez Casanueva
 * @date 28-03-2007
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "bayes.h"

/* Macros para trabajar con doubles. */
#define iguales(x, y) \
(((x) - (y) < 0.000001) && ((x) - (y) > -0.000001) ? 1 : 0)

/* Define "que" es un espacio. */
#define ESPACIO " \t"

/* Mensajes de error. */
#define ERR_FICHERO "error durante la apertura del fichero"
#define ERR_MEMORIA "fallo al reservar memoria"
#define ERR_TAMANIO "error en el formato de la entrada: cadena de longitud incorrecta"
#define ERR_TAMANIO_IN "error en el formato de la entrada: tamanio distinto del pasado por entrada"
#define ERR_ATRIBUTO_PROB "error en el formato de la entrada: uno de los atributos es 0 pero tiene dependencias"
#define ERR_ATRIBUTO_DEP  "error en el formato de la entrada: uno de los atributos depende de otro inxesistente"
#define ERR_MALFICH  "error en el formato de la entrada: detectado caracter incorrecto"

#define DEBUG 0

// Prototipos de las funciones privadas.
double probEjemploClase(set *s, red_bayes *r, double *ejemplo, int clase, int atr);
double probDepClase(red_bayes *r, set *s, double *ejemplo, int clase);

unsigned **reservarMatriz(int n, int m);
void liberarMatriz(unsigned **matriz);
void imprimirMatriz(unsigned **matriz, int n, int m);
int diagonalMatriz(unsigned **matriz, int n, int m);
void *errorCrearRedBayes(red_bayes *red, char *msj);


/********************************************
 * Implementacion de las funciones publicas *
 ********************************************/

/**
 * Genera una estructura "red_bayes" a partir de una cadena de representacion binaria.
 * La cadena a partir de la cual se va a generar la red de Bayes, puede estar formada
 * o bien por una serie de 1's y 0's sin separacion alguna, o bien por grupos de 1's
 * y 0's separados mediante espacios y/o tabuladores. Esta segunda representacion se
 * ha implementado para permitir identificar de un vistazo a que ecuacion de Bayes
 * pretende representar cada grupo de manera sencilla y asi evitar muchos errores.
 * P.ej. estas dos cadenas son dos posibilidades validas que prepresentan lo mismo:
 *   "000000 10101 0000 101 00 1"
 *   "000000101010000101001"
 *
 * @param cadena cadena con la red de bayes en representacion binaria.
 * @param N_Buena cuantos atributos deberia tener la red bayesiana.
 * Si N_Buena es 0, la comprobacion se omite.
 * @return puntero a una estructura de tipo red_bayes con toda la informacion.
 * @return NULL si error.
 * @warning liberar el puntero devuelto mediante BAY_LiberarRedBayes().
 */
red_bayes *BAY_CrearRedBayes(char *cadena, int N_Buena) {
    red_bayes *red = NULL;
    unsigned L, N;
    unsigned i, j, pos;

    if (strlen(cadena) < 1)
        return errorCrearRedBayes(red, ERR_TAMANIO);

    if (strpbrk(cadena, ESPACIO) != NULL) {
        char *pcadena = NULL;
        char *token = NULL;
        char *cadena2 = NULL;

        //Parece que la cadena esta formada por grupos de caracteres, separados por espacios.
        cadena2 = strdup(cadena);
        if (cadena2 == NULL)
            return errorCrearRedBayes(red, ERR_MEMORIA);

        token = strtok(cadena2, ESPACIO);
        if (strlen(token) >= strlen(cadena)/2) {
            //Se entra aqui si la cadena no esta separada por espacios pero tiene
            //algun espacio o tabulador al final, justo antes del retorno de carro.
            free(cadena2);
            return errorCrearRedBayes(red, ERR_TAMANIO);
        }

        //Leemos la primera agrupacion de caracteres.
        N = strlen(token);
        if (N_Buena > 0 && N != N_Buena) {
            //Nuestro calculo no coincide con lo que nos dicen que es lo correcto.
            free(cadena2);
            return errorCrearRedBayes(red, ERR_TAMANIO_IN);
        }

        /* Si la red tiene N terminos, la cadena debe tener N agrupaciones
         * de longitud decreciente. En este bucle deberiamos encontrar N-1
         * agrupaciones de longitud N-1, N-2, N-3, ..., 3, 2, 1. */
        for (i=N-1; i>=1; i--) {
            token = strtok(NULL, ESPACIO);
            if (token == NULL) {
                //Se acaban las agrupaciones antes de lo debido.
                free(cadena2);
                return errorCrearRedBayes(red, ERR_TAMANIO);
            }

            if (strlen(token) != i) {
                //Hay un grupo de caracteres, pero de una longitud incorrecta.
                free(cadena2);
                return errorCrearRedBayes(red, ERR_TAMANIO);
            }
        }

        /* Si llegamos aqui, la cadena tiene una longitud correcta.
         * Procedemos a construir una cadena de caracteres sin espacios o
         * tabuladores de separacion entre los grupos. */
        strcpy(cadena2, cadena);
        pcadena = cadena;
        token = strtok(cadena2, ESPACIO);
        while (token != NULL) {
            //Copia cada grupo a la cadena.
            strcpy(pcadena, token);
            pcadena += strlen(token);
            token = strtok(NULL, ESPACIO);
        }

        //Ya tenemos la cadena sin separadores entre los grupos de caracteres.
        //Liberamos variables auxiliares.
        free(cadena2);
    } else {
        /* La cadena esta formada por un solo grupo de caracteres sin separaciones.
         * Comprueba si hay un numero correcto de terminos.
         * Si L es Longitud de la cadena, y N es Num. de terminos, tenemos:
         * L = N(N+1)/2 ==> N^2 + N - 2L = 0 ==> N = (-1 + sqrt(1 + 8L)) / 2. */
        L = strlen(cadena);
        N = (unsigned int) (-1 + sqrt(1 + 8*L))/2;
        if (L != N*(N+1)/2)
            return errorCrearRedBayes(red, ERR_TAMANIO);
        if (N_Buena > 0 && N != N_Buena) {
            //Nuestro calculo no coincide con lo que nos dicen que es lo correcto.
            return errorCrearRedBayes(red, ERR_TAMANIO_IN);
        }
    }

    //Reservamos la estructura para almacenar los datos.
    red = (red_bayes *)calloc(1, sizeof(red_bayes));
    if (red == NULL)
        return errorCrearRedBayes(red, ERR_MEMORIA);

    //Reserva memoria para los atributos.
    red->atributos = (unsigned *)calloc(N, sizeof(unsigned));
    if (red->atributos == NULL)
        return errorCrearRedBayes(red, ERR_MEMORIA);

    //Reserva la matriz de dependencias.
    red->dependencias = reservarMatriz(N, N);
    if (red->dependencias == NULL)
        return errorCrearRedBayes(red, ERR_MEMORIA);

    red->numAtributos = N;

    /* Comprueba que los terminos son correctos,
     * y rellena el array de atributos y la matriz de dependencias. */
    for (i=0, pos=0; i<red->numAtributos; i++) {
        if (cadena[pos] != '0' && cadena[pos] != '1')
            return errorCrearRedBayes(red, ERR_MALFICH);

        red->atributos[i] = cadena[pos] - '0';
        pos++;

        //Cada atributo depende de si mismo
        red->dependencias[i][i] = 1;

        for (j=i+1; j<red->numAtributos; j++, pos++) {
            if (red->atributos[i]==0 && cadena[pos]=='1')
                return errorCrearRedBayes(red, ERR_ATRIBUTO_PROB);
            else
                red->dependencias[i][j] = cadena[pos] - '0';
        }
    }

    /* Comprueba que si un atributo es 0 nadie dependa de el;
     * para ello comprueba la columna de la matriz de dependencias. */
    for (i=1; i<red->numAtributos; i++) {
        if (red->atributos[i] == 0) {
            for (j=0; j<i; j++) {
                if (red->dependencias[j][i] != 0)
                    return errorCrearRedBayes(red, ERR_ATRIBUTO_DEP);
            }
        }
    }

    return red;
}

/**
 * Genera una estructura "red_bayes" a partir de una cadena de representacion binaria.
 * La cadena a partir de la cual se va a generar la red de Bayes, puede estar formada
 * o bien por una serie de 1's y 0's sin separacion alguna, o bien por grupos de 1's
 * y 0's separados mediante espacios y/o tabuladores. Esta segunda representacion se
 * ha implementado para permitir identificar de un vistazo a que ecuacion de Bayes
 * pretende representar cada grupo de manera sencilla y asi evitar muchos errores.
 * P.ej. estas dos cadenas son dos posibilidades validas que prepresentan lo mismo:
 *   "000000 10101 0000 101 00 1"
 *   "000000101010000101001"
 * En caso de la cadena binaria describa una red de bayes incorrecta, la corrige
 *
 * @param cadena cadena con la red de bayes en representacion binaria.
 * @param N_Buena cuantos atributos deberia tener la red bayesiana.
 * Si N_Buena es 0, la comprobacion se omite.
 * @return puntero a una estructura de tipo red_bayes con toda la informacion.
 * @return NULL si error.
 * @warning liberar el puntero devuelto mediante BAY_LiberarRedBayes().
 */
red_bayes *BAY_CrearRedBayesCorregida(char *cadena, int N_Buena) {
    red_bayes *red = NULL;
    unsigned L, N;
    unsigned i, j, pos;

    if (strlen(cadena) < 1)
        return errorCrearRedBayes(red, ERR_TAMANIO);

    if (strpbrk(cadena, ESPACIO) != NULL) {
        char *pcadena = NULL;
        char *token = NULL;
        char *cadena2 = NULL;

        //Parece que la cadena esta formada por grupos de caracteres, separados por espacios.
        cadena2 = strdup(cadena);
        if (cadena2 == NULL)
            return errorCrearRedBayes(red, ERR_MEMORIA);

        token = strtok(cadena2, ESPACIO);
        if (strlen(token) >= strlen(cadena)/2) {
            //Se entra aqui si la cadena no esta separada por espacios pero tiene
            //algun espacio o tabulador al final, justo antes del retorno de carro.
            free(cadena2);
            return errorCrearRedBayes(red, ERR_TAMANIO);
        }

        //Leemos la primera agrupacion de caracteres.
        N = strlen(token);
        if (N_Buena > 0 && N != N_Buena) {
            //Nuestro calculo no coincide con lo que nos dicen que es lo correcto.
            free(cadena2);
            return errorCrearRedBayes(red, ERR_TAMANIO_IN);
        }

        /* Si la red tiene N terminos, la cadena debe tener N agrupaciones
         * de longitud decreciente. En este bucle deberiamos encontrar N-1
         * agrupaciones de longitud N-1, N-2, N-3, ..., 3, 2, 1. */
        for (i=N-1; i>=1; i--) {
            token = strtok(NULL, ESPACIO);
            if (token == NULL) {
                //Se acaban las agrupaciones antes de lo debido.
                free(cadena2);
                return errorCrearRedBayes(red, ERR_TAMANIO);
            }

            if (strlen(token) != i) {
                //Hay un grupo de caracteres, pero de una longitud incorrecta.
                free(cadena2);
                return errorCrearRedBayes(red, ERR_TAMANIO);
            }
        }

        /* Si llegamos aqui, la cadena tiene una longitud correcta.
         * Procedemos a construir una cadena de caracteres sin espacios o
         * tabuladores de separacion entre los grupos. */
        strcpy(cadena2, cadena);
        pcadena = cadena;
        token = strtok(cadena2, ESPACIO);
        while (token != NULL) {
            //Copia cada grupo a la cadena.
            strcpy(pcadena, token);
            pcadena += strlen(token);
            token = strtok(NULL, ESPACIO);
        }

        //Ya tenemos la cadena sin separadores entre los grupos de caracteres.
        //Liberamos variables auxiliares.
        free(cadena2);
    } else {
        /* La cadena esta formada por un solo grupo de caracteres sin separaciones.
         * Comprueba si hay un numero correcto de terminos.
         * Si L es Longitud de la cadena, y N es Num. de terminos, tenemos:
         * L = N(N+1)/2 ==> N^2 + N - 2L = 0 ==> N = (-1 + sqrt(1 + 8L)) / 2. */
        L = strlen(cadena);
        N = (unsigned int) (-1 + sqrt(1 + 8*L))/2;
        if (L != N*(N+1)/2)
            return errorCrearRedBayes(red, ERR_TAMANIO);
        if (N_Buena > 0 && N != N_Buena) {
            //Nuestro calculo no coincide con lo que nos dicen que es lo correcto.
            return errorCrearRedBayes(red, ERR_TAMANIO_IN);
        }
    }

    //Reservamos la estructura para almacenar los datos.
    red = (red_bayes *)calloc(1, sizeof(red_bayes));
    if (red == NULL)
        return errorCrearRedBayes(red, ERR_MEMORIA);

    //Reserva memoria para los atributos.
    red->atributos = (unsigned *)calloc(N, sizeof(unsigned));
    if (red->atributos == NULL)
        return errorCrearRedBayes(red, ERR_MEMORIA);

    //Reserva la matriz de dependencias.
    red->dependencias = reservarMatriz(N, N);
    if (red->dependencias == NULL)
        return errorCrearRedBayes(red, ERR_MEMORIA);

    red->numAtributos = N;

    /* Rellena el array de atributos y la matriz de dependencias. */
    for (i=0, pos=0; i<red->numAtributos; i++) {
        if (cadena[pos] != '0' && cadena[pos] != '1')
            return errorCrearRedBayes(red, ERR_MALFICH);

        red->atributos[i] = cadena[pos] - '0';
        pos++;

        //Cada atributo depende de si mismo
        red->dependencias[i][i] = 1;

        for (j=i+1; j<red->numAtributos; j++, pos++) {
        	red->dependencias[i][j] = cadena[pos] - '0';
        }
    }
    /* Si un atributo esta a 0 elimina todas las dependencias. */
    for (i=1; i<red->numAtributos; i++) {
        if (red->atributos[i] == 0) {
			/* Elimina las dependencias vericales */
			for(j=0; j<i;j++) {
				red->dependencias[j][i] = 0;
			}
			/* Elimina las dependencias horizontales */
            for (j=i; j<red->numAtributos; j++) {
				red->dependencias[i][j] = 0;
            }
        }
    }

    return red;
}

int BAY_CorregirRepresentacionBayes(char *cadena) {
	int i, j,pos;
	red_bayes * red;

	red = BAY_CrearRedBayesCorregida(cadena,0);
	if(red == NULL) {
		return -1;
	}

	//Imprime la red.
    for (i=0, pos=0; i<red->numAtributos; i++) {
        if (red->atributos[i] == 0)  {
            for (j = i; j < red->numAtributos; j++, pos++) {
                cadena[pos]='0';
            }

        } else {
            for (j = i; j < red->numAtributos; j++, pos++) {
               cadena[pos]='0' + red->dependencias[i][j];
            }
        }
    }

	BAY_LiberarRedBayes(red);

	return 0;
}

/**
 * Libera toda la informacion de una estructura red_bayes.
 * @param red red de bayes. Si es NULL, no se hace nada.
 */
void BAY_LiberarRedBayes(red_bayes *red) {
    if (red == NULL)
        return;

    free(red->atributos);
    liberarMatriz(red->dependencias);
    free(red);
}


/**
 * Imprime toda la informacion de una red.
 * @param red red de bayes.
 * @param fichero indica el destino de la impresion.
 *     Si fichero == NULL, imprime por pantalla.
 *     Si fichero != NULL, imprime en un fichero de ese nombre.
 */
int BAY_ImprimirRedBayes(red_bayes *red, char *fichero) {
    FILE *fOut;
    unsigned i, j;
    unsigned numAtributos;

    printf("Atributos: [ ");
    for (i=0; i<red->numAtributos; i++)
        printf("%d",red->atributos[i]);
    printf(" ]\n");

    printf("Dependencias :\n");
    imprimirMatriz(red->dependencias, red->numAtributos, red->numAtributos);

    //Abre el fichero.
    if (fichero == NULL)
        fOut = stdout;
    else {
        fOut = fopen(fichero, "w");
        if (fOut == NULL) {
            perror("==> Error: BAY_ImprimirRedBayes()");
            return -1;
        }
    }

    numAtributos = red->numAtributos;

    //Imprime la red.
    for (i=0; i<numAtributos; i++) {
        if (red->atributos[i] == 0)  {
            for (j=i; j<numAtributos; j++) {
                fprintf(fOut, "0");
            }

        } else {
            for (j=i; j<numAtributos; j++) {
                fprintf(fOut, "%d", red->dependencias[i][j]);
            }
        }
    }
    fprintf(fOut,"\n");

    if (fichero != NULL)
        fclose(fOut);

    return 0;
}


/**
 * Clasifica un conjunto de clases y ejemplos usando una red de Bayes.
 * @param red red de bayes.
 * @param train conjunto de clases y ejemplos de entrenamiento.
 * @param test conjunto de clases y ejemplos de test.
 * @return -1 error
 * @return porcentaje de error
 */
double BAY_ClasificarBayes(red_bayes *red, set *train, set *test) {
    /* Buscamos clasificar cada ejemplo (x,y,z) en una clase w.
     * Es decir, la probabilidad de clase condicionada al ejemplo dado:
     * P(w | x,y,z) = [ P(x,y,z | w) * P(w) ] / P(x,y,z) Por la regla de Bayes.
     * De modo que calculamos:
     * P(w): prob. absoluta de la clase w.
     * P(x,y,z): prob. absoluta del ejemplo (x,y,z).
     * P(x,y,z | w): prob. del ejemplo condicionada a la clase. A su vez, esta se calcula como:
     *     P(x,y,z | w) = P(x | y,z,w) * P(y | z,w) * P(z | w).
     */

    unsigned i, j, k, claseGanadora, indefinidos=0;
    unsigned coincidencias;

    double *probClase;
    double probEjCond;    // P(x,y,z | w)
    double probClaseCond; // P(w | x,y,z)
    double maxProbClaseCond; // La clase ganadora es la que tiene mayor valor de prob.
    double error;

    //Prepara una matriz de resultados.
    //resultados[clase real][clase asignada].
    unsigned **resultados = reservarMatriz(test->numClases, test->numClases);
    if (resultados == NULL)
        return -1;

    //calculo de probabilidades a priori
    probClase = (double *) malloc(test->numClases * sizeof(double));
    if (probClase == NULL) {
        liberarMatriz(resultados);
        return -1;
    }
    for(i=0; i < test->numClases; i++)
        probClase[i] = 1.0*train->numEjemplos[i] / train->numTotal;


    for (i=0; i<test->numClases; i++) {
        /**/ if(DEBUG) {
            /**/ printf("clase %d:\n",i);
        /**/ }
        for (j=0; j<test->numEjemplos[i]; j++) {
            //Vemos cual es la clase del conjunto de entrenamiento con mas probabilidad.
            maxProbClaseCond = 0.0;
            claseGanadora = 0;
            coincidencias = 1;
            for (k=0; k<train->numClases; k++) {
                //Probabilidad dependiente de clase.
                probEjCond = probDepClase(red, train, test->datos[i][j], k);

                //Probabilidad de la clase k:
                probClaseCond = probEjCond * probClase[k];

                /**/ if(DEBUG) {
                    /**/ printf("\t\tClase %d: %.4lf\n", k, probClaseCond);
                /**/ }

                // si son iguales elige uno al azar con distribucion equiprobable
                if(iguales(probClaseCond, maxProbClaseCond)) {
                    coincidencias++;
                    if ((1.0*rand()*coincidencias/RAND_MAX) <= 1) {
                        maxProbClaseCond = probClaseCond;
                        claseGanadora = k;
                    }
                }
                else if (probClaseCond > maxProbClaseCond) {
                    maxProbClaseCond = probClaseCond;
                    claseGanadora = k;
                    coincidencias = 1;
                }
            }

            // si se han elegido al azar, incremenamos el contador
            if(coincidencias>1)
                indefinidos++;

            //Rellena la matriz de resultados.
            resultados[i][claseGanadora]++;
        }
    }


/*    //Imprime la matriz de resultados.
    printf("ClasificarBayes; matriz de resultados:\n");
    imprimirMatriz(resultados, test->numClases, test->numClases);

    printf("Total de ejemplos: %u; indefinidos: %u\n", test->numTotal, indefinidos);
	printf("Error: %.4lf\n", 1.0 - (1.0*diagonalMatriz(resultados, test->numClases, test->numClases)/test->numTotal));
*/
    error = 1.0 - (1.0*diagonalMatriz(resultados, test->numClases, test->numClases)/test->numTotal);
    //Libera recursos.
    liberarMatriz(resultados);
    free(probClase);

    return error;
}


/********************************************
 * Implementacion de las funciones privadas *
 ********************************************/

/* Probabilidad de un atributo concreto entre los ejemplos de una clase
determinada con otros atributos determinados.
Ej: P(x | y, z, k); los atributos a priori son y, z, y la clase es k.
    Casos favorables: Numero de ejemplos de clase k con atributos x, y, z.
    Casos posibles: Numero de ejemplos de clase k con atributos y, z.
    Por tanto: P(x | y, z, k) = (ejs. clase k con x,y,z) / (ejs. clase k con y,z).
*/
double probEjemploClase(set *s, red_bayes *r, double *ejemplo, int clase, int atr) {
    char boolAtributo;
    unsigned i, j, numPosibles=0, numFavorables=0;

    for (i=0; i<s->numEjemplos[clase]; i++) {

        //Comprueba si el ejemplo actual cumple con los atributos a priori.
        boolAtributo = 1;

        for (j=atr+1; j<s->numAtributos; j++) {
            if (r->dependencias[atr][j]
                    && !iguales(s->datos[clase][i][j], ejemplo[j])) {
                boolAtributo = 0;
                break;
            }
        }

        if (boolAtributo)
            numPosibles++;
        else
            //Si no los cumple, pasamos a siguiente ejemplo.
            continue;

        //Comprueba si el ejemplo actual cumple con el atributo a posteriori.
        if (iguales(s->datos[clase][i][atr], ejemplo[atr]))
            numFavorables++;
    }

    return 1.0*(numFavorables + 1) / (numPosibles + s->valoresAtributo[atr]);
}


double probDepClase(red_bayes *r, set *s, double *ejemplo, int clase) {
    unsigned i;
    double probDC = 1.0;

    for (i=0; i<s->numAtributos; i++) {
        //Para cada atributo, si participa en la probabilidad total lo tenemos en cuenta.
        if (r->atributos[i] == 1) {
            probDC *= probEjemploClase(s, r, ejemplo, clase, i);
            if (probDC == 0)
                //Aborta si el resultado se hace 0.
                break;
        }
    }

    return probDC;
}


/**
 * Maneja los errores ocurridos en BAY_CrearRedBayes().
 * Cierra ficheros y libera memora reservada.
 * @param s conjunto de clases y ejemplos.
 * @param msj mensaje de texto a mostrar por stderr.
 */
void *errorCrearRedBayes(red_bayes *red, char *msj) {
    fprintf(stderr, "==> Error: crearRedBayes: ");
    fprintf(stderr, "%s\n", msj);
    BAY_LiberarRedBayes(red);
    return NULL;
}


/**
 * Crea una matriz de n x m de unsigned int.
 * Utliza calloc() para reservar la memoria, por lo que las celdas
 * de la matriz son inicializadas a 0.
 * @param n numero de filas.
 * @param m numero de columnas.
 * @return doble puntero a la matriz.
 * @return NULL si error.
 * @warning liberar el puntero devuelto mediante liberarMatriz().
 */
unsigned** reservarMatriz(int n, int m) {
    int i;
    unsigned *matriz, **vector;

    vector = (unsigned **) calloc(n, sizeof(unsigned *));
    if (vector == NULL)
        return NULL;

    matriz = (unsigned *) calloc(n*m, sizeof(unsigned));
    if (matriz == NULL) {
        free(vector);
        return NULL;
    }

    vector[0] = matriz;
    for (i = 1; i < n; i++) {
        vector[i] = vector[i-1] + m;
    }

    return vector;
}


/**
 * Libera toda la informacion de una matriz
 * @param matriz matriz a liberar.
 */
void liberarMatriz(unsigned **matriz) {
    if (matriz == NULL)
        return;

    free(matriz[0]);
    free(matriz);
}


/**
 * Imprime la informacion de una matriz
 * @param matriz matriz a imprimir.
 * @param n numero de filas.
 * @param m numero de columnas.
 */
void imprimirMatriz(unsigned **matriz, int n, int m) {
    int i, j;

    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++)
            printf("%5d ", matriz[i][j]);
        printf("\n");
    }
}

// calcula el numero de elmentos en la diagonal de una matriz cuadrada
int diagonalMatriz(unsigned **matriz, int n, int m) {
	int i;
	int total=0;

	// si no es una matriz cuadrada
	if(n!=m)
		return -1;

	for(i=0; i<n; i++) {
			total += matriz[i][i];
	}

	return total;
}
