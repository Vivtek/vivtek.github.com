#include "user.h"
#include "xmlcgi.h"

XML * current_user;
XML * user = NULL;
XML * edited;
XML * cgi;
XML * environment;
XML * query;


int main (int argc, char * argv[])
{
   ATTR * attr;
   char * field;

   cgi = cgi_init ();
   environment = xml_loc (cgi, "cgicall.environment");
   query = xml_loc (cgi, "cgicall.query");

   current_user = user_authenticate (environment, "wftk workflow");
   edited = current_user;
   if (!current_user) {
      printf ("Content-type: text/html\n\n");
      printf ("<html><head><title>User authentication required</title></head>\n");
      printf ("<body><h2>User authentication required </h2><hr>\n");
      printf ("A valid user authentication response is required to access this area.\n");
      printf ("</body></html>\n");
      xml_free (cgi);
      exit(0);
   }

   if (strcmp ("", xml_attrval (query, "user"))) {
      user = user_get (xml_attrval (query, "user"));
      if (!user) {
         printf ("Content-type: text/html\n\n");
         printf ("<html><head><title>Unknown user '%s'</title></head>\n", xml_attrval (query, "user"));
         printf ("<body><h2>Unknown user '%s'</h2><hr>\n", xml_attrval (query, "user"));
         printf ("The system was unable to locate the username you entered.\n</body></html>\n");
         xml_free (cgi);
         xml_free (current_user);
         exit(0);
      }
      edited = user;
      if (!user_perm (current_user, "user", xml_attrval (query, "user"), "edit")) {
         printf ("Content-type: text/html\n\n");
         printf ("<html><head><title>Insufficient privilege</title></head>\n");
         printf ("<body><h2>Insufficient privilege</h2><hr>\n");
         printf ("You don't have permission to edit the personal information for user '%s'.\n", xml_attrval (query, "user"));
         printf ("</body></html>\n");
         xml_free (cgi);
         xml_free (current_user);
         exit(0);
      }
   }

   attr = query->attrs;
   while (attr) {
      if (strcmp (attr->name, "user") && strcmp (attr->name, "name")) {
         xml_set (edited, attr->name, attr->value);
      }
      attr = attr->next;
   }

   user_save (edited);

   printf ("Content-type: text/html\n\n");
   printf ("<html><head><title>Edit user information for '%s'</title></head>\n", xml_attrval (edited, "name"));
   printf ("<body><h2>User information for '%s'</h2><hr>\n", xml_attrval (edited, "name"));
   printf ("<form action=\"%s\" method=post>\n", xml_attrval (environment, "SCRIPT_NAME"));
   if (strcmp (xml_attrval (query, "user"), "")) {
      printf ("<input type=hidden name=user value=\"%s\">\n", xml_attrval (query, "user"));
   }
   printf ("<table>\n");
   attr = edited->attrs;
   while (attr) {
      if (!strcmp (attr->name, "user") || !strcmp (attr->name, "name")) {
      } else if (!strcmp (attr->name, "password")) {
         /*printf ("<tr><td>Password</td><td><input name=password></td></tr>\n");*/
      } else if (!strcmp (attr->name, "fullname")) {
         printf ("<tr><td>Full name</td><td><input name=fullname value=\"%s\"></td></tr>\n", attr->value);
      } else if (!strcmp (attr->name, "email")) {
         printf ("<tr><td>Email address</td><td><input name=email value=\"%s\"></td></tr>\n", attr->value);
      } else {
         printf ("<tr><td>%s</td><td><input name=\"%s\" value=\"%s\"></td></tr>\n", attr->name, attr->name, attr->value);
      }
      attr = attr->next;
   }
   printf ("<tr><td colspan=2><center><input type=submit value=\"Update user info\"></center></td></tr>\n");
   printf ("</table>\n");
   printf ("</form></body></html>\n");

   xml_free (cgi);
   if (user) xml_free (user);
   xml_free (current_user);
}
