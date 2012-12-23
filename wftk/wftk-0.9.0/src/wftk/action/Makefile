CC=gcc
XMLAPI=../../xmlapi
CFLAGS=-O2 -I$(XMLAPI) -DXML_NS

all: ACTION_wftk.o ACTION_system.o

ACTION_wftk.o: ACTION_wftk.xml
	perl ../../lpml.pl ACTION_wftk.xml
	$(CC) $(CFLAGS) -c -o ACTION_wftk.o ACTION_wftk.c

ACTION_system.o: ACTION_system.xml
	perl ../../lpml.pl ACTION_system.xml
	$(CC) $(CFLAGS) -c -o ACTION_system.o ACTION_system.c

clean:
	rm -f *.o *.c~ ACTION_wftk.c ACTION_system.c

