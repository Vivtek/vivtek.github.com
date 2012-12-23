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

all: user browse.cgi user_edit.cgi

user: main.o user.o $(XMLOBJS)
	$(CC) $(CFLAGS) -o $@ main.o user.o $(XMLOBJS)

user.o: user.c
	$(CC) $(CFLAGS) -c -o user.o user.c

main.o: main.c
	$(CC) $(CFLAGS) -c -o main.o main.c

user.c: user.xml
	perl ../lpml.pl user.xml
	cp user.h ..

main.c: user.xml
	perl ../lpml.pl user.xml
	cp user.h ..

browse.cgi: browse.o user.o $(XMLOBJS)
	$(CC) $(CFLAGS) -o browse.cgi browse.o user.o $(XMLOBJS)
browse.o: browse.c
	$(CC) $(CFLAGS) -c -o browse.o browse.c

browse.c: browse.xml
	perl ../lpml.pl browse.xml


user_edit.cgi: user_edit.o user.o $(XMLOBJS)
	$(CC) $(CFLAGS) -o user_edit.cgi user_edit.o user.o $(XMLOBJS)
user_edit.o: user_edit.c
	$(CC) $(CFLAGS) -c -o user_edit.o user_edit.c

user_edit.c: user_edit.xml
	perl ../lpml.pl user_edit.xml


clean:
	rm -f *.o

