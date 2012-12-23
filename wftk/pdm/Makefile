CC=gcc
EXPAT=/nd/local/src/expat
CFLAGS=-O2 -I$(EXPAT)/xmltok -I$(EXPAT)/xmlparse
EXPATOBJS=$(EXPAT)/xmltok/xmltok.o \
  $(EXPAT)/xmltok/xmlrole.o \
  $(EXPAT)/xmlparse/xmlparse.o \
  $(EXPAT)/xmlparse/hashtable.o

all: pdm

pdm: pdm.o $(EXPATOBJS)
	$(CC) $(CFLAGS) -o $@ $< $(EXPATOBJS) xmlapi.o

pdm.o: pdm.c
	$(CC) $(CFLAGS) -c -o pdm.o pdm.c

clean:
	rm -f pdm.o

