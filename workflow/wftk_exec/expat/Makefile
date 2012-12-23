CC=gcc
# If you know what your system's byte order is, define XML_BYTE_ORDER:
# use -DXML_BYTE_ORDER=12 for little-endian byte order;
# use -DXML_BYTE_ORDER=21 for big-endian (network) byte order.
# -DXML_NS adds support for checking of lexical aspects of XML namespaces spec
# -DXML_MIN_SIZE makes a smaller but slower parser
CFLAGS=-O2 -Ixmltok -Ixmlparse -DXML_NS
# Use one of the next two lines; unixfilemap is better if it works.
FILEMAP_OBJ=xmlwf/unixfilemap.o
#FILEMAP_OBJ=xmlwf/readfilemap.o
EXPATOBJS=xmltok/xmltok.o \
  xmltok/xmlrole.o \
  xmlparse/xmlparse.o \
  xmlparse/hashtable.o
WFOBJS=xmlwf/xmlwf.o \
  xmlwf/xmlfile.o \
  xmlwf/codepage.o \
  $(FILEMAP_OBJ)
EXE=

all: xmlwf/xmlwf$(EXE) bin/elements$(EXE)

xmlwf/xmlwf$(EXE): $(WFOBJS) $(EXPATOBJS)
	$(CC) $(CFLAGS) -o $@ $< $(EXPATOBJS)

bin/elements$(EXE): sample/elements.o $(EXPATOBJS)
	$(CC) $(CFLAGS) -o $@ $< $(EXPATOBJS)

clean:
	rm -f $(WFOBJS) $(EXPATOBJS) xmlwf/xmlwf$(EXE)

xmltok/nametab.h: gennmtab/gennmtab$(EXE)
	rm -f $@
	gennmtab/gennmtab$(EXE) >$@

gennmtab/gennmtab$(EXE): gennmtab/gennmtab.c
	$(CC) $(CFLAGS) -o $@ gennmtab/gennmtab.c

xmltok/xmltok.o: xmltok/nametab.h

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<
