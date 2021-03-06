#ifndef __USER_H_
#define __USER_H_
#include <stdio.h>
#include <stdlib.h>
#include "../xmlapi.h"
XML * user_authenticate (XML * cgi_environment, const char * realm);
int user_exists (const char *username);
XML * user_get (const char * username);
int user_save (XML * user);
XML * user_auth_get (const char * username, XML * current_user);
int user_auth_save (XML * user, XML * current_user);
int group_exists (const char *groupname);
XML * group_get (const char * groupname);
int group_save (XML * group);
XML * group_auth_get (const char * groupname, XML * current_user);
int group_auth_save (XML * group, XML * current_user);
int user_join (XML * user, XML * group);
int user_leave (XML * user, XML * group);
int object_grant (XML * user_or_group, const char * class, const char * object, const char * permission);
int object_revoke (XML * user_or_group, const char * class, const char * object, const char * permission);
int group_include (XML * outgroup, XML * ingroup, const char * permission);
int group_unlink (XML * outgroup, XML * ingroup, const char * permission);
int user_perm (XML * user, const char * class, const char * object, const char * permission);
int user_perm_group (XML * user, const char * groupname, const char * permission);
XML * user_list (XML * user, const char * class, const char * object, const char * permission);
void user_expand (XML * list_or_group);
void user_expandall (XML * list_or_group);
void user_groups_list (XML * user, XML * holder, const char * permission);
void group_users_list (XML * group, XML * holder, const char * permission);
void group_users_list_detailed (XML * group, XML * holder, const char * permission);
#endif
