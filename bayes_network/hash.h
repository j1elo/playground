/* Libreria que implementa una Tabla Hash de Encadenamiento, adaptada para servir como Tabla de Simbolos*/
#ifndef _HASH_H
#define _HASH_H

#define MAX_TABLA 65521  // numero primo mas cercano a 2^16 - 1

/* estructura que contiene los datos de la tabla Hash
   Como es una tabla de encadenamiento, hemos decidido que cada casilla de
   la tabla sea una lista enlazada, facilitando asi ordenar las variables*/
typedef struct VARIABLE{
	double info;
	struct VARIABLE *siguiente;
	struct VARIABLE *anterior;
} Variable;


/*Crea una tabla Hash.
     Retorno
	Tabla_Hash *: puntero a la tabla creada	
*/
typedef struct {
    // indica el numero de variables contenida en la tabla
    int tamanio;
	Variable* fila[MAX_TABLA];
} Tabla_Hash;


Tabla_Hash *Hash_crear();

/*Libera toda la memoria asociada a la tabla Hash pasada como parametro
     Entrada
	Tabla_Hash *tabla: tabla a liberar
*/
void Hash_destruir(Tabla_Hash *tabla);

/* Busca en al tabla Hash la informacion que concuerde con la ID
     Entrada
	Tabla_Hash *tabla: Tabla donde se buscara la informacion
	double *ID: ID a buscar
     Retorno
	NULL si no se ha encontrado la ID
	La informacion asociada a la ID si se ha encontrado
*/
double *Hash_buscar(Tabla_Hash *tabla, double *ID);

/* Inserta una informacion en la tabla de simbolos
     Entrada
	Tabla_Hash *tabla: tabla donde se insertara la informacion
        double *info: informacion a insertar
     Retorno
	-1:  si ha sido posible insertar. A este resultado se llega unicamente si ha sido posible reservar memoria.
	0:  la informacion se ha insertado
*/
int Hash_insertar(Tabla_Hash *tabla, double *info);

/* Devuele array con referencias a toda la  informacion contenida en la tabla
     Entrada
	Tabla_Hash *tabla: la tabla de donde se extraen los datos
     Retorno
	 NULL: si ha habido lagun error
         double **: array con todas las referencias
*/
double **DevolverTodo(Tabla_Hash *tabla);

/* Devuele el numero de elementos almacenados en la tabla hash
    Entrada
        Tabla_Hash *tabla: la tabla donde se almacenan los elmentos
    Retorno
        -1 si ha habido lagun error
        >=0 si no ha habido ningun error
*/
int tamanioHash(Tabla_Hash *tabla);


#endif
