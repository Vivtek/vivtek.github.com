#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "xmlapi.h"
#include "../wftk_internals.h"
static char *names[] = 
{
   "init",
   "free",
   "info",
   "do"
};

XML * ACTION_system_init (WFTK_ADAPTOR * ad, va_list args);
XML * ACTION_system_free (WFTK_ADAPTOR * ad, va_list args);
XML * ACTION_system_info (WFTK_ADAPTOR * ad, va_list args);
XML * ACTION_system_do (WFTK_ADAPTOR * ad, va_list args);

static WFTK_API_FUNC vtab[] = 
{
   ACTION_system_init,
   ACTION_system_free,
   ACTION_system_info,
   ACTION_system_do
};

static struct adaptor_info _ACTION_system_info =
{
   4,
   names,
   vtab
};
struct adaptor_info * ACTION_system_get_info ()
{
   return & _ACTION_system_info;
}
XML * ACTION_system_init (WFTK_ADAPTOR * ad, va_list args) {
   xml_set (ad->parms, "spec", "wftk");
   return (XML *) 0;
}
XML * ACTION_system_free (WFTK_ADAPTOR * ad, va_list args) { return (XML *) 0; }
XML * ACTION_system_info (WFTK_ADAPTOR * ad, va_list args) {
   XML * info;

   info = xml_create ("info");
   xml_set (info, "type", "action");
   xml_set (info, "name", "system");
   xml_set (info, "ver", "1.0.0");
   xml_set (info, "compiled", __TIME__ " " __DATE__);
   xml_set (info, "author", "Michael Roberts");
   xml_set (info, "contact", "wftk@vivtek.com");
   xml_set (info, "extra_functions", "0");

   return (info);
}
XML * ACTION_system_do  (WFTK_ADAPTOR * ad, va_list args)
{
   XML * action = (XML *) 0;
   const char * aname;
   char * content;

   if (args) action = va_arg (args, XML *);
   if (action) {
      aname = xml_attrval (action, "action");
      if (!strcmp (aname, "exec")) {
         /* TODO: Line by line, run the content of the action through the data interpreter
            and pass it over to the shell.  Primitive, yeah -- you can't break lines. */
         /* TODO: do a better parser for this, line breaks and stuff. */
         content = xml_stringcontent (action);
         system (content);
         free (content);
      }
   }
   return 0;
}
