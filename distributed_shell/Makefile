CC = gcc
CFLAGS = -Wall -ggdb -I.
LDFLAGS = -lpthread

BIN = sapt d_server dls dcat dtac dsh
DOC = doxygen

all : d_server apps

# Incluye compilacion de la herramienta sapt
test : all
	make --silent --directory=./sapt-src/
	cp ./sapt-src/sapt .
	./test.sh

d_server : libdclient.o dserver.o dserver-petic.o dserver-r_exec.o dserver-ls.o semaphore.o sock.o
	$(CC) -o $@ $^ $(LDFLAGS)

apps : client dls dcat dtac dsh

client : libdclient.o

dls : libdclient.o dls.o sock.o
	$(CC) -o $@ $^ $(LDFLAGS)

dcat : libdclient.o dcat.o sock.o
	$(CC) -o $@ $^ $(LDFLAGS)

dtac : libdclient.o dtac.o sock.o
	$(CC) -o $@ $^ $(LDFLAGS)

dsh : libdclient.o dsh.o sock.o
	$(CC) -o $@ $^ $(LDFLAGS)


## Documentacion (requiere Doxygen)
doxygen :
	doxygen Doxyfile


## Limpieza
clean :
	make --silent --directory=./sapt-src/ clean
	rm -rf $(BIN) $(DOC) *.o
