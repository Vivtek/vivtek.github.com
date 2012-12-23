CC=gcc
XMLAPI=../../xmlapi
CFLAGS=-O2 -I$(XMLAPI) -DXML_NS

all: DATATYPE_option.o

DATATYPE_option.o: DATATYPE_option.xml
	perl ../../lpml.pl DATATYPE_option.xml
	$(CC) $(CFLAGS) -c -o DATATYPE_option.o DATATYPE_option.c

clean:
	rm -f *.o *.c~ DATATYPE_option.c

