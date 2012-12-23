CC=gcc
XMLAPI=../../xmlapi
CFLAGS=-O2 -I$(XMLAPI) -DXML_NS

all: USER_localxml.o

USER_localxml.o: USER_localxml.xml
	perl ../../lpml.pl USER_localxml.xml
	$(CC) $(CFLAGS) -c -o USER_localxml.o USER_localxml.c

clean:
	rm -f *.o *.c~ USER_localxml.c

