CC=gcc
EXPAT=../expat
XMLAPI=../xmlapi
CFLAGS=-O2 -I$(XMLAPI) -DXML_NS
XMLOBJS=$(EXPAT)/xmltok/xmltok.o \
  $(EXPAT)/xmltok/xmlrole.o \
  $(EXPAT)/xmlparse/xmlparse.o \
  $(EXPAT)/xmlparse/hashtable.o \
  $(XMLAPI)/xmlapi.o \
  $(XMLAPI)/xmlcgi.o

all: wftk_util

wftk_util: wftk_util.o wftk_lib.o $(XMLOBJS)
	$(CC) $(CFLAGS) -o $@ wftk_util.o wftk_lib.o $(XMLOBJS)

wftk_lib.o: wftk_lib.c
	$(CC) $(CFLAGS) -c -o wftk_lib.o wftk_lib.c

wftk_util.o: wftk_util.c
	$(CC) $(CFLAGS) -c -o wftk_util.o wftk_util.c

wftk_lib.c: wftk_lib.xml
	perl ../lpml.pl wftk_lib.xml
	/usr/local/bin/indent -kr -sob -i3 -lp -ts100 wftk_lib.c

wftk_util.c: wftk_util.xml
	perl ../lpml.pl wftk_util.xml
	/usr/local/bin/indent -kr -sob -i3 -lp -ts100 wftk_util.c


clean:
	rm -f *.o

