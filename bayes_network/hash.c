/* Libreria que implementa una Tabla Hash de Encadenamiento, adaptada para servir como Tabla de Simbolos*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hash.h"
#include "util.h"

/* Obtiene el valor asociado a una ID, clave de la ID. Para ello almacena el suelo de la informacion.
     Entrada:  
	 char *ID : cadena de la que se quiere obtener la clave
     Retorno:
	 unsigned short int: la clave, despues de haberla adaptado al tama�o de la tabla.
*/
unsigned short int Hash_ID_valor(double *ID){
	return  ((int) *ID)%MAX_TABLA;
}

/* Copia una estructura que contiene la informacion de una casilla de la tabla hash
     Entrada
        double *var1 : destino
        double *var2: origen.
*/
void copiaAtrib(double *var1, double *var2) {
	*var1 = *var2;
	return;	
}

/* Crea "Variable" de la tabla hash, cuya informacion sera una copia del parametro pasado info.
     Entrada
	 double *info: informacion que contendra la nueva Variable
     Retorno
	 Variable *: la Variable creada
*/
Variable *Hash_variable(double *info){
	Variable *temp;

	temp = malloc(sizeof(Variable));

	if(temp == NULL)
            return NULL;

	copiaAtrib( &(temp->info), info);

	temp->siguiente = NULL;
	temp->anterior = NULL;

	return temp;
}

/*Crea una tabla Hash.
     Retorno
	Tabla_Hash *: puntero a la tabla creada	
*/
Tabla_Hash *Hash_crear(){
	Tabla_Hash *tabla;
	int i;
	
	tabla = malloc(sizeof(Tabla_Hash));
	if(tabla == NULL)
		return NULL;
	tabla->tamanio = 0;
	
	// inicia cad una de las posiciones
	for(i=0;i<MAX_TABLA; i++) 
		tabla->fila[i] = NULL;

	return tabla;
}

/*Libera toda la memoria asociada a la tabla Hash pasada como parametro
     Entrada
	Tabla_Hash *tabla: tabla a liberar
*/
void Hash_destruir(Tabla_Hash *tabla) {
	int i=0;
	Variable *aux;

	if (tabla == NULL)
		return;

    // libera cada una de las posiciones
	for(i=0;i<MAX_TABLA; i++){
	    // libera la lista enlazada
		while(tabla->fila[i] != NULL) {
			aux=tabla->fila[i];
			tabla->fila[i] = (tabla->fila[i])->siguiente;
			free(aux);
		}		
	}

	free(tabla);
}


/* Busca en al tabla Hash la informacion que concuerde con la ID
     Entrada
	Tabla_Hash *tabla: Tabla donde se buscara la informacion
	char *ID: ID a buscar
     Retorno
	NULL si no se ha encontrado la ID
	La informacion asociada a la ID si se ha encontrado
*/
double *Hash_buscar(Tabla_Hash *tabla, double *ID) {
	unsigned short valID;
	int ret;
	Variable *aux;

	if((tabla == NULL) || (ID == NULL))
		return NULL;

	// obtiene la casilla de la tabla
	valID = Hash_ID_valor(ID);

	// a la casilla de la tabla contiene una referncia la comienzo de una lista de variables
	aux = tabla->fila[valID];

	// buscamos mientras no lleguemos al final de la lista.
	while(aux!=NULL) {
        ret = Igual(*ID, aux->info);
		// si lo hemos encontrado
		if(ret == 0)
            return &(aux->info);
		// como la tabla esta ordenador, si encuentra uno menor que el actual es que no esta
		else if(ret < 0)
            return NULL;

		aux = aux->siguiente;
	}

	// si llegamos al final de la tabla sin encontrar nada
	return NULL;
}

/* Inserta una informacion en la tabla de simbolos
     Entrada
	Tabla_Hash *tabla: tabla donde se insertara la informacion
	tipo_atributos *info: informacion a insertar
     Retorno
	-2:  si ha sido posible insertar. A este resultado se llega unicamente si ha sido posible reservar memoria.
        -1: si el elemento ya existia.
	0:  la informacion se ha insertado
*/
int Hash_insertar(Tabla_Hash *tabla, double *info) {
	unsigned short valID;
	Variable *temp, *aux;

	if((tabla == NULL) || (info == NULL))
		return -2;

	// crea la variable que contendra una copia de info
	temp = Hash_variable(info);

	if(temp == NULL)
		return -2;

	// encuentra la casilla
	valID = Hash_ID_valor(info);


	// si esa fila estaba vacia
	if(tabla->fila[valID] ==NULL)
		tabla->fila[valID] = temp;
    // si la variable es igual al primer valor de la tabla
    else if(Igual(tabla->fila[valID]->info,*info))
            return -1;
    else if (MayorQue(tabla->fila[valID]->info,*info)) {
		tabla->fila[valID]->anterior = temp;
		temp->siguiente = tabla->fila[valID];
		tabla->fila[valID] = temp;
	}
	else {
		aux = tabla->fila[valID];
		// busca hasta el final de la tabla la posicion donde incluirlo
                while(MenorQue(aux->info, *info) && (aux->siguiente != NULL))
			aux = aux->siguiente;

		// si tiene que ir antes que aux
        if(Igual(aux->info, *info))
                return -1;
		if(MayorQue(aux->info, *info) > 0) {
			aux=aux->anterior;
			(aux->siguiente)->anterior = temp;
			temp->siguiente = aux->siguiente;
			aux->siguiente = temp;
			temp->anterior = aux;
		}
		// si tiene que ir al final de la tabla
		else {
			aux->siguiente=temp;
			temp->anterior = aux;
		}
	}

	// se incrementa el numero de variables
	(tabla->tamanio)++;

	return 0;
}

/* Devuele array con referencias a toda la  informacion contenida en la tabla
     Entrada
	Tabla_Hash *tabla: la tabla de donde se extraen los datos
     Retorno
	 NULL: si ha habido lagun error
	tipo_atributos **: array con todas las referencias
*/
double **DevolverTodo(Tabla_Hash *tabla) {
	int i, n_vars=0;
	Variable *var;
	double **vars;
	
	if (tabla == NULL)
		return NULL;
	
	// crea la tabla para que entren todas las variables
    vars = (double**) calloc((tabla->tamanio) + 1,sizeof(double*));
	if (vars == NULL)
		return NULL;
	
	// recorre toda la tabla copiando las referencias
	for (i=0; i<MAX_TABLA; i++) {
		var = tabla->fila[i];
		while (var != NULL) {
			vars[n_vars] = &(var->info);
			n_vars++;
			var = var->siguiente;
		}
	}
	// iguala la ultima opsicion de la tabla a NULL, asi se sabra donde esta el fianl de esta
	vars[n_vars]=NULL;
	return vars;
}

/* Devuele el numero de elementos almacenados en la tabla hash
    Entrada
        Tabla_Hash *tabla: la tabla donde se almacenan los elmentos
    Retorno
        -1 si ha habido lagun error
        >=0 si no ha habido ningun error
*/
int tamanioHash(Tabla_Hash *tabla) {
	if (tabla == NULL)
		return -1;
        
	return tabla->tamanio;
}
