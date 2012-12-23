CC=gcc
EXPAT=/nd/local/src/expat
CFLAGS=-O2 -I$(EXPAT)/xmltok -I$(EXPAT)/xmlparse -DXML_NS
EXPATOBJS=$(EXPAT)/xmltok/xmltok.o \
  $(EXPAT)/xmltok/xmlrole.o \
  $(EXPAT)/xmlparse/xmlparse.o \
  $(EXPAT)/xmlparse/hashtable.o

all: xmlsnip xmlreplace xmlset xmlinsert xmlcreate

xmlsnip: xmlsnip.o $(EXPATOBJS)
	$(CC) $(CFLAGS) -o $@ $< $(EXPATOBJS)

xmlsnip.o: xmltools.c
	$(CC) $(CFLAGS) -DXMLSNIP -c -o xmlsnip.o xmltools.c

xmlreplace: xmlreplace.o $(EXPATOBJS)
	$(CC) $(CFLAGS) -o $@ $< $(EXPATOBJS)

xmlreplace.o: xmltools.c
	$(CC) $(CFLAGS) -DXMLREPLACE -c -o xmlreplace.o xmltools.c

xmlset: xmlset.o $(EXPATOBJS)
	$(CC) $(CFLAGS) -o $@ $< $(EXPATOBJS)

xmlset.o: xmltools.c
	$(CC) $(CFLAGS) -DXMLSET -c -o xmlset.o xmltools.c

xmlinsert: xmlinsert.o $(EXPATOBJS)
	$(CC) $(CFLAGS) -o $@ $< $(EXPATOBJS)

xmlinsert.o: xmltools.c
	$(CC) $(CFLAGS) -DXMLINSERT -c -o xmlinsert.o xmltools.c

xmlcreate: xmlcreate.o
	$(CC) $(CFLAGS) -o $@ $<

xmlcreate.o: xmlcreate.c
	$(CC) $(CFLAGS) -c -o xmlcreate.o xmlcreate.c

clean:
	rm -f *.o

