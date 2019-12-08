
CC=gcc
CFLAGS= -Wall -pedantic -O2 -fomit-frame-pointer # -m486
LDFLAGS= -s

all: lyxass


.c.o: 
	$(CC) $(CFLAGS) -c $<

OBJECTS: lyxass.o parser.o debug.o label.o\
	opcode.o pseudo.o mnemonics.o expression.o\
	macro.o ref.o error.o

lyxass: lyxass.o parser.o debug.o label.o opcode.o pseudo.o mnemonics.o expression.o\
	macro.o ref.o error.o jaguar.o
	$(CC) $(LDFLAGS) $+ -o $@

ALL=global_vars.h error.h my.h #Makefile

lyxass.o : lyxass.c label.h opcode.h pseudo.h mnemonics.h $(ALL)
parser.o : parser.c parser.h $(ALL)
pseudo.o : pseudo.c pseudo.h opcode.h parser.h $(ALL)
label.o  : label.c label.h $(ALL)
mnemonics.o : mnemonics.c mnemonics.h jaguar.h parser.h $(ALL)
expression.o : expression.c $(ALL)
ref.o : ref.c $(ALL)
macro.o : macro.c $(ALL)
error.o : error.c $(ALL)
debug.o : debug.c $(ALL)
opcode.o : opcode.c $(ALL)
jaguar.o : jaguar.c jaguar.h $(ALL)

clean:
	rm -f *.o
	rm -f lyxass
	rm -f lyxass.exe
	rm -f *~
