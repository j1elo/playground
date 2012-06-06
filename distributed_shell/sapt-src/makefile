CFLAGS =  -Wall -g 
COMPILER = g++

all: sapt

sock.o: sock.c sock.h
	@echo building sock.o...
	$(COMPILER) $(CFLAGS) -c sock.c

semaphore.o: semaphore.c semaphore.h
	@echo building semaphore.o...
	$(COMPILER) $(CFLAGS) -c semaphore.c

sapt.o:	sapt.cpp sock.h semaphore.h
	@echo building sapt.o
	$(COMPILER) $(CFLAGS) -c sapt.cpp

sapt:	sapt.o sock.o semaphore.o
	@echo building sapt
	$(COMPILER) $(CFLAGS) -o sapt sapt.o sock.o semaphore.o -lpthread

test:	sapt todo-im.txt 
	@echo -n "testing all everything... "
	@./sapt todo-im.txt

clean:
	rm -fr core *.o sapt
