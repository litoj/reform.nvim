.PHONY: all clean docfmt

CC = gcc
CFLAGS = -O2 -fPIC -pedantic -Wall
LDFLAGS = -shared

HEAD = $(shell echo *.h)
SRC = $(HEAD:.h=.c)
OBJ = $(SRC:.c=.o)
TARGET = ../lua/reform/docfmt.so

docfmt: $(TARGET)

all: clean $(TARGET)

debug: $(OBJ)
	$(CC) $(CFLAGS) -g -fsanitize=address,leak -o debug.out debug.c $(OBJ)


clean:
	rm -f docfmt.o debug.out $(OBJ) $(TARGET) || echo

$(TARGET): $(OBJ) docfmt.o
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) docfmt.o $(OBJ)

