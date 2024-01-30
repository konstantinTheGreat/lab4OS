CC=gcc
CFLAGS=-Wall

all: astral_test

astral_test: astral_test.c libastral.a
 $(CC) $(CFLAGS) -o astral_test astral_test.c -L. -lastral

libastral.a: libastral.o
 ar rcs libastral.a libastral.o

libastral.o: libastral.c libastral.h
 $(CC) $(CFLAGS) -c libastral.c

clean:
 rm -f *.o *.a astral_test
