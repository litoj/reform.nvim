.PHONY: all clean docfmt

MAKEFLAGS+=-j4
CC=gcc
CFLAGS+=-O3
LDFLAGS=-fPIC -shared

HEAD=$(shell echo *.h)
SRC=$(HEAD:.h=.c)
OBJ=$(SRC:.c=.o)
TARGET=../lua/reform/docfmt.so

docfmt: $(TARGET)

$(TARGET): $(OBJ) docfmt.o
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) docfmt.o $(OBJ)

all: clean $(TARGET)

debug: CFLAGS=-pedantic -Wall -g -fsanitize=address,leak -DDEBUG
debug: $(OBJ)
	$(CC) $(CFLAGS) -o main.out docfmt.c $(OBJ)

clean:
	@rm -f *.o main.out $(TARGER)


