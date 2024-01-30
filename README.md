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
#include <stdio.h>
#include <stdlib.h>

void file_copy(const char *source, const char *destination) {
    char buffer[1024];
    size_t bytes;

    FILE *src = fopen(source, "rb");
    if (src == NULL) {
        perror("Error opening source file");
        exit(EXIT_FAILURE);
    }

    FILE *dest = fopen(destination, "wb");
    if (dest == NULL) {
        perror("Error opening destination file");
        fclose(src);
        exit(EXIT_FAILURE);
    }

    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes, dest);
    }

    fclose(src);
    fclose(dest);
}
#include <stdio.h>

extern void file_copy(const char *source, const char *destination);

int main() {
    const char *source_path = "source.txt";
    const char *destination_path = "destination.txt";

    file_copy(source_path, destination_path);

    printf("File has been copied successfully.\n");
    return 0;
}
