#include "user.h"

XML * user;
XML * group;

ATTR * attr;

XML * holder;

XML * elem;

int main (int argc, char * argv[])
{
   if (argc < 2) {
      printf ("user: No command specified.\n");
      exit (1);
   }
 
   if (!strcmp (argv[1], "newuser")) {
      if (argc < 4) {
         printf ("user newuser: User and password not specified.\n");
         exit (1);
      }
      user = xml_create ("user");
      xml_set (user, "name", argv[2]);
      xml_set (user, "password", argv[3]);
      group = group_get ("everybody");
      user_join (user, group);
      user_save (user);
      group_save (group);
      xml_free (user);
      xml_free (group);
   } else if (!strcmp (argv[1], "newgroup")) {
      if (argc < 3) {
         printf ("user newgroup: Group name not specified.\n");
         exit (1);
      }
      group = xml_create ("group");
      xml_set (group, "name", argv[2]);
      group_save (group);
      xml_free (group);
   } else if (!strcmp (argv[1], "password")) {
      if (argc < 4) {
         printf ("user password: Username or new password missing.\n");
         exit (1);
      }
      user = user_get (argv[2]);
      if (!user) {
         printf ("user password: Unable to load user %s.\n", argv[2]);
         exit (1);
      }
      xml_set (user, "password", argv[3]);
      user_save (user);
      xml_free (user);
   } else if (!strcmp (argv[1], "auth")) {
      if (argc < 4) {
         printf ("user auth: Username or password missing.\n");
         exit (1);
      }
      user = user_get (argv[2]);
      if (!user) exit(1);
      if (!strcmp (argv[3], xml_attrval (user, "password"))) {
         attr = user->attrs;
         while (attr) {
            if (strcmp (attr->name, "password")) {
               printf ("%s:%s\n", attr->name, attr->value);
            }
            attr = attr->next;
         }
      }
      xml_free (user);
   } else if (!strcmp (argv[1], "listuser")) {
      if (argc < 3) {
         printf ("user listuser: Username missing.\n");
         exit (1);
      }
      user = user_get (argv[2]);
      if (!user) exit(1);
      attr = user->attrs;
      while (attr) {
         if (strcmp (attr->name, "password")) {
            printf ("%s:%s\n", attr->name, attr->value);
         }
         attr = attr->next;
      }
      xml_free (user);
   } else if (!strcmp (argv[1], "user")) {
      if (argc < 3) {
         printf ("user user: Username missing.\n");
         exit (1);
      }
      user = user_get (argv[2]);
      if (!user) {
         printf ("user user: Unable to load user %s.\n", argv[2]);
         exit (1);
      }
      xml_write (stdout, user);
      xml_free (user);
   } else if (!strcmp (argv[1], "group")) {
      if (argc < 3) {
         printf ("user group: Group name missing.\n");
         exit (1);
      }
      group = group_get (argv[2]);
      if (!group) {
         printf ("user group: Unable to load group %s.\n", argv[2]);
         exit (1);
      }
      xml_write (stdout, group);
      xml_free (group);
   } else if (!strcmp (argv[1], "join")) {
      if (argc < 4) {
         printf ("user join: Username or group missing.\n");
         exit (1);
      }
      user = user_get (argv[2]);
      if (!user) {
         printf ("user join: Unable to load user %s.\n", argv[2]);
         exit (1);
      }
      group = group_get (argv[3]);
      if (!group) {
         printf ("user join: Unable to load group %s.\n", argv[3]);
         xml_free (user);
         exit (1);
      }
      user_join (user, group);
      user_save (user);
      group_save (group);
      xml_free (user);
      xml_free (group);
   } else if (!strcmp (argv[1], "leave")) {
      if (argc < 4) {
         printf ("user leave: Username or group missing.\n");
         exit (1);
      }
      user = user_get (argv[2]);
      if (!user) {
         printf ("user leave: Unable to load user %s.\n", argv[2]);
         exit (1);
      }
      group = group_get (argv[3]);
      if (!group) {
         printf ("user leave: Unable to load group %s.\n", argv[3]);
         xml_free (user);
         exit (1);
      }
      user_leave (user, group);
      user_save (user);
      group_save (group);
      xml_free (user);
      xml_free (group);
   } else if (!strcmp (argv[1], "grantuser")) {
      if (argc < 5) {
         printf ("user grantuser: missing arguments.\n");
         exit (1);
      }
      user = user_get (argv[2]);
      if (!user) {
         printf ("user grantuser: Unable to load user %s.\n", argv[2]);
         exit (1);
      }
      object_grant (user, argv[3], argv[4], argv[5]);
      user_save (user);
      xml_free (user);
   } else if (!strcmp (argv[1], "grantgroup")) {
      if (argc < 5) {
         printf ("user grantgroup: missing arguments.\n");
         exit (1);
      }
      group = group_get (argv[2]);
      if (!group) {
         printf ("user grantgroup: Unable to load group %s.\n", argv[2]);
         exit (1);
      }
      object_grant (group, argv[3], argv[4], argv[5]);
      group_save (group);
      xml_free (group);
   } else if (!strcmp (argv[1], "revokeuser")) {
      if (argc < 3) {
         printf ("user revokeuser: missing arguments.\n");
         exit(1);
      }
      user = user_get (argv[2]);
      if (!user) {
         printf ("user revokeuser: Unable to load user %s.\n", argv[2]);
         exit (1);
      }
      object_revoke (user, argv[3], argv[4], argv[5]);
      user_save (user);
      xml_free (user);
   } else if (!strcmp (argv[1], "revokegroup")) {
      if (argc < 3) {
         printf ("user revokegroup: missing arguments.\n");
         exit(1);
      }
      group = group_get (argv[2]);
      if (!group) {
         printf ("user revokegroup: Unable to load group %s.\n", argv[2]);
         exit (1);
      }
      object_revoke (group, argv[3], argv[4], argv[5]);
      group_save (group);
      xml_free (group);
   } else if (!strcmp (argv[1], "include")) {
      if (argc < 5) {
         printf ("user include: missing arguments.\n");
         exit (1);
      }
      group = group_get (argv[2]);
      if (!group) {
         printf ("user include: Unable to load group %s.\n", argv[2]);
         exit (1);
      }
      user = group_get (argv[3]);
      if (!user) {
         printf ("user include: Unable to load group %s.\n", argv[2]);
         xml_free (group);
         exit (1);
      }
      group_include (group, user, argv[4]);
      group_save (group);
      group_save (user);
      xml_free (group);
      xml_free (group);
   } else if (!strcmp (argv[1], "unlink")) {
      if (argc < 4) {
         printf ("user unlink: missing arguments.\n");
         exit (1);
      }
      group = group_get (argv[2]);
      if (!group) {
         printf ("user include: Unable to load group %s.\n", argv[2]);
         exit (1);
      }
      user = group_get (argv[3]);
      if (!user) {
         printf ("user include: Unable to load group %s.\n", argv[2]);
         xml_free (group);
         exit (1);
      }
      group_unlink (group, user, argv[4]);
      group_save (group);
      group_save (user);
      xml_free (group);
      xml_free (group);
   } else if (!strcmp (argv[1], "perm")) {
      if (argc < 6) {
         printf ("user perm: missing arguments.\n");
         exit (1);
      }
      user = user_get (argv[2]);
      if (!user) {
         printf ("user perm: unable to load user %s.\n", argv[2]);
         exit (1);
      }
      if (user_perm (user, argv[3], argv[4], argv[5])) {
         printf ("Permission OK.\n");
      } else {
         printf ("No permission.\n");
      }
      xml_free (user);
   } else if (!strcmp (argv[1], "permgroup")) {
      if (argc < 5) {
         printf ("user perm: missing arguments.\n");
         exit (1);
      }
      user = user_get (argv[2]);
      if (!user) {
         printf ("user perm: unable to load user %s.\n", argv[2]);
         exit (1);
      }
      if (user_perm_group (user, argv[3], argv[4])) {
         printf ("OK\n");
      } else {
         printf ("No\n");
      }
      xml_free (user);
   } else if (!strcmp (argv[1], "list")) {
      if (argc < 5) {
         printf ("user list: missing arguments.\n");
         exit (1);
      }
      user = user_get (argv[2]);
      if (!user) {
         printf ("user perm: unable to load user %s.\n", argv[2]);
         exit (1);
      }
      group = user_list (user, argv[3], argv[4], argv[5]);
      xml_write (stdout, group);
      printf ("\n\n");
      xml_free (group);
      xml_free (user);
   } else if (!strcmp (argv[1], "groups")) {
      if (argc < 3) {
         printf ("user groups: missing username.\n");
         exit(1);
      }
      user = user_get (argv[2]);
      if (!user) {
         printf ("user groups: unable to load user %s.\n", argv[2]);
         exit (1);
      }

      /* OK, this is really ugly but it'll have to do for now. */
      holder = xml_create ("list");
      user_groups_list (user, holder, argc > 3 ? argv[3] : "view");

      elem = xml_firstelem (holder);
      while (elem) {
         printf ("%s:%s\n", xml_attrval (elem, "name"), xml_attrval (elem, "label"));
         elem = xml_nextelem (elem);
      }

      xml_free (holder);
      xml_free (user);
   } else if (!strcmp (argv[1], "users")) {
      if (argc < 3) {
         printf ("user users: missing group name.\n");
         exit(1);
      }
      group = group_get (argv[2]);
      if (!group) {
         printf ("user users: unable to load group %s.\n", argv[2]);
         exit (1);
      }

      holder = xml_create ("list");
      group_users_list (group, holder, argc > 3 ? argv[3] : "view");

      elem = xml_firstelem (holder);
      while (elem) {
         printf ("%s\n", xml_attrval (elem, "name"));
         elem = xml_nextelem (elem);
      }

      xml_free (holder);
      xml_free (user);
   } else if (!strcmp (argv[1], "userlist")) {
      if (argc < 3) {
         printf ("user userlist: missing group name.\n");
         exit(1);
      }
      group = group_get (argv[2]);
      if (!group) {
         printf ("user userlist: unable to load group %s.\n", argv[2]);
         exit (1);
      }

      holder = xml_create ("list");
      group_users_list_detailed (group, holder, argc > 3 ? argv[3] : "view");

      elem = xml_firstelem (holder);
      while (elem) {
         printf ("%s:%s:%s\n", xml_attrval (elem, "name"), xml_attrval (elem, "email"), xml_attrval (elem, "fullname"));
         elem = xml_nextelem (elem);
      }

      xml_free (holder);
      xml_free (user);
   } else {
      printf ("user: command %s not supported.\n", argv[1]);
      exit (1);
   }
   return (0);
}
