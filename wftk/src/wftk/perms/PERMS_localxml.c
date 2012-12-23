#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef WIN32
#include <io.h>
#endif
#include <errno.h>
#include "xmlapi.h"
#include "../wftk_internals.h"
static char *names[] = 
{
   "init",
   "free",
   "info",
   "perm"
};

XML * PERMS_localxml_init (WFTK_ADAPTOR * ad, va_list args);
XML * PERMS_localxml_free (WFTK_ADAPTOR * ad, va_list args);
XML * PERMS_localxml_info (WFTK_ADAPTOR * ad, va_list args);
XML * PERMS_localxml_perm (WFTK_ADAPTOR * ad, va_list args);

static WFTK_API_FUNC vtab[] = 
{
   PERMS_localxml_init,
   PERMS_localxml_free,
   PERMS_localxml_info,
   PERMS_localxml_perm
};

static struct adaptor_info _PERMS_localxml_info =
{
   4,
   names,
   vtab
};
struct adaptor_info * PERMS_localxml_get_info ()
{
   return & _PERMS_localxml_info;
}
XML * PERMS_localxml_init (WFTK_ADAPTOR * ad, va_list args) {
   struct stat statbuf;
   char directory[1024]; /* TODO: the usual. */
   char * end;
   const char * parms = xml_attrval (ad->parms, "parms");
   if (!*parms) parms = config_get_value (ad->session, "perms.localxml.directory");

   /* Check for existence, return error if the directory doesn't exist or if it isn't a directory. */
   strcpy (directory, parms);
   end = directory + strlen (directory) - 1;
   if (*end == '\\') *end = '\0';

   if (stat (directory, &statbuf) == -1) {
      xml_set (ad->parms, "error", "Directory not found.");
      return (XML *) 0;
   }
   if (!(statbuf.st_mode & S_IFDIR)) {
      xml_set (ad->parms, "error", "Directory not directory.");
      return (XML *) 0;
   }

   strcat (directory, "\\");
   xml_set (ad->parms, "dir", directory);
   strcpy (directory, "localxml:");
   strcat (directory, xml_attrval (ad->parms, "dir"));
   xml_set (ad->parms, "spec", directory);
   return (XML *) 0;
}
XML * PERMS_localxml_free (WFTK_ADAPTOR * ad, va_list args) { return (XML *) 0; }
XML * PERMS_localxml_info (WFTK_ADAPTOR * ad, va_list args) {
   XML * info;

   info = xml_create ("info");
   xml_set (info, "type", "perms");
   xml_set (info, "name", "localxml");
   xml_set (info, "ver", "1.0.0");
   xml_set (info, "compiled", __TIME__ " " __DATE__);
   xml_set (info, "author", "Michael Roberts");
   xml_set (info, "contact", "wftk@vivtek.com");
   xml_set (info, "extra_functions", "0");

   return info;
}
void _PERMS_localxml_decide (XML * ret, XML * rules, XML * action, XML * user);
XML * PERMS_localxml_perm (WFTK_ADAPTOR * ad, va_list args) {
   char path[1024]; /* TODO: fix constant. */
   XML *action = (XML *) 0;
   XML *user = (XML *) 0;
   FILE *file;
   XML * rules;
   XML * ret;

   if (args) action = va_arg (args, XML *);
   if (!action) {
      xml_set (ad->parms, "error", "No action given.");
      return (XML *) 0;
   }
   user = va_arg (args, XML *);

   ret = xml_create ("value");
   strcpy (path, xml_attrval (ad->parms, "dir"));
   strcat (path, xml_attrval (action, "action"));
   strcat (path, ".xml");
   file = fopen (path, "r");
   if (!file) {
      xml_set (ret, "value", "ok");
      return ret;
   }

   rules = xml_read (file);
   fclose (file);

   /* OK, we've got a perms rulebase for the action in question, and we now have to decide whether we
      can authorize the action or not. */
   _PERMS_localxml_decide (ret, rules, action, user);
   xml_free (rules);

   /* In the absence of any other rules, we permit action to be taken. */
   if (!*xml_attrval (ret, "value")) {
      xml_set (ret, "value", "yes");
   }

   return (ret);
}
int _PERMS_localxml_test (XML * rule, XML * action, XML * user)
{
   int result = 0;
   const char * value;
   char * which;

   if (*xml_attrval (rule, "equal")) {  /* Cutting corners.  TODO: maybe some more choices? */
      which = "equal";
   } else return result;

   if (!strcmp (xml_attrval (rule, "value"), "user")) {
      if (user) {
         value = xml_attrval (user, "id");
      } else {
         value = "anonymous";
      }
   } else {
      value = xml_attrval (action, "value");
   }

   if (!strcmp (which, "equal")) {
      result = !strcmp (value, xml_attrval (rule, "equal"));
   }

   if (!strcmp (rule->name, "unless")) return !result;
   return result;
}
void _PERMS_localxml_decide (XML * ret, XML * rules, XML * action, XML * user)
{
   XML * elem;
   XML * elem2;
   int fire;

   elem = xml_firstelem (rules);
   while (elem) {
      if (!strcmp (elem->name, "if") || !strcmp (elem->name, "unless")) {
         if (_PERMS_localxml_test (elem, action, user)) {
            xml_set (ret, "value", xml_attrval (elem, "result"));
            xml_set (ret, "reason", xml_attrval (elem, "reason"));
            return;
         }
      } else if (!strcmp (elem->name, "else")) {
         xml_set (ret, "value", xml_attrval (elem, "result"));
         xml_set (ret, "reason", xml_attrval (elem, "reason"));
         return;
      } else if (!strcmp (elem->name, "any")) {
         elem2 = xml_firstelem (elem);
         while (elem2) {
            fire = 0;
            if (!strcmp (elem2->name, "if") || !strcmp (elem2->name, "unless")) {
               if (_PERMS_localxml_test (elem2, action, user)) {
                  if (*xml_attrval (elem2, "result")) {
                     xml_set (ret, "value", xml_attrval (elem2, "result"));
                     xml_set (ret, "reason", xml_attrval (elem, "reason"));
                     return;
                  }
                  fire = 1;
               }
            } else if (!strcmp (elem2->name, "then") && fire) {
               xml_set (ret, "value", xml_attrval (elem2, "result"));
               xml_set (ret, "reason", xml_attrval (elem, "reason"));
               return;
            }
            elem2 = xml_nextelem (elem2);
         }
         if (fire) {
            xml_set (ret, "value", xml_attrval (elem, "result"));
            xml_set (ret, "reason", xml_attrval (elem, "reason"));
            return;
         }
      } else if (!strcmp (elem->name, "all")) {
         elem2 = xml_firstelem (elem);
         while (elem2) {
            if (!strcmp (elem2->name, "if") || !strcmp (elem2->name, "unless")) {
               if (!_PERMS_localxml_test (elem2, action, user)) {
                  break;
               }
            } else if (!strcmp (elem->name, "then")) {
               xml_set (ret, "value", xml_attrval (elem2, "result"));
               xml_set (ret, "reason", xml_attrval (elem, "reason"));
               return;
            }
            elem2 = xml_nextelem (elem2);
         }
         if (!elem2) {
            xml_set (ret, "value", xml_attrval (elem, "result"));
            xml_set (ret, "reason", xml_attrval (elem, "reason"));
            return;
         }
      } else if (!strcmp (elem->name, "decide")) {
         _PERMS_localxml_decide (ret, elem, action, user);
         return;
      }
      elem = xml_nextelem (elem);
   }
}
