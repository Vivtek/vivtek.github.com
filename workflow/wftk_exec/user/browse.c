#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "user.h"
#include "xmlcgi.h"
#include "../localdefs.h"

/* ------------------------------------------------------------------ */
/* browse.c                                                           */
/* A CGI program for folder-oriented, permission-based hierarchical   */
/* browsing.  This program is part of the wftk workflow toolkit and   */
/* is distributed under the terms of the GNU license.                 */
/* Copyright (c) 2000, Vivtek: wftk@vivtek.com                        */
/* ------------------------------------------------------------------ */
XML * cgi;
XML * environment;
XML * input;

XML * current_user;

XML * view;

int  folder_append (XML * view, XML * elem);
void dirview_scan (XML * dv, XML * entity, const char *state)
{
   XML * elem;
   XML * direlem;
   XML * group;

   elem = xml_firstelem (entity);
   while (elem) {
      if (!strcmp (elem->name, "object")) {
         direlem = xml_create ("object");
         if (*xml_attrval (elem, "label")) {
            xml_set (direlem, "label", xml_attrval (elem, "label"));
         } else {
            xml_set (direlem, "label", xml_attrval (elem, "object"));
         }
         xml_append (dv, direlem);
      } else if (!strcmp (elem->name, "group") || !strcmp (elem->name, "group-include")) {
         direlem = xml_create ("folder");
         xml_set (direlem, "name", xml_attrval (elem, "name"));
         if (*xml_attrval (elem, "label")) {
            xml_set (direlem, "label", xml_attrval (elem, "label"));
         } else {
            xml_set (direlem, "label", xml_attrval (elem, "name"));
         }
         if (folder_append (dv, direlem)) {
            group = group_get (xml_attrval (elem, "name"));
            dirview_scan (direlem, group, state);
            xml_free (group);
         }
      }
      elem = xml_nextelem (elem);
   }
}
int folder_append (XML * view, XML * elem)
{
   XML * check;
   const char * name = xml_attrval (elem, "name");

   check = view;
   while (check) {
      if (!strcmp (xml_attrval (check, "name"), name)) {
         xml_free (elem);
         return (0);
      }
      check = check->parent;
   }

   check = xml_firstelem (view);
   while (check) {
      if (!strcmp (check->name, "folder")) {
         if (!strcmp (xml_attrval (check, "name"), name)) {
            xml_free (elem);
            return (0);
         }
      }
      check = xml_nextelem (check);
   }

   xml_append (view, elem);
   return (1);
}
void dirview_create (XML * dv, XML * cur_user)
{
   XML * elem;
   XML * direlem;

   dirview_scan (dv, cur_user, xml_attrval (dv, "state"));
}
void dirview_show (FILE * out, XML * dv)
{
   XML * elem;
   const char * link;

   elem = xml_firstelem (dv);
   if (!elem) return;

   fprintf (out, "<ul>\n");
   while (elem) {
      if (!strcmp (elem->name, "folder")) {
         fprintf (out, "<li> <strong>%s</strong>\n", xml_attrval (elem, "label"));
         dirview_show (out, elem);
      } else {
         link = xml_attrval (elem, "link");
         if (*link) {
            fprintf (out, "<li> <a href=\"%s\">%s</a>\n", link, xml_attrval (elem, "label"));
         } else {
            fprintf (out, "<li> %s\n", xml_attrval (elem, "label"));
         }
      }
      elem = xml_nextelem (elem);
   }
   fprintf (out, "</ul>\n");
}

int main (int argc, char * argv[])
{
   XML * elem;

   cgi = cgi_init ();
   environment = xml_loc (cgi, "cgicall.environment");
   input = xml_loc (cgi, "cgicall.query");

   current_user = user_authenticate (environment, AUTH_DOMAIN);
   if (!current_user) {
      printf ("<html><head><title>User authentication required</title></head>\n");
      printf ("<body><h2>User authentication required </h2><hr>\n");
      printf ("A valid user authentication response is required to access this area.\n");
      printf ("</body></html>\n");
      xml_free (cgi);
      exit(0);
   }

   view = xml_create ("dirview");
   xml_set (view, "root", xml_attrval (input, "root"));
   xml_set (view, "curuser", xml_attrval (current_user, "name"));
   xml_set (view, "state", xml_attrval (input, "state"));

   dirview_create (view, current_user);

   printf ("Content-type: text/html\n\n");
   dirview_show (stdout, view);

   xml_free (cgi);
   xml_free (current_user);
   xml_free (view);
}
