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
   "get",
   "auth"
};

XML * USER_localxml_init (WFTK_ADAPTOR * ad, va_list args);
XML * USER_localxml_free (WFTK_ADAPTOR * ad, va_list args);
XML * USER_localxml_info (WFTK_ADAPTOR * ad, va_list args);
XML * USER_localxml_get  (WFTK_ADAPTOR * ad, va_list args);
XML * USER_localxml_auth (WFTK_ADAPTOR * ad, va_list args);

static WFTK_API_FUNC vtab[] = 
{
   USER_localxml_init,
   USER_localxml_free,
   USER_localxml_info,
   USER_localxml_get,
   USER_localxml_auth
};

static struct adaptor_info _USER_localxml_info =
{
   5,
   names,
   vtab
};
struct adaptor_info * USER_localxml_get_info ()
{
   return & _USER_localxml_info;
}
XML * USER_localxml_init (WFTK_ADAPTOR * ad, va_list args) {
   struct stat statbuf;
   const char * parms;
   char directory[256];
   char * end;

   parms = xml_attrval (ad->parms, "parm");
   if (!*parms) parms = config_get_value (ad->session, "user.localxml.directory");

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
XML * USER_localxml_free (WFTK_ADAPTOR * ad, va_list args) { return (XML *) 0; }
XML * USER_localxml_info (WFTK_ADAPTOR * ad, va_list args) {
   XML * info;

   info = xml_create ("info");
   xml_set (info, "type", "user");
   xml_set (info, "name", "localxml");
   xml_set (info, "ver", "1.0.0");
   xml_set (info, "compiled", __TIME__ " " __DATE__);
   xml_set (info, "author", "Michael Roberts");
   xml_set (info, "contact", "wftk@vivtek.com");
   xml_set (info, "extra_functions", "0");

   return (info);
}
XML * USER_localxml_get (WFTK_ADAPTOR * ad, va_list args) {
   char path[256];
   char *id = (char *) 0;
   FILE *file;
   XML * ret;

   if (args) id = va_arg (args, char *);
   if (!id) {
      xml_set (ad->parms, "error", "No ID given.");
      return (XML *) 0;
   }

   strcpy (path, xml_attrval (ad->parms, "dir"));
   strcat (path, id);
   strcat (path, ".xml");
   file = fopen (path, "r");
   if (!file) {
      xml_set (ad->parms, "error", "Couldn't open file for reading.");
      return (XML *) 0;
   }

   ret = xml_read (file);
   fclose (file);

   return ret;

}

XML * USER_localxml_auth (WFTK_ADAPTOR * ad, va_list args) {
   XML * user = 0;
   char * password;

   if (args) user = va_arg (args, XML *);
   if (!user) {
      xml_set (ad->parms, "error", "No user given.");
      return (XML *) 0;
   }
   password = va_arg (args, char *);

   if (!strcmp (password, xml_attrval (user, "password"))) return user;
   return (XML *) 0;
}
