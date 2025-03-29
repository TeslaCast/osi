all: client server

server: libMultModulo.a server.o
	gcc -o server server.o -L. -lMultModulo -lpthread 

client: libMultModulo.a client.o
	gcc -o client client.o -L. -lMultModulo -lpthread 

server.o: server.c
	gcc -c server.c  

client.o: client.c
	gcc -c client.c  

libMultModulo.a: MultModulo.o
	ar cr libMultModulo.a MultModulo.o

MultModulo.o : MultModulo.h
	gcc -c MultModulo.c 

clean :
	rm client server libMultModulo.o
