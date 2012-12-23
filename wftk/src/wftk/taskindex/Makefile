CC=gcc
XMLAPI=../../xmlapi
CFLAGS=-O2 -I$(XMLAPI) -DXML_NS

all: TASKINDEX_stdout.o

TASKINDEX_stdout.o: TASKINDEX_stdout.xml
	perl ../../lpml.pl TASKINDEX_stdout.xml
	$(CC) $(CFLAGS) -c -o TASKINDEX_stdout.o TASKINDEX_stdout.c

clean:
	rm -f *.o *.c~ TASKINDEX_stdout.c

