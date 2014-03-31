COMPILER      = -cc
OPTIONS       = -std=c99 -pedantic -Wall -o
OPTIONS_LIBS  = -std=c99 -pedantic -Wall -c


all: main

main: main.c main.h
	$(COMPILER) $(OPTIONS_LIBS) GeneralHashFunctions.c

clean:
	rm -f core *.o *.bak *stackdump *
