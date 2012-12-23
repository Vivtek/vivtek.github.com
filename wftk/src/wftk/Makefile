CC=gcc
EXPAT=../expat
XMLAPI=../xmlapi
CFLAGS=-O2 -I$(XMLAPI) -DXML_NS
XMLOBJS=$(EXPAT)/xmltok/xmltok.o \
  $(EXPAT)/xmltok/xmlrole.o \
  $(EXPAT)/xmlparse/xmlparse.o \
  $(EXPAT)/xmlparse/hashtable.o \
  $(XMLAPI)/xmlapi.o
ADAPTORS=\
  dsrep/DSREP_localxml.o \
  pdrep/PDREP_localxml.o \
  datastore/DATASTORE_role.o \
  datatype/DATATYPE_option.o \
  notify/NOTIFY_smtp.o \
  perms/PERMS_localxml.o \
  taskindex/TASKINDEX_stdout.o \
  user/USER_localxml.o \
  action/ACTION_wftk.o \
  action/ACTION_system.o \

# The following is required for Solaris.  Other Unices either don't need this or need -lsocket.  Please tell me.
SOCKETLIB=-lnsl -lsocket



all: 
	$(MAKE) -C dsrep all
	$(MAKE) -C pdrep all
	$(MAKE) -C datastore all
	$(MAKE) -C datatype all
	$(MAKE) -C notify all
	$(MAKE) -C perms all
	$(MAKE) -C taskindex all
	$(MAKE) -C user all
	$(MAKE) -C action all
	$(MAKE) wftk

wftk: wftk_util.o wftk_lib.o wftk_adaptors.o config.o $(XMLOBJS) $(ADAPTORS)
	$(CC) $(CFLAGS) -o $@ wftk_util.o wftk_lib.o wftk_adaptors.o config.o $(XMLOBJS) $(ADAPTORS) $(SOCKETLIB)

wftk_lib.o: wftk_lib.xml
	perl ../lpml.pl wftk_lib.xml
	$(CC) $(CFLAGS) -c -o wftk_lib.o wftk_lib.c

wftk_util.o: wftk_util.xml
	perl ../lpml.pl wftk_util.xml
	$(CC) $(CFLAGS) -c -o wftk_util.o wftk_util.c

wftk_adaptors.o: wftk_adaptors.xml
	perl ../lpml.pl wftk_adaptors.xml
	$(CC) $(CFLAGS) -c -o wftk_adaptors.o wftk_adaptors.c

config.o: wftk_config.xml
	perl ../lpml.pl wftk_config.xml
	$(CC) $(CFLAGS) -c -o config.o config.c


clean:
	rm -f *.o *.c~ wftk_lib.c wftk_util.c wftk_adaptors.c config.c
	$(MAKE) -C dsrep clean
	$(MAKE) -C pdrep clean
	$(MAKE) -C datastore clean
	$(MAKE) -C datatype clean
	$(MAKE) -C notify clean
	$(MAKE) -C perms clean
	$(MAKE) -C taskindex clean
	$(MAKE) -C user clean
	$(MAKE) -C action clean

