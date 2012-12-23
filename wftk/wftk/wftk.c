#include <stdio.h>
#include <malloc.h>
#include <stdarg.h>
#include <string.h>
#include "xmlparse.h"
#include "localdefs.h"
void output(char type, const char *format, ...);
typedef struct _element XML;
typedef struct _attr ATTR;
typedef struct _list ELEMENTLIST;

struct _element {
   char *name;
   ATTR *attrs;
   XML *parent;
   ELEMENTLIST *children;
};

struct _attr {
   char *name;
   char *value;
   ATTR *next;
};

struct _list {
   XML *element;
   ELEMENTLIST *next;
   ELEMENTLIST *prev;
};
XML *xml_create(const char *name)
{
   XML *ret;

   ret = (XML *) malloc(sizeof(struct _element));
   ret->name = (char *) malloc(strlen(name) + 1);
   strcpy(ret->name, name);
   ret->attrs = NULL;
   ret->children = NULL;

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

void xml_writecontent(FILE * file, XML * xml);
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
      xml_set(element, *atts++, *atts++);
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
         output('E', "XML error: %s at line %d",
                XML_ErrorString(XML_GetErrorCode(parser)),
                XML_GetCurrentLineNumber(parser));
         xml_free(ret);
         return NULL;
      }
   } while (!done);
   XML_ParserFree(parser);

   return (ret);
}
typedef struct _command COMMAND;
struct _command {
   char *name;
   int argc;
   char *argv[5];
   COMMAND *next;
};
COMMAND *command_add(COMMAND * list, char *name, int argc, ...)
{
   COMMAND *cmd;
   int i;
   va_list argv;

   va_start(argv, argc);
   cmd = (COMMAND *) malloc(sizeof(struct _command));
   cmd->name = name;
   cmd->argc = argc;
   for (i = 0; i < argc; i++) {
      cmd->argv[i] = va_arg(argv, char *);
   }
   va_end(argv);
   cmd->next = NULL;

   if (!list)
      return (cmd);

   while (list->next != NULL)
      list = list->next;
   list->next = cmd;

   return cmd;
}

void command_freelist(COMMAND * list)
{
   COMMAND *cmd;

   while (list) {
      cmd = list;
      list = list->next;
      free((void *) cmd);
   }
}
void command_load(COMMAND * list, FILE * file)
{

}
void output(char type, const char *format, ...)
{
   va_list args;
   va_start(args, format);
   printf("%c ", type);
   vprintf(format, args);
   va_end(args);
   printf("\n");
}

XML *procdef;
XML *datasheet;
COMMAND *command_stack;
XML *state;
XML *queue;
int idcount;

char *process;

char sbuf[1024];
XML *queue_procdef(XML * action)
{
   XML *item;

   if (action == NULL)
      return;

   if (state == NULL) {
      state = xml_create("state");
      xml_append(datasheet, state);
   }
   if (queue == NULL) {
      queue = xml_create("queue");
      xml_append(state, queue);
   }
   item = xml_create("item");
   xml_setnum(item, "id", idcount++);
   xml_set(item, "type", action->name);
   xml_getloc(action, sbuf, 1023);
   xml_set(item, "loc", sbuf);
   xml_append(queue, item);
   return (item);
}

void process_procdef()
{
   XML *item;
   XML *def;
   XML *holder;
   XML *task;
   XML *data;
   XML *next;
   const char *type;
   int count;
   int keep;

   item = xml_first(queue);

   while (item != NULL) {
      if (!strcmp("yes", xml_attrval(item, "block"))) {
         item = xml_next(item);
         continue;
      }
      def = xml_loc(procdef, xml_attrval(item, "loc"));
      type = xml_attrval(item, "type");

      keep = 0;
      if (!strcmp(type, "workflow") || !strcmp(type, "sequence")) {
         if (!strcmp("", xml_attrval(item, "cur"))) {
            next = xml_firstelem(def);
         } else {
            next = xml_loc(procdef, xml_attrval(item, "cur"));
            next = xml_nextelem(next);
         }

         if (next) {
            xml_set(queue_procdef(next), "parent",
                    xml_attrval(item, "id"));
            xml_getloc(next, sbuf, sizeof(sbuf) - 1);
            xml_set(item, "cur", sbuf);
            keep = 1;
         } else if (!strcmp(type, "workflow")) {
            output('F', "Process %s complete.", process);
         }
      } else if (!strcmp(type, "parallel")) {
         if (!strcmp("", xml_attrval(item, "remaining"))) {
            count = 0;
            next = xml_firstelem(def);
            while (next != NULL) {
               count++;
               xml_set(queue_procdef(next), "parent",
                       xml_attrval(item, "id"));
               next = xml_nextelem(next);
            }
         } else {
            count = xml_attrvalnum(item, "remaining");
            count--;
         }
         xml_setnum(item, "remaining", count);
         if (count > 0)
            keep = 1;
      } else if (!strcmp(type, "task")) {
         if (strcmp(xml_attrval(item, "block"), "resume")) {
            sprintf(sbuf, "%s:%s", process, xml_attrval(item, "id"));
            task = xml_create("task");
            xml_set(task, "id", sbuf);
            xml_append(datasheet, task);
            output('A', "%s-%s-%s", sbuf, xml_attrval(def, "role"),
                   xml_attrval(def, "label"));

            holder = xml_firstelem(def);
            while (holder != NULL) {
               if (!strcmp(holder->name, "data")) {
                  data = xml_create("data");
                  xml_set(data, "id", xml_attrval(holder, "name"));
                  xml_set(data, "type", xml_attrval(holder, "type"));
                  xml_append(task, data);
               }
               holder = xml_nextelem(holder);
            }
            keep = 1;
         }
      } else if (!strcmp(type, "data")) {
      } else if (!strcmp(type, "situation")) {
      } else if (!strcmp(type, "if") || !strcmp(type, "elseif")) {
      } else if (!strcmp(type, "alert")) {
         output('L', "%s:%s", xml_attrval(def, "type"),
                xml_attrval(def, "to"));
         xml_writecontent(stdout, def);
         printf("\nEOF\n");
      } else if (!strcmp(type, "start")) {
      }

      if (keep) {
         xml_set(item, "block", "yes");
         item = xml_next(item);
      } else {
         if (strcmp("workflow", type)) {
            sprintf(sbuf, "queue.item[%d]",
                    xml_attrvalnum(item, "parent"));
            next = xml_loc(queue, sbuf);
            xml_delete(item);
            item = next;
            xml_set(item, "block", "no");
         } else {
            xml_delete(item);
            item = NULL;
         }
      }
   }
   sprintf(sbuf, "%d", idcount);
   xml_set(state, "idcount", sbuf);
}

void load_datasheet()
{
   FILE *temp;
   if (datasheet)
      return;

   sprintf(sbuf, "%s%s", DATASHEET_REPOSITORY, process);
   temp = fopen(sbuf, "r");
   if (!temp) {
      output('E', "Can't open datasheet for process %s.", process);
      return;
   }
   datasheet = xml_read(temp);
   fclose(temp);
   state = xml_loc(datasheet, "datasheet.state");
   idcount = xml_attrvalnum(state, "idcount");
   queue = xml_loc(datasheet, "datasheet.state.queue");
   sprintf(sbuf, "%s%s", PROCESS_DEFINITION_REPOSITORY,
           xml_attrval(datasheet, "procdef"));
   temp = fopen(sbuf, "r");
   if (!temp) {
      output('E', "Can't open process definition for process %s.",
             process);
      xml_delete(datasheet);
      datasheet = NULL;
      return;
   }
   procdef = xml_read(temp);
   fclose(temp);
}

void interpret(COMMAND * list)
{
   FILE *temp;
   int datasheet_dirty;
   char line[1024];
   char *mark;
   XML *holder;

   datasheet_dirty = 0;

   while (list) {
      if (!strcmp(list->name, "start")) {
         if (procdef != NULL) {
            output('E', "Start command must be first.  Skipping command.");
            goto skip_command;
         }

         if (list->argc < 1) {
            sprintf(sbuf, "%s%s", DATASHEET_REPOSITORY, process);
            temp = fopen(sbuf, "r");
            if (!temp) {
               output('E', "No process '%s' exists.", process);
               return;
            }

            datasheet = xml_read(temp);
            fclose(temp);

            sprintf(sbuf, "%s%s", PROCESS_DEFINITION_REPOSITORY,
                    xml_attrval(datasheet, "procdef"));
         } else {
            sprintf(sbuf, "%s%s_versions.txt",
                    PROCESS_DEFINITION_REPOSITORY, list->argv[0]);
            temp = fopen(sbuf, "r");
            if (!temp) {
               output('E', "No process '%s' defined.", list->argv[0]);
               return;
            }

            *line = '\0';
            while (!feof(temp)) {
               fgets(line, 1023, temp);
            }
            fclose(temp);
            mark = strchr(line, '\t');
            if (mark)
               *mark = '\0';

            sprintf(sbuf, "%s%s_%s.xml", PROCESS_DEFINITION_REPOSITORY,
                    list->argv[0], line);
         }

         temp = fopen(sbuf, "r");
         if (!temp) {
            output('E', "Process definition version file '%s' is missing.",
                   sbuf);
            return;
         }

         procdef = xml_read(temp);
         fclose(temp);

         if (datasheet == NULL) {
            datasheet = xml_create("datasheet");
            sprintf(sbuf, "%s_%s.xml", list->argv[0], line);
            xml_set(datasheet, "procdef", sbuf);
         }

         datasheet_dirty = 1;

         output('N', xml_attrval(procdef, "name"));
         output('O', xml_attrval(procdef, "author"));
         holder = xml_loc(procdef, "workflow.note[description]");
         if (holder != NULL) {
            xml_writecontent(stdout, holder);
         }
         printf("\nEOF\n");

         queue_procdef(procdef);
         process_procdef();
      } else if (!strcmp(list->name, "complete")) {
         load_datasheet();
         mark = strrchr(list->argv[0], ':');
         if (mark)
            mark++;
         else
            (mark = list->argv[0]);

         sprintf(sbuf, "queue.item[%s]", mark);
         holder = xml_loc(queue, sbuf);
         if (holder)
            xml_set(holder, "block", "resume");
         datasheet_dirty = 1;
         process_procdef();
      } else if (!strcmp(list->name, "reject")) {
      } else if (!strcmp(list->name, "setvalue")) {
      } else if (!strcmp(list->name, "script")) {
         temp = NULL;
         if (list->argc) {
            output('D', "Script %s", list->argv[0]);
            temp = fopen(list->argv[0], "r");
            if (!temp) {
               output('E', "Unable to open script file '%s'",
                      list->argv[0]);}
         } else {
            output('D', "Script on stdin");
            temp = stdin;
         }

         if (temp) {
            output('D', "This is where the script would be loaded.",
                   list->argv[0]);
            if (temp != stdin)
               fclose(temp);
         }
      } else {
         output('E', "Unknown command %s encountered", list->name);
      }
    skip_command:
      list = list->next;
   }

   if (datasheet != NULL && datasheet_dirty) {
      sprintf(sbuf, "%s%s", DATASHEET_REPOSITORY, process);
      temp = fopen(sbuf, "w");
      if (!temp) {
         output('E', "Can't write to datasheet for process %s.", process);
      } else {
         xml_write(temp, datasheet);
         fclose(temp);
      }
   }
}
int main(int argc, char *argv[])
{
   if (argc < 3) {
      printf("usage: wftk <command> <arguments>\n");
      printf("  Commands:\n");
      printf("  start <process id> <procdef>\n");
      printf("  activate <process id> <locator> <task id>\n");
      printf("  complete <process id> <task id>\n");
      printf("  reject <process id> <task id>\n");
      printf
          ("  setvalue <process id> <name> <type> <file> or value on stdin\n");
      printf
          ("  settaskvalue <process id> <task id> <name> <type> <file> or value on stdin\n");
      printf("  script <process id> <file> or command list on stdin\n");
   }
   process = argv[2];
   switch (argc) {
   case 3:
      command_stack = command_add(NULL, argv[1], 0);
      break;
   case 4:
      command_stack = command_add(NULL, argv[1], 1, argv[3]);
      break;
   case 5:
      command_stack = command_add(NULL, argv[1], 2, argv[3], argv[4]);
      break;
   case 6:
      command_stack =
          command_add(NULL, argv[1], 3, argv[3], argv[4], argv[5]);
      break;
   case 7:
      command_stack =
          command_add(NULL, argv[1], 4, argv[3], argv[4], argv[5],
                      argv[6]);
      break;
   }

   interpret(command_stack);

   xml_free(datasheet);
   xml_free(procdef);
   return 0;
}
