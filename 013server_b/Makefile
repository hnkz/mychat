CC=gcc
C++=g++
LD=g++
CFLAGS=-c -Wall -pedantic-errors -O0 -g3 -std=c11  -I../
LDLIB=-lsqlite3 -lssl -lcrypto
OBJECTS=013server.o ../exp1lib.o db.o message_queue.o fds.o
EXECUTABLE=013server

all: $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@ $(LDLIB)

.cpp.o:
	$(C++) $(CFLAGS) $< -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -f ${EXECUTABLE} *.o core tmp.txt

