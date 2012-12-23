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
   "version",
   "load"
};

XML * PDREP_localxml_init (WFTK_ADAPTOR * ad, va_list args);
XML * PDREP_localxml_free (WFTK_ADAPTOR * ad, va_list args);
XML * PDREP_localxml_info (WFTK_ADAPTOR * ad, va_list args);
XML * PDREP_localxml_version (WFTK_ADAPTOR * ad, va_list args);
XML * PDREP_localxml_load (WFTK_ADAPTOR * ad, va_list args);

static WFTK_API_FUNC vtab[] = 
{
   PDREP_localxml_init,
   PDREP_localxml_free,
   PDREP_localxml_info,
   PDREP_localxml_version,
   PDREP_localxml_load
};

static struct adaptor_info _PDREP_localxml_info =
{
   5,
   names,
   vtab
};
struct adaptor_info * PDREP_localxml_get_info ()
{
   return & _PDREP_localxml_info;
}
XML * PDREP_localxml_init (WFTK_ADAPTOR * ad, va_list args) {
   struct stat statbuf;
   char directory[256];
   char * end;

   const char * parms = xml_attrval (ad->parms, "parms");
   if (!*parms) parms = config_get_value (ad->session, "pdrep.localxml.directory");

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
XML * PDREP_localxml_free (WFTK_ADAPTOR * ad, va_list args) { return (XML *) 0; }
XML * PDREP_localxml_info (WFTK_ADAPTOR * ad, va_list args) {
   XML * info;

   info = xml_create ("info");
   xml_set (info, "type", "pdrep");
   xml_set (info, "name", "localxml");
   xml_set (info, "ver", "1.0.0");
   xml_set (info, "compiled", __TIME__ " " __DATE__);
   xml_set (info, "author", "Michael Roberts");
   xml_set (info, "contact", "wftk@vivtek.com");
   xml_set (info, "extra_functions", "0");

   return (info);
}
XML * PDREP_localxml_version  (WFTK_ADAPTOR * ad, va_list args)
{
   char path[256];
   char * id = (char *) 0;
   FILE * file;
   XML * index;
   XML * value;

   xml_set (ad->parms, "error", "");
   if (args) id = va_arg (args, char *);
   if (!id) {
      xml_set (ad->parms, "error", "No ID given.");
      return (XML *) 0;
   }

   strcpy (path, xml_attrval (ad->parms, "dir"));
   strcat (path, id);
   strcat (path, "__versions.xml");
   file = fopen (path, "r");
   if (!file) {
      xml_set (ad->parms, "error", "Procdef master file not found.");
      return (XML *) 0;
   }

   index = xml_read (file);
   if (!index) {
      xml_set (ad->parms, "error", "Procdef master file corrupted.");
      return (XML *) 0;
   }

   value = xml_create ("value");
   xml_set (value, "value", xml_attrval (index, "ver"));

   xml_free (index);
   fclose (file);

   return value;
}
XML * PDREP_localxml_load (WFTK_ADAPTOR * ad, va_list args) {
   char path[256];
   char *id = (char *) 0;
   char *ver = (char *) 0;
   FILE *file;
   XML * ret;

   if (args) id = va_arg (args, char *);
   if (!id) {
      xml_set (ad->parms, "error", "No ID given.");
      return (XML *) 0;
   }
   ver = va_arg (args, char *);
   if (!ver) {
      xml_set (ad->parms, "error", "No version given.");
      return (XML *) 0;
   }

   strcpy (path, xml_attrval (ad->parms, "dir"));
   strcat (path, id);
   strcat (path, "_");
   strcat (path, ver);
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
