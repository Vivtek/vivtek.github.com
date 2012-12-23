CC=gcc
XMLAPI=../../xmlapi
CFLAGS=-O2 -I$(XMLAPI) -DXML_NS

all: DSREP_localxml.o

DSREP_localxml.o: DSREP_localxml.xml
	perl ../../lpml.pl DSREP_localxml.xml
	$(CC) $(CFLAGS) -c -o DSREP_localxml.o DSREP_localxml.c

clean:
	rm -f *.o *.c~ DSREP_localxml.c

