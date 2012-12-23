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
   "get",
   "set",
   "isnull",
   "makenull"
};

XML * DATASTORE_role_init     (WFTK_ADAPTOR * ad, va_list args);
XML * DATASTORE_role_free     (WFTK_ADAPTOR * ad, va_list args);
XML * DATASTORE_role_info     (WFTK_ADAPTOR * ad, va_list args);
XML * DATASTORE_role_get      (WFTK_ADAPTOR * ad, va_list args);
XML * DATASTORE_role_set      (WFTK_ADAPTOR * ad, va_list args);
XML * DATASTORE_role_isnull   (WFTK_ADAPTOR * ad, va_list args);
XML * DATASTORE_role_makenull (WFTK_ADAPTOR * ad, va_list args);

static WFTK_API_FUNC vtab[] = 
{
   DATASTORE_role_init,
   DATASTORE_role_free,
   DATASTORE_role_info,
   DATASTORE_role_get,
   DATASTORE_role_set,
   DATASTORE_role_isnull,
   DATASTORE_role_makenull
};

static struct adaptor_info _DATASTORE_role_info =
{
   7,
   names,
   vtab
};
struct adaptor_info * DATASTORE_role_get_info ()
{
   return & _DATASTORE_role_info;
}
XML * DATASTORE_role_init (WFTK_ADAPTOR * ad, va_list args) { return (XML *) 0; }
XML * DATASTORE_role_free (WFTK_ADAPTOR * ad, va_list args) { return (XML *) 0; }
XML * DATASTORE_role_info (WFTK_ADAPTOR * ad, va_list args) {
   XML * info;

   info = xml_create ("info");
   xml_set (info, "type", "datastore");
   xml_set (info, "name", "role");
   xml_set (info, "ver", "1.0.0");
   xml_set (info, "compiled", __TIME__ " " __DATE__);
   xml_set (info, "author", "Michael Roberts");
   xml_set (info, "contact", "wftk@vivtek.com");
   xml_set (info, "extra_functions", "0");

   return (info);
}
XML * DATASTORE_role_get  (WFTK_ADAPTOR * ad, va_list args)
{
   XML * datasheet = (XML *) 0;
   char * name;
   char * colon;

   if (args) datasheet = va_arg (args, XML *);
   if (!datasheet) {
      xml_set (ad->parms, "error", "No datasheet supplied.");
      return (XML *) 0;
   }
   name = va_arg (args, char *);
   if (!name) {
      xml_set (ad->parms, "error", "No role named.");
      return (XML *) 0;
   }

   colon = strchr (name, ':');
   return (wftk_session_stashvalue (ad->session, wftk_role_user (ad->session, datasheet, colon ? colon + 1 : name)));
}
XML * DATASTORE_role_set  (WFTK_ADAPTOR * ad, va_list args)
{
   XML * datasheet = (XML *) 0;
   char * name;
   char * value;
   char * colon;

   if (args) datasheet = va_arg (args, XML *);
   if (!datasheet) {
      xml_set (ad->parms, "error", "No datasheet supplied.");
      return (XML *) 0;
   }
   name = va_arg (args, char *);
   if (!name) {
      xml_set (ad->parms, "error", "No role named.");
      return (XML *) 0;
   }
   value = va_arg (args, char *);

   colon = strchr (name, ':');
   wftk_role_assign (ad->session, datasheet, colon ? colon + 1 : name, value);
   return (XML *) 0;
}
XML * DATASTORE_role_isnull   (WFTK_ADAPTOR * ad, va_list args) { return (XML *) 0; }
XML * DATASTORE_role_makenull (WFTK_ADAPTOR * ad, va_list args) { return (XML *) 0; }
