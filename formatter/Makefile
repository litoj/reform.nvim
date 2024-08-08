.PHONY: all clean formatter debug test testDir

MAKEFLAGS+=j8
CC:=gcc
CFLAGS+=-O3
LDFLAGS:=-fPIC -shared
SHELL:=/bin/bash

HEAD:=$(wildcard *.h)
SRC:=$(HEAD:.h=.c)
OBJ:=$(SRC:.c=.o)
TARGET:=../lua/reform/formatter.so

formatter:
	@[ ! -f main.out ] || $(MAKE) clean
	$(MAKE) $(TARGET)

$(TARGET): $(OBJ) formatter.o
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) formatter.o $(OBJ)

all:
	$(MAKE) clean
	$(MAKE) formatter

main.out: CFLAGS=-pedantic -Wall -g -fsanitize=address,leak -DDEBUG
main.out: $(OBJ)
	$(CC) $(CFLAGS) -o main.out formatter.c $(OBJ)

debug: main.out

testDir:
	@[ ! -d /tmp/reform.test/ ] || rm -r /tmp/reform.test/
	@mkdir -p /tmp/reform.test/

test/%in.md: debug testDir
	@./main.out $(shell echo $@ | sed 's/[_.].*in\.md//;s,test/,,') -r \
		< $@ > /tmp/reform.$(shell echo $@ | sed 's/in\./gen./')
	@-diff $(shell echo $@ | sed 's/in\./out./') /tmp/reform.$(shell echo $@ | sed 's/in\./gen./') \
		--color=never > /tmp/reform.$(shell echo $@ | sed 's/in\.md/diff/') \
		&& rm /tmp/reform.$(shell echo $@ | sed 's/in\.md//')* \
		|| echo diff in \'$(shell echo $@ | sed 's/\.in\.md//;s,test/,,')\'

test: $(wildcard ./test/*in.md)
	@rmdir /tmp/reform.test/ &> /dev/null

clean:
	@-rm -f *.o main.out
