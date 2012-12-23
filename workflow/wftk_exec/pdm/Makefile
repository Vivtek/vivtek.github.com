CC=gcc
EXPAT=../expat
XMLAPI=../xmlapi
CFLAGS=-O2 -I$(XMLAPI) -DXML_NS
XMLOBJS=$(EXPAT)/xmltok/xmltok.o \
  $(EXPAT)/xmltok/xmlrole.o \
  $(EXPAT)/xmlparse/xmlparse.o \
  $(EXPAT)/xmlparse/hashtable.o \
  $(XMLAPI)/xmlapi.o

all: pdm.cgi pdmlist

pdm.cgi: pdm.o $(XMLOBJS)
	$(CC) $(CFLAGS) -o $@ pdm.o ../user/user.o $(XMLOBJS)

pdmlist: pdmlist.o $(XMLOBJS)
	$(CC) $(CFLAGS) -o $@ pdmlist.o ../user/user.o $(XMLOBJS)

pdm.o: pdm.c
	$(CC) $(CFLAGS) -c -o pdm.o pdm.c

pdmlist.o: pdmlist.c
	$(CC) $(CFLAGS) -c -o pdmlist.o pdmlist.c

pdm.c: pdm.xml
	perl ../lpml.pl pdm.xml
	/usr/local/bin/indent -kr -sob -i3 -lp -ts100 pdm.c

pdmlist.c: pdmlist.xml
	perl ../lpml.pl pdmlist.xml
	/usr/local/bin/indent -kr -sob -i3 -lp -ts100 pdmlist.c

clean:
	rm -f *.o

