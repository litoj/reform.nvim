.PHONY: all clean docfmt

CC = gcc
CFLAGS = -O2 -fPIC -pedantic -Wall
LDFLAGS = -shared

SRC = $(shell echo *.c)
OBJ = $(SRC:.c=.o)
TARGET = ../lua/reform/docfmt.so

docfmt: $(TARGET)

all: clean $(TARGET)

clean:
	rm -f $(OBJ) $(TARGET) || echo

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $(OBJ)

