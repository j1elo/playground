#!/bin/sh
#
# Este script sirve para probar la práctica 2 de SO2. Si el script devuelve 0
# como resultado, se entiende que todo es correcto. Si devuelve otro valor, y
# se esta llamando desde un 'makefile', se interpretara como error.
#
# Para ello, lanza SAPT y comprueba que cada una de las llamadas del servidor
# funciona como debe.
#
# Para incluir mas pruebas, modifica el valor de la variable SCRIPTS:
# Ej.: SCRIPTS="mi_prueba1.txt mi_prueba2.txt"
#

# constantes
TESTED="d_server"
SCRIPTS="prueba_dserver.txt"
OUTPUT="/tmp/salida-sapt"

# comprueba que está todo
if ! [ -x sapt ] ; then
    echo "Error: no encuentro el ejecutable 'sapt'"; exit
fi
if ! [ -x $TESTED ] ; then
    echo "Error: no encuentro el ejecutable a probar, '$TESTED'"; exit;
fi
for i in $SCRIPTS; do
    if ! [ -f $i ] ; then
        echo "Error: no encuentro el script de pruebas '$i'"; exit;
    fi
done

# lanza el servidor
./$TESTED 8000 2>&1 > /dev/null &

# lanza pruebas
TOTAL_ERRORS=0
echo
for i in $SCRIPTS; do

    # prueba con fichero de pruebas $i
    OUT="$OUTPUT-$i"
    echo "[ERROR] - 'sapt' no ha llegado a producir salida." > $OUT
    echo -n "Probando con '$i'... "
    if ! ./sapt $i > $OUT ; then
        echo "Error: 'sapt' ha devuelto error durante su ejecucion."
    fi;

    # muestra resultados con ese fichero
    ERRNUM=$(grep -c ERROR $OUT | sed -e 's/.*://')
    if [[ $ERRNUM != 0 ]] ; then
        echo "NO PASA EL TEST: $ERRNUM error(es). Detalles en '$OUT'"
        TOTAL_ERRORS=1
    else
        echo "PASA EL TEST"
        rm $OUT
    fi

done

# para el servidor
killall $TESTED

# devuelve 0 si se pasan todos los tests sin errores
if [[ $TOTAL_ERRORS == 0 ]] ; then
    exit 0
else
    exit 1
fi
