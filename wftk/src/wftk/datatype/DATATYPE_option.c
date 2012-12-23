#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "../wftk.h"
#include "../wftk_internals.h"
static char *names[] = 
{
   "init",
   "free",
   "info",
   "html",
   "htmlblank"
};

XML * DATATYPE_option_init     (WFTK_ADAPTOR * ad, va_list args);
XML * DATATYPE_option_free     (WFTK_ADAPTOR * ad, va_list args);
XML * DATATYPE_option_info     (WFTK_ADAPTOR * ad, va_list args);
XML * DATATYPE_option_html     (WFTK_ADAPTOR * ad, va_list args);
XML * DATATYPE_option_htmlblank(WFTK_ADAPTOR * ad, va_list args);

static WFTK_API_FUNC vtab[] = 
{
   DATATYPE_option_init,
   DATATYPE_option_free,
   DATATYPE_option_info,
   DATATYPE_option_html,
   DATATYPE_option_htmlblank
};

static struct adaptor_info _DATATYPE_option_info =
{
   5,
   names,
   vtab
};
struct adaptor_info * DATATYPE_option_get_info ()
{
   return & _DATATYPE_option_info;
}
XML * DATATYPE_option_init (WFTK_ADAPTOR * ad, va_list args) { return (XML *) 0; }
XML * DATATYPE_option_free (WFTK_ADAPTOR * ad, va_list args) { return (XML *) 0; }
XML * DATATYPE_option_info (WFTK_ADAPTOR * ad, va_list args) {
   XML * info;

   info = xml_create ("info");
   xml_set (info, "type", "datatype");
   xml_set (info, "name", "option");
   xml_set (info, "ver", "1.0.0");
   xml_set (info, "compiled", __TIME__ " " __DATE__);
   xml_set (info, "author", "Michael Roberts");
   xml_set (info, "contact", "wftk@vivtek.com");
   xml_set (info, "extra_functions", "0");

   return (info);
}
XML * DATATYPE_option_html (WFTK_ADAPTOR * ad, va_list args)
{
   XML * datasheet = (XML *) 0;
   XML * data = (XML *) 0;
   XML * field;
   XML * option;
   XML * mark;

   if (args) datasheet = va_arg (args, XML *);
   if (!datasheet) {
      xml_set (ad->parms, "error", "No datasheet supplied.");
      return (XML *) 0;
   }
   data = va_arg (args, XML *);
   if (!data) {
      xml_set (ad->parms, "error", "No value supplied.");
      return (XML *) 0;
   }

   field = xml_create ("select");
   xml_set (field, "name", xml_attrval (data, "id"));
   if (*xml_attrval (data, "size")) {
      xml_set (field, "size", xml_attrval (data, "size"));
   }
   mark = xml_firstelem (data);
   while (mark) {
      option = xml_copy (mark);
      xml_append (field, option);
      if (!strcmp (xml_attrval (option, "value"), xml_attrval (data, "value"))) {
         xml_set (option, "selected", "yes");
      }
      mark = xml_nextelem (mark);
   }

   return (field);
}
XML * DATATYPE_option_htmlblank  (WFTK_ADAPTOR * ad, va_list args)
{
   XML * datasheet = (XML *) 0;
   XML * data = (XML *) 0;
   XML * field;
   XML * option;
   XML * mark;

   if (args) datasheet = va_arg (args, XML *);
   if (!datasheet) {
      xml_set (ad->parms, "error", "No datasheet supplied.");
      return (XML *) 0;
   }
   data = va_arg (args, XML *);
   if (!data) {
      xml_set (ad->parms, "error", "No value supplied.");
      return (XML *) 0;
   }

   field = xml_create ("select");
   xml_set (field, "name", xml_attrval (data, "id"));
   if (*xml_attrval (data, "size")) {
      xml_set (field, "size", xml_attrval (data, "size"));
   }
   mark = xml_firstelem (data);
   while (mark) {
      option = xml_copy (mark);
      xml_append (field, option);
      mark = xml_nextelem (mark);
   }

   return (field);
}
