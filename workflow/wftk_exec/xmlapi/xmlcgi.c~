#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xmlapi.h"

void xml_urldecode (XML * element, char *string);
char * cgi_init_env_vars[] = {
   "SERVER_TYPE",
   "SERVER_NAME",
   "GATEWAY_INTERFACE",
   "SERVER_PROTOCOL",
   "SERVER_PORT",
   "REQUEST_METHOD",
   "PATH_INFO",
   "PATH_TRANSLATED",
   "SCRIPT_NAME",
   "QUERY_STRING",
   "REMOTE_HOST",
   "REMOTE_ADDR",
   "AUTH_TYPE",
   "REMOTE_USER",
   "REMOTE_IDENT",
   "CONTENT_TYPE",
   "CONTENT_LENGTH",
   "HTTP_ACCEPT",
   "HTTP_USER_AGENT",
   "HTTP_REFERER",
   "HTTP_AUTHORIZATION",
   "HTTP_COOKIE",
   "HTTP_HOST",
   "HTTP_CONNECTION",
   "HTTP_PRAGMA",
   "HTTPS",
   "HTTP_ACCEPT_LANGUAGE",
   "HTTP_ACCEPT_CHARSET",
   "HTTP_ACCEPT_ENCODING",
   "SERVER_PORT_SECURE",
   ""
};
XML * cgi_init () {
   XML * ret;
   XML * env;
   XML * query;
   char ** var;
   char * val;

   val = getenv ("SERVER_NAME");
   if (!val) return (NULL); /* This seems like a sufficient test... */

   ret = xml_create ("cgicall");
   env = xml_create ("environment");
   query = xml_create ("query");
   xml_append (ret, env);
   xml_append (ret, query);

   var = cgi_init_env_vars;
   while (var && *var && **var) {
      val = getenv (*var);
      if (val) xml_set (env, *var, val);
      var ++;
   }

   val = (char *) malloc (strlen (xml_attrval (env, "QUERY_STRING")) + 1);
   strcpy (val, xml_attrval (env, "QUERY_STRING"));
   xml_urldecode (query, val);
   free (val);
   if (!strcmp (xml_attrval (env, "REQUEST_METHOD"), "POST") &&
       !strcmp (xml_attrval (env, "CONTENT_TYPE"), "application/x-www-form-urlencoded")) {
      val = (char *) malloc (atoi (xml_attrval (env, "CONTENT_LENGTH")) + 1);
#ifdef WIN32
      _setmode (_fileno (stdin), _O_BINARY);
#endif
      val [atoi (xml_attrval (env, "CONTENT_LENGTH"))] = '\0';
      fread (val, 1, atoi (xml_attrval (env, "CONTENT_LENGTH")), stdin);
      xml_urldecode (query, val);
      free (val);
   }

   return (ret);
}

void xml_urldequote (char * str);
void xml_urldecode (XML * xml, char * q)
{
   char * and;
   char * equals;
   and = strchr (q, '&');
   equals = strchr (q, '=');
   while (equals) {
      if (and) *and = '\0';
      *equals = '\0';
      xml_urldequote (q);
      xml_urldequote (equals + 1);
      xml_set (xml, q, equals + 1);
      if (!and) break;
      q = and + 1;
      and = strchr (q, '&');
      equals = strchr (q, '=');
   }
}

#define HEXDIGITVAL(x) ((x>='0'&&x<='9')?(x-'0'):((x>='a'&&x<='f')?(x-'a'+10):((x>='A'&&x<='F')?(x-'A'+10):0)))
void xml_urldequote (char * str)
{
   while (*str) {
      if (str[0] == '+') str[0] = ' ';
      if (str[0] == '%') {
         if (str[1] && str[2]) {
            str[0] = 16 * HEXDIGITVAL (str[1]) + HEXDIGITVAL (str[2]);
            strcpy (str + 1, str + 3);
         }
      }
      str++;
   }
}
