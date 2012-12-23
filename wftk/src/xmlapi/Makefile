CC=gcc
EXPAT=../expat
CFLAGS=-O2 -I$(EXPAT)/xmltok -I$(EXPAT)/xmlparse
EXPATOBJS=$(EXPAT)/xmltok/xmltok.o \
  $(EXPAT)/xmltok/xmlrole.o \
  $(EXPAT)/xmlparse/xmlparse.o \
  $(EXPAT)/xmlparse/hashtable.o

all:  xmlapi.o xmlcgi.o tester

xmlapi.c: xmlapi.xml
	perl ../lpml.pl xmlapi.xml
	./pretty_up xmlapi.c

xmlapi.o: xmlapi.c
	$(CC) -c $(CFLAGS) -o $@ $<

xmlcgi.c: xmlcgi.xml
	perl ../lpml.pl xmlcgi.xml
	./pretty_up xmlcgi.c

xmlcgi.o: xmlcgi.c
	$(CC) -c $(CFLAGS) -o $@ $<

tester: test.o $(EXPATOBJS)
	$(CC) $(CFLAGS) -o $@ $< $(EXPATOBJS) xmlapi.o

test.o: test.c
	$(CC) $(CFLAGS) -c -o test.o test.c

clean:
	rm -f *.o tester *.c~ xmlapi.c xmlcgi.c

