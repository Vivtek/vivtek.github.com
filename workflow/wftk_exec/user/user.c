#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "user.h"
#include "../localdefs.h"

/* ------------------------------------------------------------------ */
/* user.c                                                             */
/* This implements the user and group API for wftk.                   */
/* ------------------------------------------------------------------ */
void _base64decode (char * buf, const char * encoded)
{
   int bytes = 0;
   unsigned long chunk = 0;
   unsigned long remainder = 0;

repeat:
   while (*encoded && bytes < 4) {
      if      (*encoded == '=') chunk = chunk * 64;
      else if (*encoded  >  96) chunk = chunk * 64 + (*encoded - 71);
      else if (*encoded  >  64) chunk = chunk * 64 + (*encoded - 65);
      else if (*encoded  >  47) chunk = chunk * 64 + (*encoded + 4);
      else if (*encoded ==  43) chunk = chunk * 64 + 62;
      else if (*encoded ==  47) chunk = chunk * 64 + 63;
      else                      chunk = chunk * 64;

      bytes ++;
      encoded ++;
   }

   bytes = 2;
   while (chunk > 0 && bytes > -1) {
      remainder = chunk % (1 << (bytes * 8));
      *buf++ = (chunk - remainder) / (1 << (bytes * 8));
      chunk = remainder;
      bytes--;
   }
   bytes = 0;

   if (*encoded) goto repeat;

   *buf = '\0';
}
XML * user_authenticate(XML * cgi_env, const char * realm)
{
   XML * ret;
   char username[512];
   char * password;

   _base64decode (username, xml_attrval (cgi_env, "HTTP_AUTHORIZATION") + 6);

   password = strchr (username, ':');
   if (password) *password++ = '\0';
   else password = "";

   ret = user_get (username);
   if (ret) {
      if (!strcmp (password, xml_attrval (ret, "password"))) return (ret);
      xml_free (ret);
   }

   printf ("Status: 401 Authentication required\n");
   printf ("WWW-Authenticate: BASIC realm=\"%s\"\n\n", realm);

   return NULL;
}

int user_exists(const char * user)
{
   char buf[1024];
   FILE * file;
   sprintf (buf, "%s%s", USER_REPOSITORY, user);
   file = fopen (buf, "r");
   if (file) {
      fclose (file);
      return 1;
   }
   return 0;
}
int group_exists(const char * group)
{
   char buf[1024];
   FILE * file;
   sprintf (buf, "%s%s", GROUP_REPOSITORY, group);
   file = fopen (buf, "r");
   if (file) {
      fclose (file);
      return 1;
   }
   return 0;
}

XML * user_get (const char * username)
{
   char buf[1024];
   FILE * file;
   XML * ret;

   if (!strcmp (username, "anonymous")) {
      ret = xml_create ("user");
      xml_set (ret, "name", "anonymous");
      return (ret);
   }

   sprintf (buf, "%s%s.xml", USER_REPOSITORY, username);
   file = fopen (buf, "r");
   if (!file) return NULL;

   ret = xml_read (file);
   fclose (file);
   return (ret);
}
int user_save (XML * user)
{
   char buf[1024];
   FILE * file;

   sprintf (buf, "%s%s.xml", USER_REPOSITORY, xml_attrval (user, "name"));
   file = fopen (buf, "w+");
   if (!file) return 1;

   xml_write (file, user);
   fclose (file);
   return (0);
}
XML * group_get (const char * groupname)
{
   char buf[1024];
   FILE * file;
   XML * ret;

   sprintf (buf, "%s%s.xml", GROUP_REPOSITORY, groupname);
   file = fopen (buf, "r");
   if (!file) {
      ret = xml_create ("group");
      xml_set (ret, "name", groupname);
      return (ret);
   }

   ret = xml_read (file);
   fclose (file);
   return (ret);
}
int group_save (XML * group)
{
   char buf[1024];
   FILE * file;

   sprintf (buf, "%s%s.xml", GROUP_REPOSITORY, xml_attrval (group, "name"));
   file = fopen (buf, "w+");
   if (!file) return 1;

   xml_write (file, group);
   fclose (file);
   return (0);
}

int user_join (XML * user, XML * group)
{
   XML * child;
   child = xml_firstelem (group);
   while (child) {
      if (!strcmp (child->name, "user"))
         if (!strcmp (xml_attrval (child, "name"), xml_attrval (user, "name")))
            goto ensure_group;
      child = xml_nextelem (child);
   }
   child = xml_create ("user");
   xml_set (child, "name", xml_attrval (user, "name"));
   xml_append (group, child);

ensure_group:
   child = xml_firstelem (user);
   while (child) {
      if (!strcmp (child->name, "group"))
         if (!strcmp (xml_attrval (child, "name"), xml_attrval (group, "name")))
            return 0;
      child = xml_nextelem (child);
   }
   child = xml_create ("group");
   xml_set (child, "name", xml_attrval (group, "name"));
   xml_append (user, child);

   return 0;
}
int user_leave (XML * user, XML * group)
{
   XML * child;

   child = xml_firstelem (group);
   while (child) {
      if (!strcmp (child->name, "user")) {
         if (!strcmp (xml_attrval (child, "name"), xml_attrval (user, "name"))) {
            xml_delete (child);
            child = xml_firstelem (group);
         }
      }
      child = xml_nextelem (child);
   }

   child = xml_firstelem (user);
   while (child) {
      if (!strcmp (child->name, "group")) {
         if (!strcmp (xml_attrval (child, "name"), xml_attrval (group, "name"))) {
            xml_delete (child);
            child = xml_firstelem (user);
         }
      }
      child = xml_nextelem (child);
   }
}
int object_grant (XML * user_or_group, const char * class, const char * object, const char * permission)
{
   XML * child;
   child = xml_firstelem (user_or_group);
   while (child) {
      if (!strcmp (child->name, "object"))
         if (!strcmp (xml_attrval (child, "class"), class) &&
             !strcmp (xml_attrval (child, "object"), object) &&
             !strcmp (xml_attrval (child, "permission"), permission))
            return 0;
      child = xml_nextelem (child);
   }
   child = xml_create ("object");
   xml_set (child, "class", class);
   xml_set (child, "object", object);
   xml_set (child, "permission", permission);
   xml_append (user_or_group, child);
   return 0;
}
int object_revoke (XML * user_or_group, const char * class, const char * object, const char * permission)
{
   XML * child;
   int ok;
   child = xml_firstelem (user_or_group);
   while (child) {
      if (!strcmp (child->name, "object") &&
          !strcmp (xml_attrval (child, "class"), class)) {
         ok = 0;
         if (!object) ok = 1;
         if (!ok) if (!strcmp (xml_attrval (child, "object"), object)) ok = 1;
         if (ok) {
            ok = 0;
            if (!permission) ok = 1;
            if (!ok) if (!strcmp (xml_attrval (child, "permission"), permission)) ok = 1;
            if (ok) {
               xml_delete (child);
               child = xml_firstelem (user_or_group);
            }
         }
      }
      child = xml_nextelem (child);
   }
}
int group_include (XML * outgroup, XML * ingroup, const char * permission)
{
   XML * child;
   child = xml_firstelem (outgroup);
   while (child) {
      if (!strcmp (child->name, "group-include"))
         if (!strcmp (xml_attrval (child, "name"), xml_attrval (ingroup, "name")) &&
             !strcmp (xml_attrval (child, "permission"), permission))
            goto ensure_ingroup;
      child = xml_nextelem (child);
   }
   child = xml_create ("group-include");
   xml_set (child, "name", xml_attrval (ingroup, "name"));
   xml_set (child, "permission", permission);
   xml_append (outgroup, child);

ensure_ingroup:
   child = xml_firstelem (ingroup);
   while (child) {
      if (!strcmp (child->name, "group-included-by"))
         if (!strcmp (xml_attrval (child, "name"), xml_attrval (outgroup, "name")) &&
             !strcmp (xml_attrval (child, "permission"), permission))
            return 0;
      child = xml_nextelem (child);
   }
   child = xml_create ("group-included-by");
   xml_set (child, "name", xml_attrval (outgroup, "name"));
   xml_set (child, "permission", permission);
   xml_append (ingroup, child);

   return 0;
}
int group_unlink (XML * outgroup, XML * ingroup, const char * permission)
{
   XML * child;
   int ok;
   child = xml_firstelem (outgroup);
   while (child) {
      if (!strcmp (child->name, "group-include") &&
          !strcmp (xml_attrval (child, "name"), xml_attrval (ingroup, "name"))) {
         ok = 0;
         if (!permission) ok = 1;
         if (!ok) if (!strcmp (xml_attrval (child, "permission"), permission)) ok = 1;
         if (ok) {
            xml_delete (child);
            child = xml_firstelem (outgroup);
         }
      }
      child = xml_nextelem (child);
   }

   child = xml_firstelem (ingroup);
   while (child) {
      if (!strcmp (child->name, "group-included-by") &&
          !strcmp (xml_attrval (child, "name"), xml_attrval (outgroup, "name"))) {
         ok = 0;
         if (!permission) ok = 1;
         if (!ok) if (!strcmp (xml_attrval (child, "permission"), permission)) ok = 1;
         if (ok) {
            xml_delete (child);
            child = xml_firstelem (ingroup);
         }
      }
      child = xml_nextelem (child);
   }
}

int _user_perm_cmp (const char * perm1, const char * permission)
{
   if (!strcmp (perm1, "own")) return (1);
   if (!strcmp (perm1, permission)) return (1);
   if (!strcmp (permission, "view")) return (1);
   return (0);
}
int user_perm (XML * user, const char * class, const char * object, const char * permission)
{
   XML * group;
   XML * child;

   child = xml_firstelem (user);
   while (child) {
      if (!strcmp (child->name, "object")) {
         if (!strcmp (xml_attrval (child, "class"), class) &&
             !strcmp (xml_attrval (child, "object"), object)) {
            if (_user_perm_cmp (xml_attrval (child, "permission"), permission)) return (1);
         }
      } else if (!strcmp (child->name, "group")) {
         if (!strcmp (xml_attrval (child, "name"), "admin")) return (1);
         group = group_get (xml_attrval (child, "name"));
         if (group) {
            if (user_perm (group, class, object, permission)) {
               xml_free (group);
               return (1);
            }
            xml_free (group);
         }
      } else if (!strcmp (child->name, "group-include")) {
         if (_user_perm_cmp (xml_attrval (child, "permission"), permission)) {
            group = group_get (xml_attrval (child, "name"));
            if (group) {
               if (user_perm (group, class, object, permission)) {
                  xml_free (group);
                  return (1);
               }
               xml_free (group);
            }
         }
      }
      child = xml_nextelem (child);
   }
   if (strcmp (user->name, "group") && strcmp (xml_attrval (user, "name"), "everybody")) {
      group = group_get ("everybody");
      if (group) {
         if (user_perm (group, class, object, permission)) {
            xml_free (group);
            return (1);
         }
         xml_free (group);
      }
   }
   return (0);
}
int user_perm_group (XML * user, const char * groupname, const char * permission)
{
   XML * group;
   XML * child;

   if (!strcmp (groupname, "everybody")) return (1);
   if (!strcmp (groupname, "")) return (1);

   child = xml_firstelem (user);
   while (child) {
      if (!strcmp (child->name, "group")) {
         if (!strcmp (xml_attrval (child, "name"), "admin")) return (1);
         if (!strcmp (xml_attrval (child, "name"), groupname)) return (1);

         group = group_get (xml_attrval (child, "name"));
         if (group) {
            if (user_perm_group (group, groupname, permission)) {
               xml_free (group);
               return (1);
            }
            xml_free (group);
         }
      } else if (!strcmp (child->name, "group-include")) {
         if (_user_perm_cmp (xml_attrval (child, "permission"), permission)) {
            if (!strcmp (xml_attrval (child, "name"), groupname)) return (1);

            group = group_get (xml_attrval (child, "name"));
            if (group) {
               if (user_perm_group (group, groupname, permission)) {
                  xml_free (group);
                  return (1);
               }
               xml_free (group);
            }
         }
      }
      child = xml_nextelem (child);
   }
   if (strcmp (user->name, "group") && strcmp (xml_attrval (user, "name"), "everybody")) {
      group = group_get ("everybody");
      if (group) {
         if (user_perm_group (group, groupname, permission)) {
            xml_free (group);
            return (1);
         }
         xml_free (group);
      }
   }
   return (0);
}

void _user_add_to_list (XML * ret, XML * user, const char * class, const char * object, const char * permission)
{
   XML * child;
   XML * piece;
   int ok;
   child = xml_firstelem (user);
   while (child) {
      if (!strcmp (child->name, "object")) {
         if ((!strcmp (class, ".") || !strcmp (class, xml_attrval (child, "class"))) &&
             (!strcmp (object, ".") || !strcmp (object, xml_attrval (child, "object"))) &&
             (!strcmp (permission, ".") ||
              _user_perm_cmp (xml_attrval (child, "permission"), permission))) {
               piece = xml_create ("object");
               xml_set (piece, "class", xml_attrval (child, "class"));
               xml_set (piece, "object", xml_attrval (child, "object"));
               xml_set (piece, "permission", xml_attrval (child, "permission"));
               xml_append (ret, piece);
         }
      } else if (!strcmp (child->name, "group-include")) {
         if (!strcmp (permission, ".") ||
             _user_perm_cmp (xml_attrval (child, "permission"), permission)) {
             piece = xml_create ("group-include");
             xml_set (piece, "name", xml_attrval (child, "name"));
             xml_set (piece, "permission", xml_attrval (child, "permission"));
             xml_append (ret, piece);
         }
      } else if (!strcmp (child->name, "group")) {
         piece = group_get (xml_attrval (child, "name"));
         if (piece) {
            _user_add_to_list (ret, piece, class, object, permission);
            xml_free (piece);
         }
      }
      child = xml_nextelem (child);
   }
}

XML * user_list (XML * user, const char * class, const char * object, const char * permission)
{
   XML * ret;

   ret = xml_create ("user_list");
   _user_add_to_list (ret, user, class ? class : ".", object ? object : ".", permission ? permission : ".");
   return (ret);
}
void user_groups_list (XML * user, XML * holder, const char * permission)
{
   XML * elem;
   XML * check;
   XML * newone;
   XML * group;

   if (!permission) permission = "view";
   if (!*permission) permission = "view";

   elem = xml_firstelem (user);
   while (elem) {
      if (!strcmp (elem->name, "group") ||
            (!strcmp (elem->name, "group-include") &&
            _user_perm_cmp (xml_attrval (elem, "permission"), permission))) {
         check = xml_firstelem (holder);
         while (check) {
            if (!strcmp (xml_attrval (check, "name"), xml_attrval (elem, "name"))) break;
            check = xml_nextelem (check);
         }
         if (!check) {
            group = group_get (xml_attrval (elem, "name"));
            if (group) {
               newone = xml_create ("group");
               xml_set (newone, "name", xml_attrval (elem, "name"));
               xml_set (newone, "label", xml_attrval (elem, "label"));
               if (!*xml_attrval (newone, "label")) xml_set (newone, "label", xml_attrval (elem, "name"));

               xml_append (holder, newone);

               user_groups_list (group, holder, permission);
            }
            xml_free (group);
         }
      }

      elem = xml_nextelem (elem);
   }
}
void _group_users_search (XML * group, XML * holder, const char * permission)
{
   XML * elem;
   XML * check;
   XML * newone;
   XML * othergroup;

   if (!permission) permission = "view";
   if (!*permission) permission = "view";

   elem = xml_firstelem (group);
   while (elem) {
      if (!strcmp (elem->name, "group-included-by") &&
          _user_perm_cmp (xml_attrval (elem, "permission"), permission)) {
         check = xml_firstelem (holder);
         while (check) {
            if (!strcmp (check->name, "group") &&
                !strcmp (xml_attrval (check, "name"), xml_attrval (elem, "name"))) break;
            check = xml_nextelem (check);
         }
         if (!check) {
            othergroup = group_get (xml_attrval (elem, "name"));
            if (othergroup) {
               newone = xml_create ("group");
               xml_set (newone, "name", xml_attrval (elem, "name"));

               xml_append (holder, newone);

               _group_users_search (othergroup, holder, permission);
            }
            xml_free (othergroup);
         }
      }

      if (!strcmp (elem->name, "user")) {
         check = xml_firstelem (holder);
         while (check) {
            if (!strcmp (check->name, "user") &&
                !strcmp (xml_attrval (check, "name"), xml_attrval (elem, "name"))) break;
            check = xml_nextelem (check);
         }
         if (!check) {
            newone = xml_create ("user");
            xml_set (newone, "name", xml_attrval (elem, "name"));
            xml_append (holder, newone);
         }
      }

      elem = xml_nextelem (elem);
   }
}
void group_users_list (XML * group, XML * holder, const char * permission)
{
   XML * elem;

   _group_users_search (group, holder, permission);
   elem = xml_firstelem (holder);
   while (elem) {
      if (!strcmp (elem->name, "group")) {
         xml_delete (elem);
         elem = xml_firstelem (holder);
      } else {
         elem = xml_nextelem (elem);
      }
   }
}

void group_users_list_detailed (XML * group, XML * holder, const char * permission)
{
   XML * elem;
   XML * user;
   ATTR * attr;

   group_users_list (group, holder, permission);

   elem = xml_firstelem (holder);
   while (elem) {
      user = user_get (xml_attrval (elem, "name"));
      if (user) {
         attr = user->attrs;
         while (attr) {
            xml_set (elem, attr->name, attr->value);
            attr = attr->next;
         }
         xml_free (user);
      }
      elem = xml_nextelem (elem);
   }
}
