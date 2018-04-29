CC=gcc
C++=g++
LD=g++
CFLAGS=-c -Wall -pedantic-errors -O0 -g3 -std=c11 -I../
LDLIB= -lssl -lcrypto
LDFLAGS=
OBJECTS=013client.o ../exp1lib.o
EXECUTABLE=013client

all: $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(LD) $(LDFLAGS) $(OBJECTS) -o $@ $(LDLIB)

.cpp.o:
	$(C++) $(CFLAGS) $< -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -f ${EXECUTABLE} *.o core tmp.txt

