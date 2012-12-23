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
   "new",
   "load",
   "save",
   "delete",
   "archive"
};

XML * DSREP_localxml_init (WFTK_ADAPTOR * ad, va_list args);
XML * DSREP_localxml_free (WFTK_ADAPTOR * ad, va_list args);
XML * DSREP_localxml_info (WFTK_ADAPTOR * ad, va_list args);
XML * DSREP_localxml_new (WFTK_ADAPTOR * ad, va_list args);
XML * DSREP_localxml_load (WFTK_ADAPTOR * ad, va_list args);
XML * DSREP_localxml_save (WFTK_ADAPTOR * ad, va_list args);
XML * DSREP_localxml_delete (WFTK_ADAPTOR * ad, va_list args);
XML * DSREP_localxml_archive (WFTK_ADAPTOR * ad, va_list args);

static WFTK_API_FUNC vtab[] = 
{
   DSREP_localxml_init,
   DSREP_localxml_free,
   DSREP_localxml_info,
   DSREP_localxml_new,
   DSREP_localxml_load,
   DSREP_localxml_save,
   DSREP_localxml_delete,
   DSREP_localxml_archive
};

static struct adaptor_info _DSREP_localxml_info =
{
   8,
   names,
   vtab
};
struct adaptor_info * DSREP_localxml_get_info ()
{
   return & _DSREP_localxml_info;
}
XML * DSREP_localxml_init (WFTK_ADAPTOR * ad, va_list args) {
   struct stat statbuf;
   const char * parms;
   char directory[256];
   char * end;

   parms = xml_attrval (ad->parms, "parm");
   if (!*parms) parms = config_get_value (ad->session, "dsrep.localxml.directory");

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
XML * DSREP_localxml_free (WFTK_ADAPTOR * ad, va_list args) { return (XML *) 0; }
XML * DSREP_localxml_info (WFTK_ADAPTOR * ad, va_list args) {
   XML * info;

   info = xml_create ("info");
   xml_set (info, "type", "dsrep");
   xml_set (info, "name", "localxml");
   xml_set (info, "ver", "1.0.0");
   xml_set (info, "compiled", __TIME__ " " __DATE__);
   xml_set (info, "author", "Michael Roberts");
   xml_set (info, "contact", "wftk@vivtek.com");
   xml_set (info, "extra_functions", "0");

   return (info);
}
XML * DSREP_localxml_new  (WFTK_ADAPTOR * ad, va_list args)
{
   char path[256];
   struct stat statbuf;
   char * id = (char *) 0;
   FILE * file;
   XML * ret = xml_create ("datasheet");

   if (args) id = va_arg (args, char *);
   if (id) {
      strcpy (path, xml_attrval (ad->parms, "dir"));
      strcat (path, id);
      strcat (path, ".xml");
      if (stat (path, &statbuf) == -1) {
         file = fopen (path, "w");
         if (file) {
            xml_set (ret, "id", id);
            xml_write (file, ret);
            fclose (file);
         } else {
            xml_set (ad->parms, "error", "Couldn't open file for writing.");
         }
      } else {
         xml_set (ad->parms, "error", "File already exists.");
      }
   }
   return ret;
}
XML * DSREP_localxml_load (WFTK_ADAPTOR * ad, va_list args) {
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
XML * DSREP_localxml_save (WFTK_ADAPTOR * ad, va_list args) {
   char path[256];
   struct stat statbuf;
   XML  * ds = (XML *) 0;
   FILE * file;
   FILE * _index;
   XML  * index;
   int  counter;

   if (args) ds = va_arg (args, XML *);
   if (!ds) {
      xml_set (ad->parms, "error", "No datasheet given.");
      return (XML *) 0;
   }

   if (*xml_attrval (ds, "id")) {
      strcpy (path, xml_attrval (ad->parms, "dir"));
      strcat (path, xml_attrval (ds, "id"));
      strcat (path, ".xml");
      file = fopen (path, "w");
   } else {
      /* Find a unique ID. */
      strcpy (path, xml_attrval (ad->parms, "dir"));
      strcat (path, "index");
      if (stat (path, &statbuf) == -1) {
         _index = fopen (path, "w");
         if (!_index) {
            xml_set (ad->parms, "error", "Unable to create index file.");
            return (XML *) 0;
         }
         index = xml_create ("index");
      } else {
         _index = fopen (path, "r+");
         if (_index) {
            index = xml_read (_index);
            rewind (_index);
         } else {
            xml_set (ad->parms, "error", "Unable to create index file.");
            return (XML *) 0;
         }
      }
      if (!index) {
         xml_set (ad->parms, "error", "Directory index file corrupt.");
         return (XML *) 0;
      }

      if (!xml_attrval (index, "counter")) xml_set (index, "counter", "0");

      counter = xml_attrvalnum (index, "counter");
      do {
         counter ++;
         xml_setnum (ds, "id", counter);
         strcpy (path, xml_attrval (ad->parms, "dir"));
         strcat (path, xml_attrval (ds, "id"));
         strcat (path, ".xml");
      } while (stat (path, &statbuf) != -1);
      file = fopen (path, "w");

      xml_setnum (index, "counter", counter);
      xml_write (_index, index);
      fclose (_index);
   }

   if (file) {
      xml_write (file, ds);
      fclose (file);
   } else {
      xml_set (ad->parms, "error", "Couldn't open file for writing.");
      return (XML *) 0;
   }

   return ds;
}
XML * DSREP_localxml_delete (WFTK_ADAPTOR * ad, va_list args) {
   char path[256];
   char * id = (char *) 0;

   if (args) id = va_arg (args, char *);
   if (!id) {
      xml_set (ad->parms, "error", "No ID given.");
      return (XML *) 0;
   }

   strcpy (path, xml_attrval (ad->parms, "dir"));
   strcat (path, id);
   strcat (path, ".xml");
   if (-1 == unlink (path)) {
      if (errno == EACCES) xml_set (ad->parms, "error", "Insufficient filesystem access.");
   }
   return (XML *) 0;
}
XML * DSREP_localxml_archive (WFTK_ADAPTOR * ad, va_list args) { return (XML *) NULL; }
