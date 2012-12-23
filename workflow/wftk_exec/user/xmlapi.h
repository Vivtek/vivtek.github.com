typedef struct _element XML;
typedef struct _attr ATTR;

typedef struct _list ELEMENTLIST;

struct _element {
   char * name;
   ATTR * attrs;
   XML * parent;
   ELEMENTLIST * children;
};

struct _attr {
   char * name;
   char * value;
   ATTR * next;
};

struct _list {
   XML * element;
   ELEMENTLIST * next;
   ELEMENTLIST * prev;
};
void xml_write (FILE * file, XML * xml);
void xml_writecontent (FILE * file, XML * xml);
void xml_prepend (XML * parent, XML * child);
void xml_append (XML * parent, XML * child);
XML * xml_loc (XML * start, const char * loc);
void xml_getloc (XML * xml, char *loc, int len);
void xml_set (XML * xml, const char * name, const char * value);
void xml_setnum (XML * xml, const char *attr, int number);
const char * xml_attrval (XML * element,const char * name);
int xml_attrvalnum (XML * element,const char * name);
XML * xml_create (const char * name);
XML * xml_createtext (const char * value);
XML * xml_createtextlen (const char * value, int len);
void xml_free (XML * xml);
void xml_delete(XML * piece);
XML * xml_first (XML * xml);
XML * xml_firstelem (XML * xml);
XML * xml_last (XML * xml);
XML * xml_lastelem (XML * xml);
XML * xml_next (XML * xml);
XML * xml_nextelem (XML * xml);
XML * xml_prev (XML * xml);
XML * xml_prevelem (XML * xml);
XML * xml_read (FILE * file);
