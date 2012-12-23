CC=gcc
EXPAT=../expat
CFLAGS=-O2 -I$(EXPAT)/xmltok -I$(EXPAT)/xmlparse
EXPATOBJS=$(EXPAT)/xmltok/xmltok.o \
  $(EXPAT)/xmltok/xmlrole.o \
  $(EXPAT)/xmlparse/xmlparse.o \
  $(EXPAT)/xmlparse/hashtable.o

all:  xmlapi.o xmlcgi.o tester

xmlapi.o: xmlapi.xml
	perl ../lpml.pl xmlapi.xml
	$(CC) -c $(CFLAGS) -o $@ $<

xmlcgi.o: xmlcgi.xml
	perl ../lpml.pl xmlcgi.xml
	$(CC) -c $(CFLAGS) -o $@ $<

tester: test.o $(EXPATOBJS)
	$(CC) $(CFLAGS) -o $@ $< $(EXPATOBJS) xmlapi.o

test.o: test.c
	$(CC) $(CFLAGS) -c -o test.o test.c

clean:
	rm -f *.o tester *.c~ xmlapi.c xmlcgi.c

