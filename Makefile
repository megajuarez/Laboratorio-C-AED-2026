CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pedantic
LDFLAGS = -lm

ifeq ($(OS),Windows_NT)
TARGET = artemis_simulador.exe
else
TARGET = artemis_simulador
endif

all: $(TARGET)

$(TARGET): main.o artemis.o validaciones.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o artemis.o validaciones.o $(LDFLAGS)

main.o: main.c artemis.h validaciones.h
	$(CC) $(CFLAGS) -c main.c

artemis.o: artemis.c artemis.h validaciones.h
	$(CC) $(CFLAGS) -c artemis.c

validaciones.o: validaciones.c validaciones.h
	$(CC) $(CFLAGS) -c validaciones.c

clean:
ifeq ($(OS),Windows_NT)
	cmd /C "del /Q *.o $(TARGET) 2>NUL"
else
	rm -f *.o $(TARGET)
endif
