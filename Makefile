# spxfont Makefile

SRC=test.c
EXE=spxtest
HEADER=spxfont.h
SCRIPT=build.sh

CC=gcc
STD=-std=c89
OPT=-O2
WFLAGS=-Wall -Wextra -pedantic
INC=-I.
LIB=-lfreetype

CFLAGS=$(STD) $(OPT) $(WFLAGS) $(INC) $(LIB)

$(EXE): $(SRC) $(HEADER)
	$(CC) $< -o $@ $(CFLAGS)

clean:
	$(RM) $(EXE)

install: $(SCRIPT)
	./$< $@

uninstall: $(SCRIPT)
	./$< $@
