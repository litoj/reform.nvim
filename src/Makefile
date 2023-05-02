.PHONY: all clean docfmt

CC = gcc
CFLAGS = -O3 -pedantic -Wall
LDFLAGS = -fPIC -shared

HEAD = $(shell echo *.h)
SRC = $(HEAD:.h=.c)
OBJ = $(SRC:.c=.o)
TARGET = ../lua/reform/docfmt.so

docfmt: $(TARGET)

all: clean $(TARGET)

debug: CFLAGS=-pedantic -Wall -g -fsanitize=address,leak
debug: $(OBJ)
	$(CC) $(CFLAGS) -o main.out debug.c $(OBJ)


clean:
	rm -f docfmt.o main.out $(OBJ) || echo

$(TARGET): $(OBJ) docfmt.o
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) docfmt.o $(OBJ)

