#Makefile generico para varios objetivos.

CC     = gcc
CFLAGS = -I. -Wall -g

TARGET1 = agenetico
OBJS1 = main.o bayes.o patrones.o hash.o agenetico.o util.o
LDFLAGS1 = -lm

ALL_TARGETS = $(TARGET1)

all: $(ALL_TARGETS)

$(TARGET1): $(OBJS1)
	$(CC) $(CFLAGS) -o $(TARGET1) $(OBJS1) $(LDFLAGS1)


## Empaquetamos
tgz: clean
	tar zcvf navarro-sanchez-p6.tgz *


## Limpieza
clean:
	rm -f $(ALL_TARGETS) $(OBJS1)
