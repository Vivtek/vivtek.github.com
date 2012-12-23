CC=gcc
XMLAPI=../../xmlapi
CFLAGS=-O2 -I$(XMLAPI) -DXML_NS

all: PDREP_localxml.o

PDREP_localxml.o: PDREP_localxml.xml
	perl ../../lpml.pl PDREP_localxml.xml
	$(CC) $(CFLAGS) -c -o PDREP_localxml.o PDREP_localxml.c

clean:
	rm -f *.o *.c~ PDREP_localxml.c

