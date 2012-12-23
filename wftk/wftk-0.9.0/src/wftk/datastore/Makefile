CC=gcc
XMLAPI=../../xmlapi
CFLAGS=-O2 -I$(XMLAPI) -DXML_NS

all: DATASTORE_role.o

DATASTORE_role.o: DATASTORE_role.xml
	perl ../../lpml.pl DATASTORE_role.xml
	$(CC) $(CFLAGS) -c -o DATASTORE_role.o DATASTORE_role.c

clean:
	rm -f *.o *.c~ DATASTORE_role.c

