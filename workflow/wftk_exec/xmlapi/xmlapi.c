#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "xmlparse.h"
#include "xmlapi.h"

XML *xml_create(const char *name)
{
   XML *ret;

   ret = (XML *) malloc(sizeof(struct _element));
   ret->name = (char *) malloc(strlen(name) + 1);
   strcpy(ret->name, name);
   ret->attrs = NULL;
   ret->children = NULL;
   ret->parent = NULL;

   return (ret);
}
void xml_set(XML * xml, const char *name, const char *value)
{
   ATTR *attr;

   attr = xml->attrs;
   while (attr) {
      if (!strcmp(attr->name, name))
         break;
      attr = attr->next;
   }

   if (attr) {
      free((void *) (attr->value));
      attr->value = (char *) malloc(strlen(value) + 1);
      strcpy(attr->value, value);
      return;
   }

   if (xml->attrs == NULL) {
      attr = (ATTR *) malloc(sizeof(struct _attr));
      xml->attrs = attr;
   } else {
      attr = xml->attrs;
      while (attr->next)
         attr = attr->next;
      attr->next = (ATTR *) malloc(sizeof(struct _attr));
      attr = attr->next;
   }

   attr->next = NULL;
   attr->name = (char *) malloc(strlen(name) + 1);
   strcpy(attr->name, name);
   attr->value = (char *) malloc(strlen(value) + 1);
   strcpy(attr->value, value);
}
void xml_setnum(XML * xml, const char *attr, int number)
{
   char buf[sizeof(number) * 3 + 1];
   sprintf(buf, "%d", number);
   xml_set(xml, attr, buf);
}
const char *xml_attrval(XML * element, const char *name)
{
   ATTR *attr;

   attr = element->attrs;
   while (attr) {
      if (!strcmp(attr->name, name))
         return (attr->value);
      attr = attr->next;
   }
   return ("");
}
int xml_attrvalnum(XML * element, const char *name)
{
   return (atoi(xml_attrval(element, name)));
}

void xml_prepend(XML * parent, XML * child)
{
   ELEMENTLIST *list;

   child->parent = parent;

   list = (ELEMENTLIST *) malloc(sizeof(struct _list));
   list->element = child;
   list->prev = NULL;
   list->next = parent->children;
   parent->children = list;
}

void xml_append(XML * parent, XML * child)
{
   ELEMENTLIST *list;
   ELEMENTLIST *ch;

   child->parent = parent;

   list = (ELEMENTLIST *) malloc(sizeof(struct _list));
   list->element = child;
   list->prev = NULL;
   list->next = NULL;

   if (parent->children == NULL) {
      parent->children = list;
      return;
   }

   ch = parent->children;
   while (ch->next != NULL)
      ch = ch->next;
   list->prev = ch;
   ch->next = list;
}
XML *xml_createtext(const char *value)
{
   XML *ret;

   ret = (XML *) malloc(sizeof(struct _element));
   ret->name = NULL;
   ret->children = NULL;
   ret->parent = NULL;
   ret->attrs = (ATTR *) malloc(sizeof(struct _attr));
   ret->attrs->name = NULL;
   ret->attrs->next = NULL;
   ret->attrs->value = (char *) malloc(strlen(value) + 1);
   strcpy(ret->attrs->value, value);

   return (ret);
}

XML *xml_createtextlen(const char *value, int len)
{
   XML *ret;

   ret = (XML *) malloc(sizeof(struct _element));
   ret->name = NULL;
   ret->children = NULL;
   ret->attrs = (ATTR *) malloc(sizeof(struct _attr));
   ret->attrs->name = NULL;
   ret->attrs->next = NULL;
   ret->attrs->value = (char *) malloc(len + 1);
   strncpy(ret->attrs->value, value, len);
   ret->attrs->value[len] = '\0';

   return (ret);
}

void xml_write(FILE * file, XML * xml)
{
   ATTR *attr;
   ELEMENTLIST *list;
   if (xml->name == NULL) {
      fprintf(file, "%s", xml->attrs->value);
      return;
   }
   fprintf(file, "<%s", xml->name);
   attr = xml->attrs;
   while (attr != NULL) {
      fprintf(file, " %s=\"%s\"", attr->name, attr->value);
      attr = attr->next;
   }
   if (xml->children == NULL) {
      fprintf(file, "/>");
      return;
   } else
      fprintf(file, ">");
   xml_writecontent(file, xml);
   fprintf(file, "</%s>", xml->name);
}

void xml_writecontent(FILE * file, XML * xml)
{
   ELEMENTLIST *list;

   list = xml->children;
   while (list) {
      xml_write(file, list->element);
      list = list->next;
   }
}
void xml_free(XML * xml)
{
   ATTR *attr;
   ELEMENTLIST *list;

   if (xml == NULL)
      return;

   if (xml->name != NULL)
      free((void *) (xml->name));
   while (xml->attrs) {
      attr = xml->attrs;
      xml->attrs = xml->attrs->next;
      if (attr->name != NULL)
         free((void *) (attr->name));
      if (attr->value != NULL)
         free((void *) (attr->value));
      xml->attrs = attr->next;
      free((void *) attr);
   }

   while (xml->children) {
      list = xml->children;
      xml->children = list->next;
      if (list->element != NULL)
         xml_free(list->element);
      free((void *) list);
   }
   free((void *) xml);
}

void xml_delete(XML * piece)
{
   ELEMENTLIST *list;
   if (!piece)
      return;
   if (piece->parent != NULL) {
      list = piece->parent->children;
      while (list != NULL && list->element != piece)
         list = list->next;
      if (list != NULL) {
         if (list->next != NULL)
            list->next->prev = list->prev;
         if (list->prev != NULL)
            list->prev->next = list->next;
      }
      if (list == piece->parent->children)
         piece->parent->children = list->next;
      free((void *) list);
   }
   xml_free(piece);
}

XML *xml_first(XML * xml)
{
   if (xml == NULL)
      return NULL;
   if (xml->children == NULL)
      return NULL;
   return (xml->children->element);
}

XML *xml_firstelem(XML * xml)
{
   ELEMENTLIST *list;
   if (xml == NULL)
      return NULL;
   list = xml->children;
   while (list != NULL) {
      if (list->element->name != NULL)
         break;
      list = list->next;
   }
   if (list != NULL)
      return (list->element);
   return NULL;
}

XML *xml_last(XML * xml)
{
   ELEMENTLIST *list;
   list = xml->children;
   if (list == NULL)
      return NULL;
   while (list->next != NULL)
      list = list->next;
   return (list->element);
}

XML *xml_lastelem(XML * xml)
{
   ELEMENTLIST *list;
   list = xml->children;
   if (list == NULL)
      return NULL;
   while (list->next != NULL)
      list = list->next;
   while (list != NULL) {
      if (list->element->name != NULL)
         break;
      list = list->prev;
   }
   if (list != NULL)
      return (list->element);
   return NULL;
}

XML *xml_next(XML * xml)
{
   ELEMENTLIST *list;
   if (xml == NULL)
      return (NULL);
   if (xml->parent == NULL)
      return (NULL);
   list = xml->parent->children;
   while (list != NULL && list->element != xml)
      list = list->next;
   if (list == NULL)
      return (NULL);
   if (list->next == NULL)
      return (NULL);
   return (list->next->element);
}

XML *xml_nextelem(XML * xml)
{
   ELEMENTLIST *list;
   if (xml == NULL)
      return (NULL);
   if (xml->parent == NULL)
      return (NULL);
   list = xml->parent->children;
   while (list != NULL && list->element != xml)
      list = list->next;
   if (list == NULL)
      return (NULL);
   while (list->next != NULL) {
      if (list->next->element->name != NULL)
         break;
      list = list->next;
   }
   if (list->next == NULL)
      return (NULL);
   return (list->next->element);
}

XML *xml_prev(XML * xml)
{
   ELEMENTLIST *list;
   if (xml == NULL)
      return (NULL);
   if (xml->parent == NULL)
      return (NULL);
   list = xml->parent->children;
   while (list != NULL && list->element != xml)
      list = list->next;
   if (list == NULL)
      return (NULL);
   if (list->prev == NULL)
      return (NULL);
   return (list->prev->element);
}

XML *xml_prevelem(XML * xml)
{
   ELEMENTLIST *list;
   if (xml == NULL)
      return (NULL);
   if (xml->parent == NULL)
      return (NULL);
   list = xml->parent->children;
   while (list != NULL && list->element != xml)
      list = list->next;
   if (list == NULL)
      return (NULL);
   while (list->prev != NULL) {
      if (list->prev->element->name != NULL)
         break;
      list = list->prev;
   }
   if (list->prev == NULL)
      return (NULL);
   return (list->prev->element);
}

XML *xml_loc(XML * start, const char *loc)
{
   char *mark;
   const char *attrval;
   char piece[64];
   int i;
   int count;

   if (!loc)
      return (start);
   if (!*loc)
      return (start);

   while (start && start->name == NULL)
      start = xml_next(start);
   if (!start)
      return (NULL);

   while (*loc == ' ')
      loc++;
   i = 0;
   while (*loc && *loc != '.')
      piece[i++] = *loc++;
   piece[i] = '\0';
   if (*loc)
      loc++;
   while (*loc == ' ')
      loc++;

   mark = strchr(piece, ']');
   if (mark)
      *mark = '\0';
   mark = strchr(piece, '(');
   if (mark) {
      *mark++ = '\0';
      count = atoi(mark);
      mark = NULL;
   } else {
      count = 0;
      mark = strchr(piece, '[');
      if (mark) {
         *mark++ = '\0';
      }
   }

   while (start) {
      if (start->name == NULL) {
         start = xml_next(start);
         continue;
      }
      if (strcmp(start->name, piece)) {
         start = xml_next(start);
         continue;
      }
      if (count) {
         count--;
         start = xml_next(start);
         continue;
      }
      if (!mark) {
         if (*loc)
            return (xml_loc(xml_first(start), loc));
         return (start);
      }
      attrval = xml_attrval(start, "id");
      if (attrval) {
         if (strcmp(attrval, mark)) {
            start = xml_next(start);
            continue;
         }
         if (*loc)
            return (xml_loc(xml_first(start), loc));
         return (start);
      }
      attrval = xml_attrval(start, "name");
      if (attrval) {
         if (strcmp(attrval, mark)) {
            start = xml_next(start);
            continue;
         }
         if (*loc)
            return (xml_loc(xml_first(start), loc));
         return (start);
      }
   }
   return (NULL);
}
void xml_getloc(XML * xml, char *loc, int len)
{
   int s;
   int count;
   XML *sib;
   if (xml->parent != NULL) {
      xml_getloc(xml->parent, loc, len);
   } else {
      *loc = '\0';
   }
   s = strlen(loc);
   if (s > 0 && s < len - 1) {
      strcat(loc, ".");
      s++;
   }
   len -= s;
   loc += s;
   if (strlen(xml->name) < len) {
      strcpy(loc, xml->name);
   } else {
      strncpy(loc, xml->name, len - 1);
      loc[len - 1] = '\0';
   }
   if (xml->parent == NULL)
      return;
   sib = xml_first(xml->parent);
   count = 0;
   while (sib != xml && sib != NULL) {
      if (sib->name != NULL) {
         if (!strcmp(sib->name, xml->name))
            count++;
      }
      sib = xml_next(sib);
   }
   if (count > 0 && s > 4) {
      strcat(loc, "(");
      sprintf(loc + strlen(loc), "%d", count);
      strcat(loc, ")");
   }
}
void startElement(void *userData, const char *name, const char **atts)
{
   XML **parent;
   XML *element;

   element = xml_create(name);
   while (*atts) {
      xml_set(element, atts[0], atts[1]);
      atts += 2;
   }

   parent = (XML **) userData;
   if (*parent != NULL)
      xml_append(*parent, element);
   *parent = element;
}
void endElement(void *userData, const char *name)
{
   XML **element;

   element = (XML **) userData;
   if ((*element)->parent != NULL)
      *element = (*element)->parent;
}
void charData(void *userData, const XML_Char * s, int len)
{
   XML **parent;

   parent = (XML **) userData;
   xml_append(*parent, xml_createtextlen((char *) s, len));
}

XML *xml_read(FILE * file)
{
   XML_Parser parser;
   char buf[BUFSIZ];
   int done;
   XML *ret;

   ret = NULL;
   parser = XML_ParserCreate(NULL);

   XML_SetUserData(parser, (void *) &ret);

   XML_SetElementHandler(parser, startElement, endElement);
   XML_SetCharacterDataHandler(parser, charData);

   done = 0;

   do {
      size_t len = fread(buf, 1, sizeof(buf), file);
      done = len < sizeof(buf);
      if (!XML_Parse(parser, buf, len, done)) {
         fprintf(stderr, "Error in xml_read: %s at line %d\n",
                 XML_ErrorString(XML_GetErrorCode(parser)),
                 XML_GetCurrentLineNumber(parser));
         xml_free(ret);
         return NULL;
      }
   } while (!done);
   XML_ParserFree(parser);

   return (ret);
}
