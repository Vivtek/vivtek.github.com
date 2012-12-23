CC=gcc
EXPAT=/nd/local/src/expat
CFLAGS=-O2 -I$(EXPAT)/xmltok -I$(EXPAT)/xmlparse -DXML_NS
EXPATOBJS=$(EXPAT)/xmltok/xmltok.o \
  $(EXPAT)/xmltok/xmlrole.o \
  $(EXPAT)/xmlparse/xmlparse.o \
  $(EXPAT)/xmlparse/hashtable.o

all: wftk

wftk: wftk.o $(EXPATOBJS)
	$(CC) $(CFLAGS) -o $@ $< $(EXPATOBJS)

wftk.o: wftk.c
	$(CC) $(CFLAGS) -DXMLSNIP -c -o wftk.o wftk.c

clean:
	rm -f *.o

