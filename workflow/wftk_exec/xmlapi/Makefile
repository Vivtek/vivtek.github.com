CC=gcc
EXPAT=/nd/local/src/expat
CFLAGS=-O2 -I$(EXPAT)/xmltok -I$(EXPAT)/xmlparse
EXPATOBJS=$(EXPAT)/xmltok/xmltok.o \
  $(EXPAT)/xmltok/xmlrole.o \
  $(EXPAT)/xmlparse/xmlparse.o \
  $(EXPAT)/xmlparse/hashtable.o

all:  test

test: test.o $(EXPATOBJS)
	$(CC) $(CFLAGS) -o $@ $< $(EXPATOBJS) xmlapi.o

test.o: test.c
	$(CC) $(CFLAGS) -c -o test.o test.c

clean:
	rm -f *.o

