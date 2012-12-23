#include <stdarg.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "../xmlapi/xmlapi.h"
#include "wftk_internals.h"
#include "../include/localdefs.h"
struct adaptor_info * DSREP_localxml_get_info(void);
struct adaptor_info * PDREP_localxml_get_info(void);
struct adaptor_info * USER_localxml_get_info(void);
struct adaptor_info * PERMS_localxml_get_info(void);
struct adaptor_info * TASKINDEX_stdout_get_info(void);
#ifdef WIN32
struct adaptor_info * TASKINDEX_odbc_get_info(void);
#endif
struct adaptor_info * ACTION_wftk_get_info(void);
struct adaptor_info * ACTION_system_get_info(void);
struct adaptor_info * NOTIFY_smtp_get_info(void);
struct adaptor_info * DATASTORE_role_get_info(void);
struct adaptor_info * DATATYPE_option_get_info(void);
WFTK_ADAPTOR * config_get_adaptor (void * session, int adaptor_class, char * adaptor_descriptor, int name_length)
{
   struct adaptor_info * ai = (struct adaptor_info *) 0;

   WFTK_ADAPTOR * ret;

   switch (adaptor_class) {
      case DSREP:
         if (!name_length || (name_length == 8 && !strncmp ("localxml", adaptor_descriptor, 8))) {
            ai = DSREP_localxml_get_info();
         }
         break;
      case DATASTORE:
         if (name_length == 4 && (!strncmp ("role", adaptor_descriptor, 4))) {
            ai = DATASTORE_role_get_info();
         }
         break;
      case DATATYPE:
         if (name_length == 6 && (!strncmp ("option", adaptor_descriptor, 6))) {
            ai = DATATYPE_option_get_info();
         }
         break;
      case PDREP:
         if (!name_length || (name_length == 8 && !strncmp ("localxml", adaptor_descriptor, 8))) {
            ai = PDREP_localxml_get_info();
         }
         break;
      case USER:
         if (!name_length || (name_length == 8 && !strncmp ("localxml", adaptor_descriptor, 8))) {
            ai = USER_localxml_get_info();
         }
         break;
      case PERMS:
         /* Perms are different -- the caller gets no choice.
            Otherwise I figure we're in trouble, security-wise.
            Probably are anyway. */
         ai = PERMS_localxml_get_info();
         break;
      case TASKINDEX:
#ifdef WIN32
         if (name_length == 6 && !strncmp ("stdout", adaptor_descriptor, 6)) {
            ai = TASKINDEX_stdout_get_info();
            break;
         }
         if (!name_length || (name_length == 4 && !strncmp ("odbc", adaptor_descriptor, 4))) {
            ai = TASKINDEX_odbc_get_info();
         }
#else
         if (!name_length || (name_length == 6 && !strncmp ("stdout", adaptor_descriptor, 6))) {
            ai = TASKINDEX_stdout_get_info();
            break;
         }
#endif
         break;
      case NOTIFY:
         if (!name_length || (name_length == 4 && !strncmp ("smtp", adaptor_descriptor, 4))) {
            ai = NOTIFY_smtp_get_info();
         }
         break;
      case ACTION:
         if (name_length == 6 && !strncmp ("system", adaptor_descriptor, 6)) {
            ai = ACTION_system_get_info();
            break;
         }
         if (!name_length || (name_length == 4 && !strncmp ("wftk", adaptor_descriptor, 4))) {
            ai = ACTION_wftk_get_info();
         }
         break;
      case DEBUG_MSG:
         break;
      default:
         return (WFTK_ADAPTOR *) 0;
   }

   if (!ai) return (WFTK_ADAPTOR *) 0; /* This signifies an unknown (or at least unimplemented) adaptor class. */

   ret = (WFTK_ADAPTOR *) malloc (sizeof (struct wftk_adaptor) + ai->nfuncs * sizeof (void *));
   if (!ret) return (WFTK_ADAPTOR *) 0;

   ret->num     = adaptor_class;
   ret->parms   = (void *) 0;     /* This will be filled in by the caller. */
   ret->nfuncs  = ai->nfuncs;
   ret->names   = ai->names;
   ret->vtab    = ai->vtab;
   ret->session = session;
   return (ret);
}
WFTK_ADAPTORLIST * config_get_adaptorlist (void * session, int adaptor_class)
{
   const char * spec = "";
   int adaptors = 1;
   int i;
   const char * mark;
   char * mark2;
   char adaptorbuffer[256]; /* TODO: another dang static buffer.  Fix it. */
   WFTK_ADAPTORLIST * list;

   switch (adaptor_class) {
      case TASKINDEX:
         spec = config_get_value (session, "taskindex.always");
         break;
      case NOTIFY:
         spec = config_get_value (session, "notify.always");
         break;
      default:
         return (WFTK_ADAPTORLIST *) 0;
   }

   if (!*spec) return (WFTK_ADAPTORLIST *) 0;

   /* First pass: count semicolons, so we know how large a list structure to allocate. */
   mark = spec;
   do {
      mark = strchr (mark, ';');
      if (!mark) break;
      adaptors++; mark++;
   } while (mark);

   list = (WFTK_ADAPTORLIST *) malloc (sizeof (WFTK_ADAPTOR_LIST) + adaptors * sizeof (WFTK_ADAPTOR *));
   list->count = adaptors;

   for (i=0, mark = spec; i < adaptors; i++) {
      strcpy (adaptorbuffer, mark);
      mark = strchr(mark, ';');
      if (mark) { mark++; while (*mark == ' ') mark++; }
      mark2 = strchr (adaptorbuffer, ';');
      if (mark2) *mark2 = '\0';

      list->ads[i] = wftk_get_adaptor (session, adaptor_class, adaptorbuffer);
   }

   return (list);
}
XML * config_find_option (XML * xml, const char * name) {
   int len;
   XML * x;
   char * mark = strchr (name, '.');

   if (mark) len = mark - name;
   else      len = strlen (name);

   x = xml_firstelem (xml);
   while (x) {
      if (!strncmp (xml_attrval (x, "name"), name, len) ||
          !strncmp (xml_attrval (x, "id"), name, len)   ||
          !strncmp ("?", name, len)) {
         if (mark) {
            return (config_find_option (x, mark + 1));
         }
         return (x);
      }
      x = xml_nextelem (x);
   }
   return NULL;
}
XML * config_get_option (void * session, const char * valuename) {
   WFTK_SESSION * sess = session;
   if (sess) {
      if (sess->config) {
         return (config_find_option (sess->config, valuename));
      }
   }
   return NULL;
}
const char * config_get_value (void * session, const char * valuename) {
   XML * mark = config_get_option (session, valuename);
   if (mark) return (xml_attrval (mark, "value"));

   if (!strcmp (valuename, "pdrep.localxml.directory")) return PROCDEF_DIRECTORY;
   if (!strcmp (valuename, "dsrep.localxml.directory")) return DATASHEET_DIRECTORY;
   if (!strcmp (valuename, "user.localxml.directory")) return USER_DIRECTORY;
   if (!strcmp (valuename, "group.localxml.directory")) return GROUP_DIRECTORY;
   if (!strcmp (valuename, "taskindex.odbc.?.conn")) return ODBC_CONNECTION;
   return "";
}
void config_debug_message (char type, const char * message, ...)
{
   va_list arglist;

   va_start (arglist, message);
   printf ("DEBUG %c:", type);
   vprintf (message, arglist);
   printf ("\n");
   va_end (arglist);
}
