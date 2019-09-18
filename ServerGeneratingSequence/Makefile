# g++ with support C++14 or higher (tested on g++6.3 and g++8.3)
CC            = gcc
CXX           = g++

CFLAGS        = -pipe -O2 -Wall -W -fPIC
CXXFLAGS      = -pipe -O2 -std=c++14 -Wall -W -fPIC

LIBS = -pthread -latomic
LINK          = g++
LFLAGS        = -Wl,-O1
INCPATH       = -I../ServerGeneratingSequence


SOURCES       = ../ServerGeneratingSequence/main.cpp \
		../ServerGeneratingSequence/server.cpp \
		../ServerGeneratingSequence/sequencecommand.cpp \
		../ServerGeneratingSequence/command.cpp \
		../ServerGeneratingSequence/exportseqcommand.cpp 

OBJECTS       = main.o \
		server.o \
		sequencecommand.o \
		command.o \
		exportseqcommand.o

TARGET = server

server:  $(OBJECTS)  
	$(CXX) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJCOMP) $(LIBS)

all: Makefile server

main.o: ../ServerGeneratingSequence/main.cpp ../ServerGeneratingSequence/server.hpp \
		../ServerGeneratingSequence/src/threadpool.hpp \
		../ServerGeneratingSequence/src/jointhreads.hpp \
		../ServerGeneratingSequence/src/lockfreequeue.hpp \
		../ServerGeneratingSequence/storage.hpp \
		../ServerGeneratingSequence/common.hpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o main.o ../ServerGeneratingSequence/main.cpp

server.o: ../ServerGeneratingSequence/server.cpp ../ServerGeneratingSequence/command.hpp \
		../ServerGeneratingSequence/server.hpp \
		../ServerGeneratingSequence/src/threadpool.hpp \
		../ServerGeneratingSequence/src/jointhreads.hpp \
		../ServerGeneratingSequence/src/lockfreequeue.hpp \
		../ServerGeneratingSequence/storage.hpp \
		../ServerGeneratingSequence/common.hpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o server.o ../ServerGeneratingSequence/server.cpp

sequencecommand.o: ../ServerGeneratingSequence/sequencecommand.cpp ../ServerGeneratingSequence/common.hpp \
		../ServerGeneratingSequence/server.hpp \
		../ServerGeneratingSequence/src/threadpool.hpp \
		../ServerGeneratingSequence/src/jointhreads.hpp \
		../ServerGeneratingSequence/src/lockfreequeue.hpp \
		../ServerGeneratingSequence/storage.hpp \
		../ServerGeneratingSequence/sequencecommand.hpp \
		../ServerGeneratingSequence/command.hpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o sequencecommand.o ../ServerGeneratingSequence/sequencecommand.cpp

command.o: ../ServerGeneratingSequence/command.cpp ../ServerGeneratingSequence/sequencecommand.hpp \
		../ServerGeneratingSequence/command.hpp \
		../ServerGeneratingSequence/exportseqcommand.hpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o command.o ../ServerGeneratingSequence/command.cpp

exportseqcommand.o: ../ServerGeneratingSequence/exportseqcommand.cpp ../ServerGeneratingSequence/server.hpp \
		../ServerGeneratingSequence/src/threadpool.hpp \
		../ServerGeneratingSequence/src/jointhreads.hpp \
		../ServerGeneratingSequence/src/lockfreequeue.hpp \
		../ServerGeneratingSequence/storage.hpp \
		../ServerGeneratingSequence/common.hpp \
		../ServerGeneratingSequence/exportseqcommand.hpp \
		../ServerGeneratingSequence/command.hpp
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o exportseqcommand.o ../ServerGeneratingSequence/exportseqcommand.cpp

clean:
	rm -rf *.o server

