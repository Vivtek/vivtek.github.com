#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <malloc.h>
#include "../xmlapi/xmlapi.h"
#include "wftk_internals.h"

WFTK_ADAPTOR * config_get_adaptor (void * session, int adaptor_class, const char * descriptor, int length);
WFTK_ADAPTORLIST * config_get_adaptorlist (void * session, int adaptor_class);

WFTK_ADAPTOR * wftk_get_adaptor (void * session, int adaptor_class, const char * adaptor_descriptor)
{
   const char * mark;
   WFTK_ADAPTOR * ad;
   WFTK_ADAPTOR_LIST * list;

   /* Maybe we've already got this adaptor in our list? */
   if (session) {
      list = ((WFTK_SESSION *) session)->ads;
      while (list) {
         if (list->ad->num == adaptor_class) {
            if (!adaptor_descriptor) return (list->ad);
            if (!strcmp (adaptor_descriptor, xml_attrval (list->ad->parms, "spec"))) return (list->ad);
         }
         list = list->next;
      }
   }

   /* Ask the config module for the initial setup.  If this is an unknown adaptor name, nothing's alloc'd and we return. */
   if (adaptor_descriptor) {
      for (mark = adaptor_descriptor; *mark && *mark != ':'; mark++);
      ad = config_get_adaptor (session, adaptor_class, adaptor_descriptor, mark - adaptor_descriptor);
   } else {
      ad = config_get_adaptor (session, adaptor_class, NULL, 0);
   }
   if (!ad) return ((WFTK_ADAPTOR *) 0);

   /* Get the parms structure started.  The adaptor initializer will take it from here. */
   ad->parms = xml_create ("parms");
   if (adaptor_descriptor) {
      if (*mark) mark++;
      xml_set (ad->parms, "parm", mark);
   }

   /* Call the initializer. */
   xml_set (ad->parms, "error", "");
   (ad->vtab[0]) (ad, (va_list) 0);
   if (*xml_attrval (ad->parms, "error")) {
      config_debug_message ('A', "[%d]init: %s", adaptor_class, xml_attrval (ad->parms, "error")); /* TODO: not optimal */
      xml_free (ad->parms);
      free (ad);
      return ((WFTK_ADAPTOR *) 0);
   }

   /* Add the new adaptor to the list. */
   if (session) {
      list = malloc (sizeof (WFTK_ADAPTOR_LIST));
      list->next = ((WFTK_SESSION *) session)->ads;
      list->ad = ad;
      ((WFTK_SESSION *) session)->ads = list;
   }

   /* Return the initialized adaptor. */
   return (ad);
}
void wftk_free_adaptor (void * session, WFTK_ADAPTOR * ad)
{
   if (session) return;

   (ad->vtab[1]) (ad, (va_list) 0);  /* Let the adaptor do whatever it needs to do in order to clean up. */
   xml_free (ad->parms);
   free (ad);
}
XML * wftk_call_adaptor (WFTK_ADAPTOR * ad, const char * function, ...)
{
   va_list argptr;
   XML * retval;
   int func;

   if (!ad) return (XML *) 0;

   for (func = 0; func < ad->nfuncs; func++) {
      if (!strcmp (function, ad->names[func])) break;
   }
   if (func == ad->nfuncs) {
      config_debug_message ('A', "[%d]%s: function '%s' unknown", ad->num, xml_attrval (ad->parms, "spec"), function);
      return ((XML *) 0);
   }

   xml_set (ad->parms, "error", "");

   va_start (argptr, function);
   retval = (ad->vtab[func]) (ad, argptr);
   va_end (argptr);

   if (*xml_attrval (ad->parms, "error")) {
      config_debug_message ('A', "[%d]%s:%s - %s", ad->num, xml_attrval (ad->parms, "spec"), function, xml_attrval (ad->parms, "error"));
   }

   return (retval);
}
WFTK_ADAPTORLIST * wftk_get_adaptorlist (void * session, int adaptorclass)
{
   return (config_get_adaptorlist (session, adaptorclass));
}
void wftk_free_adaptorlist (void * session, WFTK_ADAPTORLIST * list)
{
   int i;
   if (!list) return;

   for (i=0; i < list->count; i++) {
      wftk_free_adaptor (session, list->ads[i]);
   }

   free (list);
}
int wftk_call_adaptorlist (WFTK_ADAPTORLIST * list, const char * function, ...)
{
   va_list argptr;
   int i;
   XML * retval;
   int func;

   if (!list) return 0;

   va_start (argptr, function);

   for (i=0; i < list->count; i++) {
      if (!(list->ads[i])) continue;
      for (func = 0; func < list->ads[i]->nfuncs; func++) {
         if (!strcmp (function, list->ads[i]->names[func])) break;
      }
      if (func == list->ads[i]->nfuncs) {
         config_debug_message ('A', "[%d]%s: function '%s' unknown", list->ads[i]->num, xml_attrval (list->ads[i]->parms, "spec"), function);
      } else {
         xml_set (list->ads[i]->parms, "error", "");

         retval = (list->ads[i]->vtab[func]) (list->ads[i], argptr);

         if (*xml_attrval (list->ads[i]->parms, "error")) {
            config_debug_message ('A', "[%d]%s:%s - %s", list->ads[i]->num, xml_attrval (list->ads[i]->parms, "spec"), function, xml_attrval (list->ads[i]->parms, "error"));
         }
         if (retval) xml_free (retval);
      }
   }

   va_end (argptr);

   return (1);
}
