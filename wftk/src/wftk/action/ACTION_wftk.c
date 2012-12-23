#include <stdio.h>
#include <stdarg.h>
#include "../wftk.h"
#include "../wftk_internals.h"
static char *names[] = 
{
   "init",
   "free",
   "info",
   "do"
};

XML * ACTION_wftk_init (WFTK_ADAPTOR * ad, va_list args);
XML * ACTION_wftk_free (WFTK_ADAPTOR * ad, va_list args);
XML * ACTION_wftk_info (WFTK_ADAPTOR * ad, va_list args);
XML * ACTION_wftk_do (WFTK_ADAPTOR * ad, va_list args);

static WFTK_API_FUNC vtab[] = 
{
   ACTION_wftk_init,
   ACTION_wftk_free,
   ACTION_wftk_info,
   ACTION_wftk_do
};

static struct adaptor_info _ACTION_wftk_info =
{
   4,
   names,
   vtab
};
struct adaptor_info * ACTION_wftk_get_info ()
{
   return & _ACTION_wftk_info;
}
XML * ACTION_wftk_init (WFTK_ADAPTOR * ad, va_list args) {
   xml_set (ad->parms, "spec", "wftk");
   return (XML *) 0;
}
XML * ACTION_wftk_free (WFTK_ADAPTOR * ad, va_list args) { return (XML *) 0; }
XML * ACTION_wftk_info (WFTK_ADAPTOR * ad, va_list args) {
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
XML * ACTION_wftk_do  (WFTK_ADAPTOR * ad, va_list args)
{
   XML * action = (XML *) 0;
   XML * xml1;
   XML * xml2;
   const char * aname;

   if (args) action = va_arg (args, XML *);
   if (action) {
      aname = xml_attrval (action, "action");
             if (!strcmp (aname, "procstart")) {
         xml1 = wftk_process_new (ad->session, NULL, NULL);
         wftk_process_define (ad->session, xml1, NULL, xml_attrval (action, "process"));
         wftk_process_save (ad->session, xml1);
         xml2 = wftk_task_retrieve (ad->session, xml1);
         wftk_task_complete (ad->session, xml2);
         xml_free (xml2);
      } else if (!strcmp (aname, "procadhoc")) {
         xml1 = wftk_process_load (ad->session, NULL, xml_attrval (action, "process"));
         wftk_process_adhoc (ad->session, xml1, xml_copy (xml_firstelem (action)));
      } else if (!strcmp (aname, "tasknew")) {
         wftk_task_new (ad->session, xml_copy (xml_firstelem (action)));
      } else if (!strcmp (aname, "tasksubproc")) {
         /* Not doing this yet. */
      } else if (!strcmp (aname, "request")) {
         /* Wait for implementation of request. */
      } else if (!strcmp (aname, "set")) {
         xml1 = wftk_process_load (ad->session, NULL, xml_attrval (action, "process"));
         wftk_value_set (ad->session, xml1, xml_attrval (action, "name"), xml_attrval (action, "value"));
         wftk_process_save (ad->session, xml1);
      } else if (!strcmp (aname, "statusset")) {
         xml1 = wftk_process_load (ad->session, NULL, xml_attrval (action, "process"));
         wftk_status_set (ad->session, xml1, xml_attrval (action, "status"));
      } else if (!strcmp (aname, "roleassign")) {
         xml1 = wftk_process_load (ad->session, NULL, xml_attrval (action, "process"));
         wftk_role_assign (ad->session, xml1, xml_attrval (action, "role"), xml_attrval (action, "user"));
         wftk_process_save (ad->session, xml1);
      } else if (!strcmp (aname, "taskassign")) {
         /* Wait. */
      } else if (!strcmp (aname, "useradd")) {
         xml1 = wftk_process_load (ad->session, NULL, xml_attrval (action, "process"));
         wftk_user_add (ad->session, xml1, xml_copy (xml_firstelem (action)));
         wftk_process_save (ad->session, xml1);
      }
   }
   return 0;
}
