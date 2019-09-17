# g++ with support C++14 or higher (tested on g++6.3 and g++8.3)
CC = g++
CFLAGS = -Wall -c -std=c++14
LIBS = -pthread -latomic

all: server

server: main.o server.o command.o sequencecommand.o exportseqcommand.o
	$(CC) main.o server.o command.o sequencecommand.o exportseqcommand.o $(LIBS) -o server

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp

server.o: server.cpp
	$(CC) $(CFLAGS) server.cpp

command.o: command.cpp
	$(CC) $(CFLAGS) command.cpp

sequencecommand.o: sequencecommand.cpp
	$(CC) $(CFLAGS) sequencecommand.cpp

exportseqcommand.o: exportseqcommand.cpp
	$(CC) $(CFLAGS) exportseqcommand.cpp

clean:
	rm -rf *.o server

