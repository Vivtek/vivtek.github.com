CC=gcc
XMLAPI=../../xmlapi
CFLAGS=-O2 -I$(XMLAPI) -DXML_NS

all: NOTIFY_smtp.o

NOTIFY_smtp.o: NOTIFY_smtp.xml
	perl ../../lpml.pl NOTIFY_smtp.xml
	$(CC) $(CFLAGS) -c -o NOTIFY_smtp.o NOTIFY_smtp.c

clean:
	rm -f *.o *.c~ NOTIFY_smtp.c

