CC=gcc
XMLAPI=../../xmlapi
CFLAGS=-O2 -I$(XMLAPI) -DXML_NS

all: PERMS_localxml.o

PERMS_localxml.o: PERMS_localxml.xml
	perl ../../lpml.pl PERMS_localxml.xml
	$(CC) $(CFLAGS) -c -o PERMS_localxml.o PERMS_localxml.c

clean:
	rm -f *.o *.c~ PERMS_localxml.c

