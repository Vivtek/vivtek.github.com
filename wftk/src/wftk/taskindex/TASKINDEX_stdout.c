#include <stdio.h>
#include <stdarg.h>
#include "xmlapi.h"
#include "../wftk_internals.h"
static char *names[] = 
{
   "init",
   "free",
   "info",
   "procnew",
   "procdel",
   "procget",
   "procput",
   "proclist",
   "proccomplete",
   "procerror",
   "tasknew",
   "taskdel",
   "taskget",
   "taskput",
   "tasklist",
   "taskcomplete",
   "taskreject",
   "reqnew",
   "reqdel",
   "reqget",
   "reqput",
   "reqlist",
   "reqaccept",
   "reqdecline"
};

XML * TASKINDEX_stdout_init (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_stdout_free (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_stdout_info (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_stdout_procnew (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_stdout_procdel (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_stdout_procget (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_stdout_procput (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_stdout_proclist (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_stdout_proccomplete (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_stdout_procerror (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_stdout_tasknew (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_stdout_taskdel (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_stdout_taskget (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_stdout_taskput (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_stdout_tasklist (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_stdout_taskcomplete (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_stdout_taskreject (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_stdout_reqnew (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_stdout_reqdel (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_stdout_reqget (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_stdout_reqput (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_stdout_reqlist (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_stdout_reqaccept (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_stdout_reqdecline (WFTK_ADAPTOR * ad, va_list args);

static WFTK_API_FUNC vtab[] = 
{
   TASKINDEX_stdout_init,
   TASKINDEX_stdout_free,
   TASKINDEX_stdout_info,
   TASKINDEX_stdout_procnew,
   TASKINDEX_stdout_procdel,
   TASKINDEX_stdout_procget,
   TASKINDEX_stdout_procput,
   TASKINDEX_stdout_proclist,
   TASKINDEX_stdout_proccomplete,
   TASKINDEX_stdout_procerror,
   TASKINDEX_stdout_tasknew,
   TASKINDEX_stdout_taskdel,
   TASKINDEX_stdout_taskget,
   TASKINDEX_stdout_taskput,
   TASKINDEX_stdout_tasklist,
   TASKINDEX_stdout_taskcomplete,
   TASKINDEX_stdout_taskreject,
   TASKINDEX_stdout_reqnew,
   TASKINDEX_stdout_reqdel,
   TASKINDEX_stdout_reqget,
   TASKINDEX_stdout_reqput,
   TASKINDEX_stdout_reqlist,
   TASKINDEX_stdout_reqaccept,
   TASKINDEX_stdout_reqdecline
};

static struct adaptor_info _TASKINDEX_stdout_info =
{
   24,
   names,
   vtab
};
struct adaptor_info * TASKINDEX_stdout_get_info ()
{
   return & _TASKINDEX_stdout_info;
}
XML * TASKINDEX_stdout_init (WFTK_ADAPTOR * ad, va_list args) {
   return (XML *) 0;
}
XML * TASKINDEX_stdout_free (WFTK_ADAPTOR * ad, va_list args) { return (XML *) 0; }
XML * TASKINDEX_stdout_info (WFTK_ADAPTOR * ad, va_list args) {
   XML * info;

   info = xml_create ("info");
   xml_set (info, "type", "taskindex");
   xml_set (info, "name", "stdout");
   xml_set (info, "ver", "1.0.0");
   xml_set (info, "compiled", __TIME__ " " __DATE__);
   xml_set (info, "author", "Michael Roberts");
   xml_set (info, "contact", "wftk@vivtek.com");
   xml_set (info, "extra_functions", "0");

   return (info);
}
XML * TASKINDEX_stdout_procnew (WFTK_ADAPTOR * ad, va_list args)
{
   XML * datasheet = (XML *) 0;

   if (args) datasheet = va_arg (args, XML *);
   if (!datasheet) {
      xml_set (ad->parms, "error", "No process given.");
      return (XML *) 0;
   }
   printf ("PROC new:%s:%s:%s\n", xml_attrval (datasheet, "id"),
                                  xml_attrval (datasheet, "label"),
                                  xml_attrval (datasheet, "state"));
   return (XML *) 0;
}
XML * TASKINDEX_stdout_procdel (WFTK_ADAPTOR * ad, va_list args)
{
   const char *id;

   if (args) id = va_arg (args, char *);
   if (!id) {
      xml_set (ad->parms, "error", "No process given.");
      return (XML *) 0;
   }
   printf ("PROC del:%s\n", id);
   return (XML *) 0;
}
XML * TASKINDEX_stdout_procget (WFTK_ADAPTOR * ad, va_list args) { return (XML *) 0; }
XML * TASKINDEX_stdout_procput (WFTK_ADAPTOR * ad, va_list args)
{
   XML * datasheet = (XML *) 0;

   if (args) datasheet = va_arg (args, XML *);
   if (!datasheet) {
      xml_set (ad->parms, "error", "No process given.");
      return (XML *) 0;
   }
   printf ("PROC put:%s:%s:%s\n", xml_attrval (datasheet, "id"),
                                  xml_attrval (datasheet, "label"),
                                  xml_attrval (datasheet, "state"));
   return (XML *) 0;
}
XML * TASKINDEX_stdout_proclist (WFTK_ADAPTOR * ad, va_list args) { return (XML *) 0; }
XML * TASKINDEX_stdout_proccomplete (WFTK_ADAPTOR * ad, va_list args)
{
   const char *id;

   if (args) id = va_arg (args, char *);
   if (!id) {
      xml_set (ad->parms, "error", "No process given.");
      return (XML *) 0;
   }
   printf ("PROC complete:%s\n", id);
   return (XML *) 0;
}
XML * TASKINDEX_stdout_procerror (WFTK_ADAPTOR * ad, va_list args)
{
   const char *id;

   if (args) id = va_arg (args, char *);
   if (!id) {
      xml_set (ad->parms, "error", "No process given.");
      return (XML *) 0;
   }
   printf ("PROC error:%s\n", id);
   return (XML *) 0;
}
XML * TASKINDEX_stdout_tasknew (WFTK_ADAPTOR * ad, va_list args)
{
   XML * task = (XML *) 0;

   if (args) task = va_arg (args, XML *);
   if (!task) {
      xml_set (ad->parms, "error", "No task given.");
      return (XML *) 0;
   }
   printf ("TASK new:%s:%s:%s:%s:%s\n", xml_attrval (task, "process"),
                                        xml_attrval (task, "id"),
                                        xml_attrval (task, "label"),
                                        xml_attrval (task, "role"),
                                        xml_attrval (task, "user"));
   return (XML *) 0;
}
XML * TASKINDEX_stdout_taskdel (WFTK_ADAPTOR * ad, va_list args)
{
   const char *process;
   const char *id;

   if (args) process = va_arg (args, char *);
   if (!process) {
      xml_set (ad->parms, "error", "No task given.");
      return (XML *) 0;
   }
   id = va_arg (args, char *);
   if (!id) {
      xml_set (ad->parms, "error", "No task given.");
      return (XML *) 0;
   }
   printf ("TASK del:%s:%s\n", process, id);
   return (XML *) 0;
}
XML * TASKINDEX_stdout_taskget (WFTK_ADAPTOR * ad, va_list args) { return (XML *) 0; }
XML * TASKINDEX_stdout_taskput (WFTK_ADAPTOR * ad, va_list args)
{
   XML * task = (XML *) 0;

   if (args) task = va_arg (args, XML *);
   if (!task) {
      xml_set (ad->parms, "error", "No task given.");
      return (XML *) 0;
   }
   printf ("TASK put:%s:%s:%s:%s:%s\n", xml_attrval (task, "process"),
                                        xml_attrval (task, "id"),
                                        xml_attrval (task, "label"),
                                        xml_attrval (task, "role"),
                                        xml_attrval (task, "user"));
   return (XML *) 0;
}
XML * TASKINDEX_stdout_tasklist (WFTK_ADAPTOR * ad, va_list args) { return (XML *) 0; }
XML * TASKINDEX_stdout_taskcomplete (WFTK_ADAPTOR * ad, va_list args)
{
   const char *process;
   const char *id;

   if (args) process = va_arg (args, char *);
   if (!process) {
      xml_set (ad->parms, "error", "No task given.");
      return (XML *) 0;
   }
   id = va_arg (args, char *);
   if (!id) {
      xml_set (ad->parms, "error", "No task given.");
      return (XML *) 0;
   }
   printf ("TASK complete:%s:%s\n", process, id);
   return (XML *) 0;
}
XML * TASKINDEX_stdout_taskreject (WFTK_ADAPTOR * ad, va_list args)
{
   const char *process;
   const char *id;

   if (args) process = va_arg (args, char *);
   if (!process) {
      xml_set (ad->parms, "error", "No task given.");
      return (XML *) 0;
   }
   id = va_arg (args, char *);
   if (!id) {
      xml_set (ad->parms, "error", "No task given.");
      return (XML *) 0;
   }
   printf ("TASK reject:%s:%s\n", process, id);
   return (XML *) 0;
}
XML * TASKINDEX_stdout_reqnew (WFTK_ADAPTOR * ad, va_list args)
{
   XML * task = (XML *) 0;

   if (args) task = va_arg (args, XML *);
   if (!task) {
      xml_set (ad->parms, "error", "No task given.");
      return (XML *) 0;
   }
   printf ("REQ new:%s:%s:%s:%s:%s\n", xml_attrval (task, "process"),
                                       xml_attrval (task, "id"),
                                       xml_attrval (task, "label"),
                                       xml_attrval (task, "role"),
                                       xml_attrval (task, "user"));
   return (XML *) 0;
}
XML * TASKINDEX_stdout_reqdel (WFTK_ADAPTOR * ad, va_list args)
{
   const char *process;
   const char *id;

   if (args) process = va_arg (args, char *);
   if (!process) {
      xml_set (ad->parms, "error", "No request given.");
      return (XML *) 0;
   }
   id = va_arg (args, char *);
   if (!id) {
      xml_set (ad->parms, "error", "No request given.");
      return (XML *) 0;
   }
   printf ("REQ del:%s:%s\n", process, id);
   return (XML *) 0;
}
XML * TASKINDEX_stdout_reqget (WFTK_ADAPTOR * ad, va_list args) { return (XML *) 0; }
XML * TASKINDEX_stdout_reqput (WFTK_ADAPTOR * ad, va_list args)
{
   XML * task = (XML *) 0;

   if (args) task = va_arg (args, XML *);
   if (!task) {
      xml_set (ad->parms, "error", "No request given.");
      return (XML *) 0;
   }
   printf ("REQ put:%s:%s:%s:%s:%s\n", xml_attrval (task, "process"),
                                       xml_attrval (task, "id"),
                                       xml_attrval (task, "label"),
                                       xml_attrval (task, "role"),
                                       xml_attrval (task, "user"));
   return (XML *) 0;
}
XML * TASKINDEX_stdout_reqlist (WFTK_ADAPTOR * ad, va_list args) { return (XML *) 0; }
XML * TASKINDEX_stdout_reqaccept (WFTK_ADAPTOR * ad, va_list args)
{
   const char *process;
   const char *id;

   if (args) process = va_arg (args, char *);
   if (!process) {
      xml_set (ad->parms, "error", "No request given.");
      return (XML *) 0;
   }
   id = va_arg (args, char *);
   if (!id) {
      xml_set (ad->parms, "error", "No request given.");
      return (XML *) 0;
   }
   printf ("REQ accept:%s:%s\n", process, id);
   return (XML *) 0;
}
XML * TASKINDEX_stdout_reqdecline (WFTK_ADAPTOR * ad, va_list args)
{
   const char *process;
   const char *id;

   if (args) process = va_arg (args, char *);
   if (!process) {
      xml_set (ad->parms, "error", "No request given.");
      return (XML *) 0;
   }
   id = va_arg (args, char *);
   if (!id) {
      xml_set (ad->parms, "error", "No request given.");
      return (XML *) 0;
   }
   printf ("REQ decline:%s:%s\n", process, id);
   return (XML *) 0;
}
