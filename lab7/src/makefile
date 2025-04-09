CC = gcc
CFLAGS = -Wall -O2

# Исходники
SRC = tcpserver.c tcpclient.c udpserver.c udpclient.c

# Целевые имена исполняемых файлов
TARGETS = tcpserver tcpclient udpserver udpclient

all: $(TARGETS)

tcpserver: tcpserver.c
	$(CC) $(CFLAGS) -o tcpserver tcpserver.c

tcpclient: tcpclient.c
	$(CC) $(CFLAGS) -o tcpclient tcpclient.c

udpserver: udpserver.c
	$(CC) $(CFLAGS) -o udpserver udpserver.c

udpclient: udpclient.c
	$(CC) $(CFLAGS) -o udpclient udpclient.c

clean:
	rm -f $(TARGETS)
