.phony all:
all: pman

pman: PMan.c helpers.o helpers.h
	gcc -Wall PMan.c helpers.o -lreadline -o PMan -g


.PHONY clean:
clean:
	-rm -rf *.o *.exe
