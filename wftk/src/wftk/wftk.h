#ifndef WFTK_H
#define WFTK_H
#include <stdio.h>
#include "../xmlapi/xmlapi.h"

XML  * wftk_info ();

void * wftk_session_alloc      ();
void   wftk_session_free       (void * session);
void   wftk_session_current    (void * session, XML * object);
void   wftk_session_configure  (void * session, XML * config);
void   wftk_session_setuser    (void * session, char * userid);
XML  * wftk_session_getuser    (void * session);
XML  * wftk_session_stashvalue (void * session, const char * value);
void   wftk_session_freevalue  (void * session, const char * value);
XML  * wftk_session_cache      (void * session, XML * key, int * found);
XML  * wftk_session_cachecheck (void * session, XML * key);

XML  * wftk_process_new     (void * session, const char * dsrep, const char * datasheet_id);
XML  * wftk_process_load    (void * session, const char * dsrep, const char * datasheet_id);
int    wftk_process_save    (void * session, XML * datasheet);
int    wftk_process_define  (void * session, XML * datasheet, const char * pdrep, const char * procdef_id);
int    wftk_process_archive (void * session, const char * dsrep, const char * datasheet_id, const char * archive);
int    wftk_process_delete  (void * session, const char * dsrep, const char * datasheet_id);
int    wftk_process_adhoc   (void * session, XML * datasheet, XML * arbitraryworkflow);

int    wftk_task_list      (void * session, XML * list);
int    wftk_task_new       (void * session, XML * task);
XML  * wftk_task_retrieve  (void * session, XML * task);
int    wftk_task_update    (void * session, XML * task);
int    wftk_task_complete  (void * session, XML * task);
int    wftk_task_reject    (void * session, XML * task);
int    wftk_task_rescind   (void * session, XML * task);
int    wftk_task_subproc   (void * session, XML * task, XML * subproc_datasheet);
int    wftk_task_attach    (void * session, XML * task, XML * datasheet);

int    wftk_request_list     (void * session, XML * list);
int    wftk_request_new      (void * session, XML * request);
XML  * wftk_request_retrieve (void * session, XML * request);
int    wftk_request_update   (void * session, XML * request);
int    wftk_request_rescind  (void * session, XML * request);
int    wftk_request_accept   (void * session, XML * request);
int    wftk_request_decline  (void * session, XML * request);

int    wftk_value_list      (void * session, XML * datasheet, XML * list);
XML  * wftk_value_find      (void * session, XML * datasheet, const char * name);
XML  * wftk_value_make      (void * session, XML * datasheet, const char * name);
int    wftk_value_isnull    (void * session, XML * datasheet, const char * name);
const char * wftk_value_get (void * session, XML * datasheet, const char * name);
int    wftk_value_get_num   (void * session, XML * datasheet, const char * name);
int    wftk_value_interpret (void * session, XML * datasheet, const char * spec, char * buffer, int bufsize);
int    wftk_value_makenull  (void * session, XML * datasheet, const char * name);
int    wftk_value_set       (void * session, XML * datasheet, const char * name, const char * value);
int    wftk_value_set_num   (void * session, XML * datasheet, const char * name, int value);
int    wftk_value_settype   (void * session, XML * datasheet, const char * name, const char * type);
int    wftk_value_define    (void * session, XML * datasheet, const char * name, const char * storage);
int    wftk_value_calc      (void * session, XML * datasheet, const char * name, const char * value);
XML  * wftk_value_html      (void * session, XML * datasheet, const char * name);
XML  * wftk_value_htmlblank (void * session, XML * datasheet, const char * name);
XML  * wftk_value_info      (void * session, XML * datasheet, const char * name);

const char * wftk_status_get (void * session, XML * datasheet);
int    wftk_status_set (void * session, XML * datasheet, const char *status);

int    wftk_role_list       (void * session, XML * datasheet, XML * list);
const char * wftk_role_user (void * session, XML * datasheet, const char * role);
int    wftk_role_assign     (void * session, XML * datasheet, const char * role, const char * userid);

int    wftk_user_list     (void * session, XML * datasheet, XML * list);
int    wftk_user_add      (void * session, XML * datasheet, XML * user);
XML  * wftk_user_retrieve (void * session, XML * datasheet, const char * userid);
int    wftk_user_update   (void * session, XML * datasheet, XML * user);
int    wftk_user_remove   (void * session, XML * datasheet, const char * userid);
int    wftk_user_synch    (void * session, XML * user);
int    wftk_user_auth     (void * session, XML * user, const char * password);

XML  * wftk_enactment       (void * session, XML * datasheet);
int    wftk_enactment_write (void * session, XML * datasheet, XML * xml, const char * attribute, const char * value);
int    wftk_log             (void * session, XML * datasheet, char * log);

int    wftk_action          (void * session, XML * action);

XML  * wftk_decide          (void * session, XML * datasheet, XML * decision);

int    wftk_notify (void * session, XML * context, XML * alert);
#endif
