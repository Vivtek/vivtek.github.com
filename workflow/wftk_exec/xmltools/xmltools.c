#include <stdio.h>
#include <malloc.h>
#include "xmlparse.h"

typedef struct _frame FRAME;
typedef struct _tag TAG;

struct _frame {
   const char *name;
   int level;
   int offset_in_parent;
   int empty;
   FRAME *back;
   FRAME *next;
   TAG *subitems;

   const char *locator;
};

struct _tag {
   const char *name;
   int count;
   TAG *next;
};
/* The parsing context: */
FRAME stack = {
   "<bottom>",
   0,
   0,
   0,
   NULL,
   NULL,
   NULL,
   NULL
};

/* Settings */
int emit_content = 1;
int emit_matching_tag = 1;
int emit_tags = 1;
int emit_comments = 0;
int emit_pis = 0;
int emit_locator = 0;

char *infile = NULL;
char *insertwhere = NULL;
char *setattr = NULL;
char *setvalue = NULL;

FRAME *top = &stack;

int finished;                   /* We can set this if we know we're done with the snip. */
int emit_level = -1;            /* Set this to the level when our location first matches the locator. */

int done;

void free_frame(FRAME * frame)
{
   TAG *cur;
   TAG *next;
   cur = frame->subitems;
   while (cur) {
      next = cur->next;
      free((void *) cur);
      cur = next;
   }
   if (frame->level > 0)
      free((void *) frame);
}

void free_stack()
{
   FRAME *cur;

   while (top->level > 0) {
      cur = top;
      top = top->back;
      free_frame(cur);
   }
   free_frame(top);
}

void print_stack()
{
   FRAME *cur;
   for (cur = stack.next; cur != NULL; cur = cur->next) {
      if (cur != stack.next)
         printf(".");
      printf("%s", cur->name);
      if (cur->offset_in_parent > 0) {
         printf("(%d)", cur->offset_in_parent);
      }
   }
   printf("\n");
}
void startElement(void *userData, const char *name, const char **atts)
{
   FRAME *newframe;
   TAG *scan;
   const char *mark;
   int matchoffset;
   int att;
   const char **attptr;
   int on;
   int c;
   int didit;

   newframe = (FRAME *) malloc(sizeof(FRAME));
   newframe->next = NULL;
   newframe->back = top;
   newframe->level = newframe->back->level + 1;
   newframe->name = name;
   newframe->subitems = NULL;
   newframe->offset_in_parent = 0;
   newframe->locator = NULL;
   newframe->empty = 1;

   if (stack.next == NULL) {
      stack.next = newframe;
      newframe->back = &stack;
   } else {
      top->next = newframe;
   }
   top = newframe;

   /* If parent is still empty, close its tag. */
#ifdef XMLSNIP
   if (!finished && emit_level > -1
       && top->back->level - 1 + emit_matching_tag >= emit_level
       && top->back->empty == 1) {
#else
#ifdef XMLREPLACE
   if (
       (emit_level < 0
        || top->back->level + emit_matching_tag <= emit_level)
       && top->back->empty == 1) {
#else
   if (top->back->empty == 1) {
#endif
#endif
      if (emit_tags)
         top->back->empty = 0;
      if (emit_tags)
         printf(">");
   }

   /* Scan parent's subitems to ascertain our own offset in that list. */
   if (top->back) {
      scan = top->back->subitems;
      while (scan) {
         if (!strcmp(scan->name, name))
            break;
         scan = scan->next;
      }
      if (scan) {
         scan->count++;
      } else {
         scan = (TAG *) malloc(sizeof(TAG));
         scan->name = name;
         scan->next = top->back->subitems;
         scan->count = 0;
         top->back->subitems = scan;
      }
      top->offset_in_parent = scan->count;
   }

   /* Now check parent's locator to see whether we match the next bit.
      (A null locator means the parent isn't in the match path.) */
   if (top->back->locator != NULL) {
      if (!strncmp(name, top->back->locator, strlen(name))) {
         mark = (char *) top->back->locator + strlen(name);
         matchoffset = 0;
         if (*mark == '(') {
            mark++;
            matchoffset = atoi(mark);
            while (*mark && *mark != ')')
               mark++;
            if (*mark)
               mark++;
         } else if (*mark == '[') {
            mark++;
            for (attptr = atts; *attptr && strcmp(*attptr, "id");
                 attptr += 2);
            if (!*attptr)
               for (attptr = atts; *attptr && strcmp(*attptr, "name");
                    attptr += 2);
            *attptr++;
            if (!strncmp(*attptr, mark, strlen(*attptr))) {
               mark += strlen(*attptr);
               if (*mark == ']') {
                  matchoffset = top->offset_in_parent;
                  mark++;
               }
            }

         }
         if ((*mark == '\0' || *mark == ' ')
             && top->offset_in_parent == matchoffset) {
            /* Ding, ding! */
            emit_level = top->level;
            if (emit_locator)
               print_stack();
            emit_level = top->level;
         } else if (*mark == '.' && top->offset_in_parent == matchoffset) {
            top->locator = mark + 1;
         }
      }
   }
#ifdef XMLINSERT
   if (top->level == emit_level && !strcmp(insertwhere, "before")) {
      while (!feof(stdin)) {
         c = getchar();
         if (!feof(stdin))
            putchar(c);
      }
      emit_level = -1;
   }
#endif

   /* Emit tag. */
#ifdef XMLSNIP
   if (!finished && emit_level > -1
       && top->level - 1 + emit_matching_tag >= emit_level) {
#else
#ifdef XMLREPLACE
   if (emit_level < 0 || top->level + emit_matching_tag <= emit_level) {
#else
   if (1) {
#endif
#endif
      if (emit_tags) {
         printf("<%s", name);
#ifdef XMLSET
         mark = NULL;
         finished = 0;
#endif
         for (att = 0, on = 1; atts[att]; att++) {
            if (on)
               printf(" %s", atts[att]);
#ifdef XMLSET
            if (on && !strcmp(atts[att], setattr)
                && top->level == emit_level) {
               mark = atts[att];
            }
            if (!on && mark != NULL && top->level == emit_level) {
               printf("=\"%s\"", setvalue);
               mark = NULL;
               finished = 1;
            } else
#endif
            if (!on)
               printf("=\"%s\"", atts[att]);
            on = !on;
         }
#ifdef XMLSET
         if (!finished && top->level == emit_level)
            printf(" %s=\"%s\"", setattr, setvalue);
#endif
      }
   }
#ifdef XMLSET
   emit_level = -1;
   finished = 0;
#endif

#ifdef XMLREPLACE
   if (emit_level == top->level) {
      top->empty = 0;
      if (emit_tags && !emit_matching_tag)
         printf(">");
      while (!feof(stdin)) {
         c = getchar();
         if (!feof(stdin))
            putchar(c);
      }
   }
#endif

#ifdef XMLINSERT
   if (top->level == emit_level && !strcmp(insertwhere, "beforecontent")) {
      top->empty = 0;
      if (emit_tags && emit_matching_tag)
         printf(">");
      while (!feof(stdin)) {
         c = getchar();
         if (!feof(stdin))
            putchar(c);
      }
      emit_level = -1;
   }
#endif
}
void endElement(void *userData, const char *name)
{
   FRAME *old;
   int c;

#ifdef XMLINSERT
   if (top->level == emit_level && !strcmp(insertwhere, "aftercontent")) {
      if (top->empty) {
         printf(">");
         top->empty = 0;
      }
      while (!feof(stdin)) {
         c = getchar();
         if (!feof(stdin))
            putchar(c);
      }
      emit_level = -1;
   }
#endif

#ifdef XMLSNIP
   if (!finished && emit_level > -1
       && top->level - 1 + emit_matching_tag >= emit_level) {
#else
#ifdef XMLREPLACE
   if (emit_level < 0 || top->level + emit_matching_tag <= emit_level) {
#else
   if (1) {
#endif
#endif
      if (!top->empty) {
         if (emit_tags)
            printf("</%s", name);
      } else {
         if (emit_tags)
            printf("/");
      }
      if (emit_tags)
         printf(">");
   }
#ifdef XMLINSERT
   if (top->level == emit_level && !strcmp(insertwhere, "after")) {
      while (!feof(stdin)) {
         c = getchar();
         if (!feof(stdin))
            putchar(c);
      }
      emit_level = -1;
   }
#endif

#ifdef XMLSNIP
   if (top->level == emit_level)
      finished = 1;
#endif
   if (top->level == emit_level)
      emit_level = -1;

   old = top;
   if (top->back->level == 0) {
      top->back->next = NULL;
   }
   top = top->back;
   free_frame(old);
}
void charData(void *userData, const XML_Char * s, int len)
{
   int i;
   /* If parent is still empty, close its tag. */
#ifdef XMLSNIP
   if (!finished && emit_level > -1
       && top->level - 1 + emit_matching_tag >= emit_level
       && top->empty == 1) {
#else
#ifdef XMLREPLACE
   if ((emit_level < 0 || top->level < emit_level) && top->empty == 1) {
#else
   if (top->empty == 1) {
#endif
#endif
      printf(">");
      top->empty = 0;
   }
#ifdef XMLSNIP
   if (!finished && emit_level > -1 && top->level >= emit_level) {
#else
#ifdef XMLREPLACE
   if (emit_level < 0 || top->level < emit_level) {
#else
   if (1) {
#endif
#endif
      for (i = 0; i < len; i++) {
         putchar(s[i]);
      }
   }
}
int init(int argc, char *argv[])
{
   int i;
   char *mark;

   for (i = 1; i < argc; i++) {
      if (*argv[i] == '-') {
         mark = argv[i];
         while (*mark++) {
            switch (*mark) {
            case 'c':
               emit_comments = 1;
               break;
            case 'o':
               emit_tags = 0;
               break;
            case 'e':
               emit_content = 0;
               break;
#ifndef XMLSET
            case 'm':
               emit_matching_tag = 0;
               break;
#endif
            case 'p':
               emit_pis = 1;
               break;
            case 'l':
               emit_locator = 1;
               break;
            }
         }
      }
#ifdef XMLINSERT
      else if (!insertwhere) {
         insertwhere = argv[i];
      }
#endif
      else if (!stack.locator) {
         stack.locator = argv[i];
      }
#ifdef XMLSET
      else if (!setattr) {
         setattr = argv[i];
      } else if (!setvalue) {
         setvalue = argv[i];
      }
#endif
      else if (!infile) {
         infile = argv[i];
      }
   }
}

int main(int argc, char *argv[])
{
   char buf[BUFSIZ];
   FILE *in;
   XML_Parser parser;

   init(argc, argv);

   if (!stack.locator) {
#ifdef XMLSNIP
      printf("usage: xmlsnip <flags> <location> [<file>]\n");
#endif
#ifdef XMLREPLACE
      printf
          ("usage: xmlreplace <flags> <location> <file>\n  - Replacement on stdin\n");
#endif
#ifdef XMLSET
      printf
          ("usage: xmlset <flags> <location> <attr> <new value> [<file>]\n");
#endif
#ifdef XMLINSERT
      printf
          ("usage: xmlsnip <flags> <insertwhere> <location> <file>\n  - Insert on stdin\n");
#endif
      printf
          ("  Flags: (not all may make sense for this program; xmltools share the flags.\n");
      printf("  -c : include comments\n");
      printf("  -o : exclude elements\n");
      printf("  -t : exclude nonelement content\n");
      printf("  -m : exclude the matching tag\n");
      printf("  -p : include processing instructions (PIs)\n");
      printf("  -l : include locator when match occurs\n");
   }

   if (!infile) {
      in = stdin;
   } else if (!(in = fopen(infile, "r"))) {
      printf("Unable to open input file %s\n", infile);
      exit(2);
   }

   parser = XML_ParserCreate(NULL);

   XML_SetElementHandler(parser, startElement, endElement);
   if (emit_content)
      XML_SetCharacterDataHandler(parser, charData);

   done = 0;
   finished = 0;

   do {
      size_t len = fread(buf, 1, sizeof(buf), in);
      done = len < sizeof(buf);
      if (!XML_Parse(parser, buf, len, done)) {
         fprintf(stderr,
                 "%s at line %d\n",
                 XML_ErrorString(XML_GetErrorCode(parser)),
                 XML_GetCurrentLineNumber(parser));
         return 1;
      }
   } while (!done && !finished);
   XML_ParserFree(parser);

   free_stack();

   if (emit_tags)
      printf("\n");

   if (in != stdin)
      fclose(in);
   return 0;
}
