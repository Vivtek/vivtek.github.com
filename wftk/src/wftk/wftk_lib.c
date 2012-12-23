#include "wftk.h"
#include "time.h"
#include "wftk_internals.h"

struct _state {
   XML * datasheet;
   XML * state;
   XML * queue;
   XML * procdef;
};

int unique_id (XML * datasheet, XML * state) {
   int idcount;

   if (!state) {
      state = xml_loc (datasheet, "datasheet.state");
      if (!state) {
         state = xml_create ("state");
         xml_append (datasheet, state);
      }
   }

   idcount = atoi (xml_attrval (state, "idcount"));
   idcount++;
   xml_setnum (state, "idcount", idcount);

   return idcount;
}

XML * queue_procdef (void * session, XML * datasheet, XML * state, XML * queue, XML * action, const char * where)
{
   XML * item;
   int  idcount;
   char action_loc[1024]; /* TODO: yadda, yadda. */

   if (action == NULL) return NULL;

   idcount = unique_id (datasheet, state);

   item = xml_create("item");
   xml_setnum (item, "id", idcount);
   xml_set (item, "type", action->name);
   xml_getloc (action, action_loc, sizeof(action_loc) - 1);
   xml_set (item, "loc", action_loc);
   xml_set (item, "where", where);
   xml_set (item, "oncomplete", xml_attrval (action, "oncomplete"));
   xml_set (item, "fortask", xml_attrval (action, "fortask"));
   xml_append (queue, item);
   return (item);
}
void process_procdef(void * session, XML * datasheet, XML * state, XML * queue, XML * procdef)
{
   XML * item;
   XML * def;
   XML * holder;
   XML * mark;
   XML * task;
   XML * data;
   XML * next;
   const char * type;
   int count;
   int keep;
   char sbuf[1024];
   WFTK_ADAPTORLIST * adlist;

   item = xml_firstelem (queue);

   while (item != NULL) {
      if (!strcmp("yes", xml_attrval(item, "block"))) {
         item = xml_nextelem(item);
         continue;
      }
      if (!strcmp (xml_attrval (item, "where"), "datasheet")) {
         def = xml_loc (datasheet, xml_attrval(item, "loc"));
      } else {
         def = xml_loc (procdef, xml_attrval(item, "loc"));
      }
      type = xml_attrval (item, "type");

      keep = 0;
      next = NULL;
             if (!strcmp (type, "parallel")) {
if (!strcmp ("", xml_attrval (item, "remaining"))) {
   count = 0;
   next = xml_firstelem (def);
   while (next != NULL) {
      count ++;
      xml_set (queue_procdef (session, datasheet, state, queue, next, xml_attrval (item, "where")), "parent", xml_attrval (item, "id"));
      next = xml_nextelem (next);
   }
} else {
   count = xml_attrvalnum (item, "remaining");
   count--;
}
xml_setnum (item, "remaining", count);
if (count > 0) keep = 1;
      } else if (!strcmp (type, "task")) {
if (strcmp (xml_attrval (item, "block"), "resume")) {
   /* Notify task indices of the new active task. */
   task = xml_create ("task");
   xml_set (task, "id", xml_attrval (item, "id"));
   wftk_value_interpret (session, datasheet, xml_attrval (def, "label"), sbuf, sizeof(sbuf));
   xml_set (task, "label", sbuf);
   xml_set (task, "role", xml_attrval (def, "role"));
   xml_set (task, "user", xml_attrval (def, "user"));
   if (!*xml_attrval (task, "user") && *xml_attrval (task, "role")) {
      xml_set (task, "user", wftk_role_user (session, datasheet, xml_attrval (task, "role")));
   }
   xml_set (task, "process", xml_attrval (datasheet, "id"));

   mark = xml_firstelem (def);
   while (mark) {
      if (!strcmp (mark->name, "data")) {
         if (!wftk_value_find (session, datasheet, xml_attrval (mark, "id"))) {
            xml_append (datasheet, xml_copy (mark));
         }
      }
      mark = xml_nextelem (mark);
   }

   adlist = wftk_get_adaptorlist (session, TASKINDEX);
   wftk_call_adaptorlist (adlist, "tasknew", task);
   wftk_free_adaptorlist (session, adlist);
   xml_free (task);

   keep = 1; /* This blocks the current item, so that the active task will be available for retrieval. */
}
      } else if (!strcmp (type, "action")) {
/* Notify task indices of the new active task. */
if (!strcmp (xml_attrval (item, "where"), "datasheet")) {
   mark = xml_loc (datasheet, xml_attrval (item, "loc"));
} else {
   mark = xml_loc (procdef, xml_attrval (item, "loc"));
}
if (mark) {
   holder = xml_copy (mark);
   if (wftk_session_getuser (session)) {
      xml_set (holder, "by", xml_attrval (wftk_session_getuser(session), "id"));
   }
   xml_set (holder, "dsrep", xml_attrval (datasheet, "dsrep"));
   xml_set (holder, "process", xml_attrval (datasheet, "id"));
   xml_set (holder, "pdrep", xml_attrval (procdef, "pdrep"));
   xml_set (holder, "procdef", xml_attrval (procdef, "id"));

   wftk_action (session, holder);
}
      } else if (!strcmp (type, "data")) {
if (!strcmp (xml_attrval (item, "where"), "datasheet")) {
   mark = xml_loc (datasheet, xml_attrval (item, "loc"));
} else {
   mark = xml_loc (procdef, xml_attrval (item, "loc"));
}
if (mark) {
   wftk_value_interpret (session, datasheet, xml_attrval (mark, "value"), sbuf, sizeof (sbuf));
   wftk_value_set (session, datasheet, xml_attrval (mark, "id"), sbuf);
}
      } else if (!strcmp (type, "situation")) {
      } else if (!strcmp (type, "decide")) {
if (!strcmp ("", xml_attrval (item, "cur"))) {
   if (!strcmp (xml_attrval (item, "where"), "datasheet")) {
      mark = xml_loc (datasheet, xml_attrval (item, "loc"));
   } else {
      mark = xml_loc (procdef, xml_attrval (item, "loc"));
   }
   if (mark) {
      holder = wftk_decide (session, datasheet, mark);
      if (*xml_attrval (holder, "loc")) {
         if (!strcmp (xml_attrval (item, "where"), "datasheet")) {
            next = xml_loc (datasheet, xml_attrval (holder, "loc"));
         } else {
            next = xml_loc (procdef, xml_attrval (holder, "loc"));
         }
      }
      xml_free (holder);
   }
}
if (next) {
   xml_set (queue_procdef (session, datasheet, state, queue, next, xml_attrval (item, "where")), "parent", xml_attrval (item, "parent"));
   keep = 1;
}
      } else if (!strcmp (type, "alert")) {
wftk_notify (session, datasheet, def);
      } else if (!strcmp (type, "start")) {
      } else { /* Treat it as a sequence. */
if (!strcmp ("", xml_attrval (item, "cur"))) {
   next = xml_firstelem (def);
} else {
   if (!strcmp (xml_attrval (item, "where"), "datasheet")) {
      next = xml_loc (datasheet, xml_attrval (item, "cur"));
   } else {
      next = xml_loc (procdef, xml_attrval (item, "cur"));
   }
   next = xml_nextelem (next);
}

if (next) {
   xml_set (queue_procdef (session, datasheet, state, queue, next, xml_attrval (item, "where")), "parent", xml_attrval (item, "id"));
   xml_getloc (next, sbuf, sizeof(sbuf) - 1);
   xml_set (item, "cur", sbuf);
   keep = 1;
}
      }

      if (keep) {
         xml_set (item, "block", "yes");
         item = xml_nextelem(item);
      } else {
         if (*xml_attrval (item, "oncomplete")) {
            wftk_status_set (session, datasheet, xml_attrval (item, "oncomplete"));
         }
         if (*xml_attrval (item, "parent")) {
            sprintf (sbuf, "queue.item[%d]", xml_attrvalnum (item, "parent"));
            next = xml_loc (queue, sbuf);
            xml_delete (item);
            item = next;
            xml_set (item, "block", "no");
         } else {
            if (*xml_attrval (item, "fortask")) {
               sprintf (sbuf, "datasheet.state.queue.item[%s]", xml_attrval (item, "fortask"));
               holder = xml_loc (datasheet, sbuf);
               if (holder) { /* A workflow task; we'll just handle that case right in the interpreter. */
                  xml_delete (item);
                  item = holder;
               } else { /* Maybe it's an ad-hoc task or something; wftk_task_complete can handle it. */
                  holder = xml_create ("task");
                  xml_set (holder, "id", xml_attrval (item, "fortask"));
                  xml_delete (item);
                  item = NULL;
                  xml_set (holder, "dsrep", xml_attrval (datasheet, "repository"));
                  xml_set (holder, "process", xml_attrval (datasheet, "id"));
                  wftk_task_complete (session, holder);
                  xml_delete (holder);
               }
            } else {
               xml_delete (item);
               item = NULL;
            }
         }
      }
   }
}
XML * wftk_process_new (void * session, const char * dsrep, const char * datasheet_id)
{
   WFTK_ADAPTOR * ad;
   WFTK_ADAPTORLIST * adlist;
   XML * datasheet;

   ad = wftk_get_adaptor (session, DSREP, dsrep);
   if (!ad) return (XML *) 0;

   datasheet = wftk_call_adaptor (ad, "new", datasheet_id);
   if (!datasheet) {
      wftk_free_adaptor (session, ad);
      return (XML *) 0;
   }

   xml_set (datasheet, "repository", xml_attrval (ad->parms, "spec"));
   /* ID must be set by adaptor because the adaptor may have selected it. */

   /* Notify task indices. */
   if (*xml_attrval (datasheet, "id")) {
      adlist = wftk_get_adaptorlist (session, TASKINDEX);
      wftk_call_adaptorlist (adlist, "procnew", datasheet);
      wftk_free_adaptorlist (session, adlist);
   }

   wftk_session_cache (session, datasheet, NULL);
   wftk_free_adaptor (session, ad);
   return datasheet;
}
XML * wftk_process_load (void * session, const char * dsrep, const char * datasheet_id)
{
   WFTK_ADAPTOR * ad;
   XML * key;
   XML * datasheet;

   ad = wftk_get_adaptor (session, DSREP, dsrep);
   if (!ad) return (XML *) 0;

   key = xml_create ("datasheet");
   xml_set (key, "repository", xml_attrval (ad->parms, "spec"));
   xml_set (key, "id", datasheet_id);
   datasheet = wftk_session_cachecheck (session, key);
   xml_free (key);

   if (datasheet) {
      wftk_free_adaptor (session, ad);
      return (datasheet);
   }

   datasheet = wftk_call_adaptor (ad, "load", datasheet_id);
   if (!datasheet) {
      wftk_free_adaptor (session, ad);
      return (XML *) 0;
   }

   xml_set (datasheet, "repository", xml_attrval (ad->parms, "spec"));

   wftk_session_cache (session, datasheet, NULL);
   wftk_free_adaptor (session, ad);
   return datasheet;
}
int wftk_process_save (void * session, XML * datasheet)
{
   WFTK_ADAPTOR * ad;
   WFTK_ADAPTORLIST * adlist;
   XML * ret;

   ad = wftk_get_adaptor (session, DSREP, xml_attrval (datasheet, "repository"));
   if (!ad) return 0;

   ret = wftk_call_adaptor (ad, "save", datasheet);
   if (!ret) {
      wftk_free_adaptor (session, ad);
      return 0;
   }

   /* Notify task indices. */
   adlist = wftk_get_adaptorlist (session, TASKINDEX);
   wftk_call_adaptorlist (adlist, "procput", datasheet);
   wftk_free_adaptorlist (session, adlist);

   wftk_free_adaptor (session, ad);
   return 1;
}
int wftk_process_delete (void * session, const char * dsrep, const char * datasheet_id)
{
   WFTK_ADAPTOR * ad;
   WFTK_ADAPTORLIST * adlist;
   XML * ret;

   ad = wftk_get_adaptor (session, DSREP, dsrep);
   if (!ad) return 0;

   ret = wftk_call_adaptor (ad, "delete", datasheet_id);

   /* Notify task indices. */
   adlist = wftk_get_adaptorlist (session, TASKINDEX);
   wftk_call_adaptorlist (adlist, "procdel", datasheet_id);
   wftk_free_adaptorlist (session, adlist);

   if (ret) xml_free (ret);
   wftk_free_adaptor (session, ad);
   return 1;
}
int wftk_process_define (void * session, XML * datasheet, const char * pdrep, const char * procdef_id)
{
   WFTK_ADAPTOR * ad;
   XML * version;
   XML * procdef;
   XML * mark;

   ad = wftk_get_adaptor (session, PDREP, pdrep);
   if (!ad) return 0;

   version = wftk_call_adaptor (ad, "version", procdef_id);
   if (!version) {
      wftk_free_adaptor (session, ad);
      return 0;
   }

   xml_set (datasheet, "pdrep", xml_attrval (ad->parms, "spec"));
   xml_set (datasheet, "procdef", procdef_id);
   xml_set (datasheet, "version", xml_attrval (version, "value"));

   xml_free (version);
   wftk_free_adaptor (session, ad);

   procdef = _procdef_load (session, datasheet);
   if (procdef) {
      mark = xml_firstelem (procdef);
      while (mark) {
         if (!strcmp (mark->name, "role")) {
            wftk_role_assign (session, datasheet, xml_attrval (mark, "name"), xml_attrval (mark, "localuser"));
         } else if (!strcmp (mark->name, "data")) {
            if (!wftk_value_find (session, datasheet, xml_attrval (mark, "id"))) {
               xml_append (datasheet, xml_copy (mark));
            }
         }

         mark = xml_nextelem (mark);
      }
      if (!session) xml_free (procdef);   
   }
   
   return 1;
}
XML * _procdef_load (void * session, XML * datasheet)
{
   WFTK_ADAPTOR * ad;
   XML * key;
   XML * ret;

   if (!datasheet) return NULL;
   if (!*xml_attrval (datasheet, "procdef")) return NULL;

   key = xml_create ("workflow");
   xml_set (key, "repository", xml_attrval (datasheet, "pdrep"));
   xml_set (key, "id", xml_attrval (datasheet, "procdef"));
   xml_set (key, "version", xml_attrval (datasheet, "version"));
   ret = wftk_session_cachecheck (session, key);
   xml_free (key);

   if (ret) { return (ret); }

   ad = wftk_get_adaptor (session, PDREP, xml_attrval (datasheet, "pdrep"));
   if (!ad) return NULL;

   ret = wftk_call_adaptor (ad, "load", xml_attrval (datasheet, "procdef"), xml_attrval (datasheet, "version"));
   xml_set (ret, "repository", xml_attrval (ad->parms, "spec"));
   xml_set (ret, "id", xml_attrval (datasheet, "procdef"));
   xml_set (ret, "version", xml_attrval (datasheet, "version"));

   wftk_session_cache (session, ret, NULL);
   wftk_free_adaptor (session, ad);
   return (ret);
}
int wftk_process_archive (void * session, const char * dsrep, const char * datasheet_id, const char * archive)
{
   WFTK_ADAPTOR * ad;
   XML * ret;

   ad = wftk_get_adaptor (session, DSREP, dsrep);
   if (!ad) return 0;

   ret = wftk_call_adaptor (ad, "archive", datasheet_id, archive);
   if (!ret) {
      wftk_free_adaptor (session, ad);
      return 0;
   }

   xml_free (ret);
   wftk_free_adaptor (session, ad);
   return 1;
}
int wftk_process_adhoc (void * session, XML * datasheet, XML * arbitrary_workflow)
{
   XML * state;
   XML * queue;
   XML * procdef = _procdef_load (session, datasheet); /* Necessary for running the queue, just in case something unblocks. */
   XML * adhoc = xml_create ("adhoc");

   state = xml_loc (datasheet, "datasheet.state");
   if (!state) {
      state = xml_create ("state");
      xml_append (datasheet, state);
   }
   queue = xml_loc (state, "state.queue");
   if (!queue) {
      queue = xml_create ("queue");
      xml_append (state, queue);
   }

   xml_append (adhoc, arbitrary_workflow);
   xml_append (datasheet, adhoc);

   queue_procdef (session, datasheet, state, queue, arbitrary_workflow, "datasheet");
   process_procdef (session, datasheet, state, queue, procdef);
   wftk_process_save (session, datasheet);

   return 1;
}
XML * wftk_value_find (void * session, XML * datasheet, const char * name)
{
   char valuelocbuf[128];

   if (!datasheet) return NULL;
   sprintf (valuelocbuf, "datasheet.data[%s]", name);
   return xml_loc (datasheet, valuelocbuf);
} 
XML * wftk_value_make (void * session, XML * datasheet, const char * name)
{
   XML * data;
   char valuelocbuf[128];

   if (!datasheet) return NULL;
   sprintf (valuelocbuf, "datasheet.data[%s]", name);
   data = xml_loc (datasheet, valuelocbuf);
   if (!data) {
      data = xml_create ("data");
      xml_set (data, "id", name);
      xml_append (datasheet, data);
   }

   return data;
} 
const char * wftk_value_get     (void * session, XML * datasheet, const char * name)
{
   XML * data;
   WFTK_ADAPTOR * ad;

   if (!name) return "";
   if (*name == '!') return _wftk_value_special (session, datasheet, name);

   if (strchr (name, ':')) {
      ad = wftk_get_adaptor (session, DATASTORE, name);
      if (!ad) return "";
      data = wftk_call_adaptor (ad, "get", datasheet, name);
      wftk_free_adaptor (session, ad);
      return (xml_attrval (data, "value"));
   }

   data = wftk_value_find (session, datasheet, name);
   if (!data) return "";

   if (!strcmp (xml_attrval (data, "null"), "yes")) return "";

   return (xml_attrval (data, "value"));
}
int    wftk_value_get_num (void * session, XML * datasheet, const char * name)
{
   int ret;
   XML * data;
   WFTK_ADAPTOR * ad;

   if (!name) return 0;
   if (*name == '!') return atoi (_wftk_value_special (session, datasheet, name));

   if (strchr (name, ':')) {
      ad = wftk_get_adaptor (session, DATASTORE, name);
      if (!ad) return 0;
      data = wftk_call_adaptor (ad, "get", datasheet, name);
      wftk_free_adaptor (session, ad);
      ret = atoi (xml_attrval (data, "value"));
      xml_delete (data);
      return ret;
   }

   data = wftk_value_find (session, datasheet, name);
   if (!data) return 0;

   if (!strcmp (xml_attrval (data, "null"), "yes")) return 0;

   return (atoi (xml_attrval (data, "value")));  /* TODO: types and stores. */
}

int wftk_value_isnull (void * session, XML * datasheet, const char * name)
{
   XML * data;
   WFTK_ADAPTOR * ad;

   if (!name) return 1; /* Null name is, well, null.  Logical, right? */
   if (*name == '!') return 0; /* Special value is never null.  Why?  Because I say so. */

   data = wftk_value_find (session, datasheet, name);
   if (!data) return 1;

   if (!strcmp (xml_attrval (data, "null"), "yes")) return 1;
   return 0;
}

int wftk_value_set (void * session, XML * datasheet, const char * name, const char * value)
{
   XML * data;
   WFTK_ADAPTOR * ad;

   if (!name) return 0;
   if (*name == '!') return 0;  /* Refuse to set special values. */

   if (strchr (name, ':')) {
      ad = wftk_get_adaptor (session, DATASTORE, name);
      if (!ad) return 0;
      wftk_call_adaptor (ad, "set", datasheet, name, value);
      wftk_free_adaptor (session, ad);
      return 1;
   }

   data = wftk_value_make (session, datasheet, name);
   if (!data) return 0;

   if (strcmp (wftk_value_get (session, datasheet, name), value)) {
      wftk_enactment_write (session, datasheet, data, "was", wftk_value_get (session, datasheet, name));
      wftk_process_save (session, datasheet);
      xml_set (data, "value", value);
   }
   if (!strcmp (xml_attrval (data, "null"), "yes")) xml_set (data, "null", "");
   return 1;
}
int    wftk_value_set_num (void * session, XML * datasheet, const char * name, int value)
{
   XML * data;
   WFTK_ADAPTOR * ad;
   char valbuf[sizeof(int) * 3 + 1];

   if (!name) return 0;
   if (*name == '!') return 0;

   if (strchr (name, ':')) {
      ad = wftk_get_adaptor (session, DATASTORE, name);
      if (!ad) return 0;
      sprintf (valbuf, "%d", value);
      wftk_call_adaptor (ad, "set", datasheet, name, valbuf);
      wftk_free_adaptor (session, ad);
      return 1;
   }

   data = wftk_value_make (session, datasheet, name);
   if (!data) return 0;

   sprintf (valbuf, "%d", value);
   if (strcmp (wftk_value_get (session, datasheet, name), valbuf)) {
      wftk_enactment_write (session, datasheet, data, "was", wftk_value_get (session, datasheet, name));
      wftk_process_save (session, datasheet);
      xml_setnum (data, "value", value);
   }
   if (!strcmp (xml_attrval (data, "null"), "yes")) xml_set (data, "null", "");
   return 1;
} 

int wftk_value_makenull (void * session, XML * datasheet, const char * name)
{
   XML * data;
   WFTK_ADAPTOR * ad;

   if (!name) return 0;
   if (*name == '!') return 0; /* Refuse to set special values. */

   if (strchr (name, ':')) {
      ad = wftk_get_adaptor (session, DATASTORE, name);
      if (!ad) return 0;
      wftk_call_adaptor (ad, "makenull", datasheet, name);
      wftk_free_adaptor (session, ad);
      return 1;
   }

   data = wftk_value_make (session, datasheet, name);
   if (!data) return 0;
   wftk_enactment_write (session, datasheet, data, "was", wftk_value_get (session, datasheet, name));
   wftk_process_save (session, datasheet);
   xml_set (data, "null", "yes");
   return 1;
}   
int    wftk_value_interpret (void * session, XML * datasheet, const char * spec, char * buffer, int bufsize)
{
   int count = 0;
   const char *value;
   char namebuf[256]; /* TODO: replace this. */
   int i;

   bufsize--; /* Leave room for the null. */
   while (*spec) {
      if (spec[0] == '$' && spec[1] == '{') {
         i = 0;
         spec += 2;
         while (*spec && *spec != '}' && i < sizeof(namebuf) - 1) {
            namebuf[i++] = *spec++;
         }
         if (*spec == '}') spec++;
         namebuf[i] = '\0';
         value = wftk_value_get (session, datasheet, namebuf);
         while (*value && bufsize) {
            *buffer++ = *value++;
            count++;
            bufsize--;
         }
      } else {
         *buffer++ = *spec++;
         count++;
         bufsize--;
      }
      if (!bufsize) break;
   }
   *buffer = '\0';
   return count;
}
const char * _wftk_value_special (void * session, XML * datasheet, const char * name)
{
   struct tm * timeptr;
   time_t julian;
   static char value[64]; /* Boy, this is dangerous.  TODO: there's gotta be a better way. */

   if (!strcmp (name, "!now")) {
      time (&julian);
      timeptr = localtime (&julian);
      sprintf (value, "%04d-%02d-%02d %02d:%02d:%02d", timeptr->tm_year + 1900, timeptr->tm_mon + 1, timeptr->tm_mday,
                                                       timeptr->tm_hour, timeptr->tm_min, timeptr->tm_sec);
      return (value);
   }

   return ("");
}
int    wftk_value_settype (void * session, XML * datasheet, const char * name, const char * type)
{
   XML * data;
   WFTK_ADAPTOR * ad;

   if (!name) return 0;
   if (*name == '!') return 0;  /* Refuse to set special values. */

   if (strchr (name, ':')) return 0; /* Can't set types for alternate stores -- the answer will be aliases, later. */

   data = wftk_value_make (session, datasheet, name);
   if (!data) return 0;

   xml_set (data, "type", type);
   return 1;
}
int    wftk_value_define  (void * session, XML * datasheet, const char * name, const char * storage) { return 0; } /* TODO: data aliases. */
int    wftk_value_calc    (void * session, XML * datasheet, const char * name, const char * value) { return 0; }
XML  * wftk_value_html      (void * session, XML * datasheet, const char * name)
{
   XML * field;
   XML * data;
   WFTK_ADAPTOR * ad;

   if (!name) return 0;
   if (*name == '!') return (xml_createtext (_wftk_value_special (session, datasheet, name)));

   if (strchr (name, ':')) {
      ad = wftk_get_adaptor (session, DATASTORE, name);
      if (!ad) return 0;
      data = wftk_call_adaptor (ad, "get", datasheet, name);
      wftk_free_adaptor (session, ad);
      field = xml_create ("input");
      xml_set (field, "name", name);
      xml_set (field, "value", xml_attrval (data, "value"));
      xml_delete (data);
      return (field);
   }

   data = wftk_value_find (session, datasheet, name);
   if (!data) {
      field = xml_create ("input");
      xml_set (field, "name", name);
      xml_set (field, "value", "");
      return (field);
   }

   if (*xml_attrval (data, "type")) {
      ad = wftk_get_adaptor (session, DATATYPE, xml_attrval (data, "type"));
      if (ad) {
         field = wftk_call_adaptor (ad, "html", datasheet, data);
         wftk_free_adaptor (session, ad);
         return (field);
      }
   }

   field = xml_create ("input");
   xml_set (field, "name", name);
   if (*xml_attrval (data, "size")) {
      xml_set (field, "size", xml_attrval (data, "size"));
   }
   if (!strcmp (xml_attrval (data, "null"), "yes")) {
      xml_set (field, "value", "");
   } else {
      xml_set (field, "value", wftk_value_get (session, datasheet, name));
   }

   return (field);
}

XML  * wftk_value_htmlblank (void * session, XML * datasheet, const char * name)
{
   XML * field;
   XML * data;
   WFTK_ADAPTOR * ad;

   if (!name) return 0;
   if (*name == '!') return (xml_createtext (_wftk_value_special (session, datasheet, name)));

   if (strchr (name, ':')) {
      field = xml_create ("input");
      xml_set (field, "name", name);
      xml_set (field, "value", "");
      return (field);
   }

   data = wftk_value_find (session, datasheet, name);
   if (!data) {
      field = xml_create ("input");
      xml_set (field, "name", name);
      xml_set (field, "value", "");
      return (field);
   }

   if (*xml_attrval (data, "type")) {
      ad = wftk_get_adaptor (session, DATATYPE, xml_attrval (data, "type"));
      if (ad) {
         field = wftk_call_adaptor (ad, "htmlblank", datasheet, data);
         wftk_free_adaptor (session, ad);
         return (field);
      }
   }

   field = xml_create ("input");
   xml_set (field, "name", name);
   if (*xml_attrval (data, "size")) {
      xml_set (field, "size", xml_attrval (data, "size"));
   }
   xml_set (field, "value", "");

   return (field);
}
int wftk_value_list    (void * session, XML * datasheet, XML * list) {
   int counter = 0;
   XML * pointer = xml_firstelem (datasheet);
   XML * value;

   if (!list) return 0;

   while (pointer) {
      if (!strcmp (pointer->name, "data")) {
         counter++;
         value = xml_create ("data");
         xml_set (value, "id", xml_attrval (pointer, "id"));
         xml_set (value, "value", wftk_value_get (session, datasheet, xml_attrval (pointer, "id"))); /* OK, not the most efficient method..*/
         xml_set (value, "type", xml_attrval (pointer, "type"));
         xml_append (list, value);
      }
      pointer = xml_nextelem (pointer);
   }

   xml_setnum (list, "count", counter);
   return counter;
}

XML  * wftk_value_info    (void * session, XML * datasheet, const char * name) { return 0; }
const char * wftk_status_get (void * session, XML * datasheet) {
   const char * status;
   if (!datasheet) return "none";
   status = xml_attrval (datasheet, "status");
   if (*status) return status;
   return "start";
}
int wftk_status_set (void * session, XML * datasheet, const char * status) {
   const char * cur_status;
   WFTK_ADAPTORLIST * adlist;

   if (!datasheet) return 0;

   cur_status = wftk_status_get (session, datasheet);
   if (!strcmp (cur_status, status)) return 1;

   xml_set (datasheet, "status", status);

   /* Notify task indices. */
   if (!strcmp (status, "complete")) {
      adlist = wftk_get_adaptorlist (session, TASKINDEX);
      wftk_call_adaptorlist (adlist, "proccomplete", xml_attrval (datasheet, "id"));
      wftk_free_adaptorlist (session, adlist);
   } else if (!strcmp (status, "error")) {
      adlist = wftk_get_adaptorlist (session, TASKINDEX);
      wftk_call_adaptorlist (adlist, "procerror", xml_attrval (datasheet, "id"));
      wftk_free_adaptorlist (session, adlist);
   } else {
      adlist = wftk_get_adaptorlist (session, TASKINDEX);
      wftk_call_adaptorlist (adlist, "procput", datasheet);
      wftk_free_adaptorlist (session, adlist);
   }

   if (!strcmp (status, "complete")) {
      /* TODO: If the process is attached as a subproc of another task, retrieve that task and complete it. */
   } else if (!strcmp (status, "error")) {
      /* TODO: Does this need special handling?  Figure that out. */
   }

   return 1;
}
int    wftk_task_subproc   (void * session, XML * task, XML * subproc_datasheet);
int    wftk_task_attach    (void * session, XML * task, XML * datasheet);
XML * wftk_task_retrieve (void * session, XML * task)
{
   XML * datasheet = NULL;  /* TODO: as usual, anything to do with taskindex. */
   XML * procdef = NULL;
   XML * mark;
   XML * data;
   WFTK_ADAPTOR * ad;
   const char *task_id;
   char sbuf[256];

   if (!task) return task;
   if (!strcmp (task->name, "datasheet")) {
      datasheet = task;
      task = xml_create ("task");
      xml_set (task, "process", xml_attrval (datasheet, "id"));
      xml_set (task, "dsrep", xml_attrval (datasheet, "repository"));
   } else {
      if (*xml_attrval (task, "process")) {
         datasheet = wftk_process_load (session, xml_attrval (task, "dsrep"), xml_attrval (task, "process"));
         xml_set (task, "process", xml_attrval (datasheet, "id"));
         xml_set (task, "dsrep", xml_attrval (datasheet, "repository"));
      }
   }

   if (!datasheet) {
      /* TODO: retrieval of non-datasheet tasks. (Needs doing in the ODBC adaptor, too.) */
      /*ad = wftk_get_adaptor (session, TASKINDEX, NULL);
      mark = wftk_call_adaptor (ad, "get", task);*/
      return 0;
   }

   procdef = _procdef_load (session, datasheet);

   task_id = xml_attrval (task, "id");
   if (*task_id && *task_id != '!') { /* Explicit task, either ad-hoc or workflow. */
      /* Let's look for an ad-hoc task with this ID. */
      sprintf (sbuf, "datasheet.task[%s]", task_id);
      mark = xml_loc (datasheet, sbuf);
      if (mark) {
         wftk_value_interpret (session, datasheet, xml_attrval (mark, "label"), sbuf, sizeof(sbuf));
         xml_set (task, "label", sbuf);
         xml_set (task, "status", "active");
         xml_set (task, "role", xml_attrval (mark, "role"));
         xml_set (task, "user", xml_attrval (mark, "user"));
         xml_getloc (mark, sbuf, sizeof(sbuf) - 1);
         xml_set (task, "loc", sbuf);
         mark = xml_firstelem (mark);  /* Copy data references over and that whole nine yards. */
         while (mark) {
            if (!strcmp (mark->name, "data")) {
               data = xml_create ("data");
               xml_set (data, "id", xml_attrval (mark, "id"));
               xml_set (data, "type", xml_attrval (mark, "type"));
               xml_set (data, "value", wftk_value_get (session, datasheet, xml_attrval (data, "id")));
               xml_set (data, "mode", *xml_attrval (data, "value")? "edit" : "input");
               xml_append (task, data);
            }
            mark = xml_nextelem (mark);
         }
         if (!session) {
            if (procdef) xml_free (procdef);
            xml_free (datasheet);
         }
         return (task); /* TODO: proper cleanup. */
      }

      /* Not found?  Then on to the workflow status, and retrieve active tasks from the procdef. */
      sprintf (sbuf, "datasheet.state.queue.item[%s]", xml_attrval (task, "id"));
      mark = xml_loc (datasheet, sbuf);
      if (mark) {
         xml_set (task, "role", xml_attrval (mark, "role"));
         xml_set (task, "user", xml_attrval (mark, "user"));
         if (!strcmp (xml_attrval (mark, "where"), "datasheet")) {
            mark = xml_loc (datasheet, xml_attrval (mark, "loc"));
         } else {
            mark = xml_loc (procdef, xml_attrval (mark, "loc"));
         }
      }
      if (mark) {  /* Mark is now the task definition, whether in procdef or ad-hoc in datasheet. */
         wftk_value_interpret (session, datasheet, xml_attrval (mark, "label"), sbuf, sizeof(sbuf));
         xml_set (task, "label", sbuf);
         xml_set (task, "status", "active");
         if (!*xml_attrval (task, "user")) { /* Get role/user from procdef if the datasheet ain't talkin. */
            xml_set (task, "role", xml_attrval (mark, "role"));
         }
         if (!*xml_attrval (task, "user")) {
            xml_set (task, "user", xml_attrval (mark, "user"));
         }
         xml_getloc (mark, sbuf, sizeof(sbuf) - 1);
         xml_set (task, "loc", sbuf);
         mark = xml_firstelem (mark);  /* Copy data references over and that whole nine yards. */
         while (mark) {
            if (!strcmp (mark->name, "data")) {
               data = xml_create ("data");
               xml_set (data, "id", xml_attrval (mark, "id"));
               xml_set (data, "type", xml_attrval (mark, "type"));
               xml_set (data, "value", wftk_value_get (session, datasheet, xml_attrval (data, "id")));
               xml_set (data, "mode", *xml_attrval (data, "value")? "edit" : "input");
               xml_append (task, data);
            }
            mark = xml_nextelem (mark);
         }
         if (!session) {
            if (procdef) xml_free (procdef);
            xml_free (datasheet);
         }
         return (task); /* TODO: proper cleanup. */
      }

      /* Still nothing.  No task. */
      xml_set (task, "status", "none");
   } else if (procdef) { /* Potential task. */
      /* If only one potential task exists, then we can return its data requirements.  Otherwise?  TODO: figure that out. */

      /* Special case: if status="start" then we return top-level data from the procdef, unless that's overridden. */
      if (!strcmp ("start", wftk_status_get (session, datasheet))) {
         xml_set (task, "status", "potential");
         xml_set (task, "label", "Start process");
         mark = xml_firstelem (procdef);
         while (mark) {
            if (!strcmp (mark->name, "data")) {
               data = xml_create ("data");
               xml_set (data, "id", xml_attrval (mark, "id"));
               xml_set (data, "type", xml_attrval (mark, "type"));
               xml_set (data, "value", wftk_value_get (session, datasheet, xml_attrval (data, "id")));
               xml_set (data, "mode", *xml_attrval (data, "value")? "edit" : "input");
               xml_append (task, data);
            }
            mark = xml_nextelem (mark);
         }
         return (task); /* TODO: proper cleanup. */
      }
      /* TODO: Find other potential tasks, if any, and return the first which presents itself. */

      /* No potential tasks available. */
      xml_set (task, "status", "none");
   } else { /* No procdef, no potential tasks. */
      xml_set (task, "status", "none");
   }

   /* TODO: if status="none" then check the enactment history for the named task.  It might be complete already. */

   return (task);
}
int wftk_task_update (void * session, XML * task)
{
   XML * datasheet = NULL;
   XML * mark;
   const char * task_id;
   char  sbuf[256]; /* TODO: eliminate with better loc */
   int   datachange = 0;
   int   taskchange = 0;
   WFTK_ADAPTORLIST * adlist;

   if (!task) return 0;
   if (*xml_attrval (task, "process")) {
      datasheet = wftk_process_load (session, xml_attrval (task, "dsrep"), xml_attrval (task, "process"));
   }
   if (datasheet && xml_loc (task, "task.data")) {
      mark = xml_firstelem (task);
      while (mark) {
         if (!strcmp ("data", mark->name)) {
            wftk_value_set (session, datasheet, xml_attrval (mark, "id"), xml_attrval (mark, "value"));
            datachange = 1;
         }
         mark = xml_nextelem (mark);
      }
   }

   task_id = xml_attrval (task, "id");
   if (*task_id && *task_id != '!') { /* Explicit task, either ad-hoc or workflow. */
      /* Let's look for an ad-hoc task with this ID. */
      sprintf (sbuf, "datasheet.task[%s]", task_id);
      mark = xml_loc (datasheet, sbuf);
      if (mark) {
         if (strcmp (xml_attrval (mark, "role"), xml_attrval (task, "role"))) {
            xml_set (mark, "role", xml_attrval (task, "role"));
            taskchange = 1;
         }
         if (strcmp (xml_attrval (mark, "user"), xml_attrval (task, "user"))) {
            xml_set (mark, "user", xml_attrval (task, "user"));
            wftk_user_retrieve (session, datasheet, xml_attrval (task, "user"));
            
            taskchange = 1;
         }
      } else {
         sprintf (sbuf, "datasheet.state.queue.item[%s]", xml_attrval (task, "id"));
         mark = xml_loc (datasheet, sbuf);
         if (mark) {
            if (strcmp (xml_attrval (mark, "role"), xml_attrval (task, "role"))) {
               xml_set (mark, "role", xml_attrval (task, "role"));
               taskchange = 1;
            }
            if (strcmp (xml_attrval (mark, "user"), xml_attrval (task, "user"))) {
               xml_set (mark, "user", xml_attrval (task, "user"));
               taskchange = 1;
            }
         }
      }
   }

   if (taskchange) { /* Update the task indices. */
      adlist = wftk_get_adaptorlist (session, TASKINDEX);
      wftk_call_adaptorlist (adlist, "taskput", task);
      wftk_free_adaptorlist (session, adlist);
   }

   if (datachange || taskchange) wftk_process_save (session, datasheet);
   return 1;
}

int wftk_task_complete (void * session, XML * task)
{
   XML * datasheet = NULL;
   XML * procdef = NULL;
   XML * state;
   XML * queue;
   char tasklocbuf[64]; /* TODO: Eliminate with better loc */
   XML * mark;
   const char * task_id;
   char sbuf[256]; /* TODO: Eliminate with better loc */
   int adhoc = 0;
   int complete = 1;
   WFTK_ADAPTORLIST * adlist = wftk_get_adaptorlist (session, TASKINDEX);

   if (!task) return 0;
   if (*xml_attrval (task, "process")) {
      datasheet = wftk_process_load (session, xml_attrval (task, "dsrep"), xml_attrval (task, "process"));
      procdef = _procdef_load (session, datasheet);
   }

   if (!datasheet) {
      wftk_call_adaptorlist (adlist, "taskcomplete", "", xml_attrval (task, "id"));
      wftk_free_adaptorlist (session, adlist);
      return 1; /* The task was ad-hoc with no process, so there's nothing left to do. */
   }

   wftk_task_update (session, task);

   /* Log enactment of task. */

   /* If task is ad-hoc, it needs to be deleted. */
   task_id = xml_attrval (task, "id");
   if (*xml_attrval (task, "id")) {
      sprintf (sbuf, "datasheet.task[%s]", xml_attrval (task, "id"));
      mark = xml_loc (datasheet, sbuf);
      if (mark) {
         xml_delete (mark);
         adhoc = 1;
         wftk_call_adaptorlist (adlist, "taskcomplete", xml_attrval (task, "process"), xml_attrval (task, "id"));
      }
   }

   if (adhoc) {  /* If the task is ad-hoc, we aren't going to do the queue thing below. */
      /* Check for completion: no active ad-hoc tasks, nothing left on the queue. */
      mark = xml_loc (datasheet, "datasheet.task");
      if (mark) complete = 0;
      if (complete) {
         mark = xml_loc (datasheet, "datasheet.state.queue.item");
         if (mark) complete = 0;
      }
      if (complete) wftk_status_set (session, datasheet, "complete");

      /* Now save everything and clean up. */
      wftk_enactment_write (session, datasheet, task, "action", "complete");
      wftk_process_save (session, datasheet);
      if (!session) {
         if (procdef) xml_free (procdef);
      }
      if (datasheet) xml_free (procdef);
      wftk_free_adaptorlist (session, adlist);
      return 1;
   }


   /* The process is a regular workflow process, with procdef.  Let's find (or create) the state queue. */
   state = xml_loc (datasheet, "datasheet.state");
   if (!state) {
      state = xml_create ("state");
      xml_append (datasheet, state);
   }
   queue = xml_loc (datasheet, "datasheet.state.queue");
   if (!queue) {
      queue = xml_create ("queue");
      xml_append (state, queue);
   }

   if (*task_id && *task_id != '!') { /* Explicit task, either ad-hoc or workflow. */
      sprintf (tasklocbuf, "queue.item[%s]", task_id);
      mark = xml_loc (queue, tasklocbuf);
      if (mark) { /* Workflow task. */
         xml_set (mark, "block", "resume");
         wftk_call_adaptorlist (adlist, "taskcomplete", xml_attrval (task, "process"), xml_attrval (task, "id"));
      } else {
         /* Ad-hoc task.  What has to happen?  TODO: Answer this question, then do it. */
      }
   } else {
      if (!strcmp ("start", wftk_status_get (session, datasheet))) {
         /* Special case: start state starts process. */
         queue_procdef (session, datasheet, state, queue, procdef, "procdef");
         wftk_status_set (session, datasheet, "active");
      } else {
         /* TODO: Find potential task based on state, and complete that. */
      }
   }

   process_procdef (session, datasheet, state, queue, procdef);

   /* Check for completion: no active ad-hoc tasks, nothing left on the queue. */
   mark = xml_loc (datasheet, "datasheet.task");
   if (mark) complete = 0;
   if (complete) {
      mark = xml_loc (datasheet, "datasheet.state.queue.item");
      if (mark) complete = 0;
   }
   if (complete) wftk_status_set (session, datasheet, "complete");

   wftk_enactment_write (session, datasheet, task, "action", "complete");
   wftk_process_save (session, datasheet);
   wftk_free_adaptorlist (session, adlist);

   return 1;
}
int wftk_task_list (void * session, XML * list)
{
   int count = 0;
   const char * state;
   const char * status;
   const char * userid;
   char sbuf[256];
   XML * datasheet = NULL;
   XML * procdef = NULL;
   XML * mark;
   XML * mark2;
   XML * hit;
   WFTK_ADAPTOR * ad;

   if (!list) return 0;

   if (*xml_attrval (list, "process")) {
      /* This is a process list. */
      state = xml_attrval (list, "state");
      if (!*state) state = "a";
      userid = xml_attrval (list, "user");

      datasheet = wftk_process_load (session, xml_attrval (list, "dsrep"), xml_attrval (list, "process"));
      if (!datasheet) { return 0; }

      if (strchr (state, 'a')) { /* Active tasks are included. */
         procdef = _procdef_load (session, datasheet);

         /* Find ad-hoc tasks. */
         mark = xml_firstelem (datasheet);
         while (mark) {
            if (!strcmp (mark->name, "task") && (!*userid || !strcmp (userid, xml_attrval (mark, "user")))) {
               hit = xml_create ("task");
               xml_set (hit, "id", xml_attrval (mark, "id"));
               wftk_value_interpret (session, datasheet, xml_attrval (mark, "label"), sbuf, sizeof(sbuf));
               xml_set (hit, "label", sbuf);
               xml_set (hit, "role", xml_attrval (mark, "role"));
               xml_set (hit, "user", xml_attrval (mark, "user"));
               xml_append (list, hit);
            }
            mark = xml_nextelem (mark);
         }

         /* Find active workflow tasks. */
         mark = xml_loc (datasheet, "datasheet.state.queue");
         if (mark) {
            mark = xml_firstelem (mark);
            while (mark) {
               if (!strcmp (xml_attrval (mark, "type"), "task")) {
                  hit = xml_create ("task");
                  xml_set (hit, "id", xml_attrval (mark, "id"));
                  xml_set (hit, "user", xml_attrval (mark, "user"));
                  if (!strcmp (xml_attrval (mark, "where"), "datasheet")) {
                     mark2 = xml_loc (datasheet, xml_attrval (mark, "loc"));
                  } else {
                     mark2 = xml_loc (procdef, xml_attrval (mark, "loc"));
                  }
                  if (mark2) {
                     wftk_value_interpret (session, datasheet, xml_attrval (mark2, "label"), sbuf, sizeof (sbuf));
                     xml_set (hit, "label", sbuf);
                     xml_set (hit, "role", xml_attrval (mark2, "role"));
                  }
                  xml_append (list, hit);
               }
               mark = xml_nextelem (mark);
            }
         }

         /* Find potential tasks.  No procdef means (per defn) no potential tasks. */
         if (procdef) {
            status = wftk_status_get (session, datasheet);
            if (!strcmp (status, "start")) {
               /* Special case: start state. */
               hit = xml_create ("task");
               xml_set (hit, "id", "!active");
               xml_set (hit, "label", "Start process");
               xml_append (list, hit);
            }
         }
      }

      if (strchr (state, 'c')) { /* Closed tasks are included. */
         /* TODO: Find enactment history matches. */
      }

      /* TODO: Sort the list. */
   } else {
      /* No process means we have to ask a task index.  Fortunately this is *very easy*. */
      ad = wftk_get_adaptor (session, TASKINDEX, NULL);
      count = 0;
      if (ad) {
         wftk_call_adaptor (ad, "tasklist", list);
         wftk_free_adaptor (session, ad);
         count = xml_attrvalnum (list, "count");
      }
   }

   return count;
}
int wftk_task_new (void * session, XML * task)
{
   XML * datasheet = NULL;
   XML * newtask;
   XML * data;
   XML * newdata;
   WFTK_ADAPTORLIST * adlist;

   if (!task) return 0;
   if (*xml_attrval (task, "process")) {
      datasheet = wftk_process_load (session, xml_attrval (task, "dsrep"), xml_attrval (task, "process"));
      if (!datasheet) return 0;
   }

   if (datasheet && !*xml_attrval (task, "user") && *xml_attrval (task, "role")) {
      xml_set (task, "user", wftk_role_user (session, datasheet, xml_attrval (task, "role")));
   }

   /* Inform task indices of new ad-hoc task. */
   adlist = wftk_get_adaptorlist (session, TASKINDEX);
   wftk_call_adaptorlist (adlist, "tasknew", task);
   wftk_free_adaptorlist (session, adlist);

   if (!datasheet) return 1; /* The task was ad-hoc with no process, so there's nothing left to do. */

   newtask = xml_create ("task");
   xml_set (newtask, "id", xml_attrval (task, "id"));
   xml_set (newtask, "label", xml_attrval (task, "label"));
   xml_set (newtask, "role", xml_attrval (task, "role"));
   xml_set (newtask, "user", xml_attrval (task, "user"));
   data = xml_firstelem (task);
   while (data) {
      if (!strcmp (data->name, "data")) {
         newdata = xml_create ("data");
         xml_set (newdata, "id", xml_attrval (data, "id"));
         xml_set (newdata, "mode", xml_attrval (data, "mode"));
         xml_append (newtask, newdata);
      }
      data = xml_nextelem (data);
   }
   xml_append (datasheet, newtask);
   wftk_process_save (session, datasheet);

   return 1;
}


int wftk_task_rescind (void * session, XML * task)
{
   XML * datasheet = NULL;
   XML * mark;
   char sbuf[256];
   WFTK_ADAPTORLIST * adlist;

   if (!task) return 0;
   if (*xml_attrval (task, "process")) {
      datasheet = wftk_process_load (session, xml_attrval (task, "dsrep"), xml_attrval (task, "process"));
      if (!datasheet) return 0;
   }

   /* Inform task indices of rescinded ad-hoc task. */
   adlist = wftk_get_adaptorlist (session, TASKINDEX);
   wftk_call_adaptorlist (adlist, "taskdel", xml_attrval (task, "process"), xml_attrval (task, "id"));
   wftk_free_adaptorlist (session, adlist);

   if (!datasheet) return 1; /* The task was ad-hoc with no process, so there's nothing left to do. */

   sprintf (sbuf, "datasheet.task[%s]", xml_attrval (task, "id"));
   mark = xml_loc (datasheet, sbuf);
   if (mark) {
      xml_delete (mark);
   }
   wftk_enactment_write (session, datasheet, task, "action", "rescind");
   wftk_process_save (session, datasheet);

   return 1;
}
int    wftk_task_reject    (void * session, XML * task)
{
   XML * datasheet = NULL;
   XML * procdef = NULL;
   char tasklocbuf[64]; /* TODO: Eliminate with better loc */
   XML * mark;
   const char * task_id;
   char sbuf[256]; /* TODO: Eliminate with better loc */
   int adhoc = 0;
   int complete = 1;
   WFTK_ADAPTORLIST * adlist = wftk_get_adaptorlist (session, TASKINDEX);

   if (!task) return 0;
   if (*xml_attrval (task, "process")) {
      datasheet = wftk_process_load (session, xml_attrval (task, "dsrep"), xml_attrval (task, "process"));
      procdef = _procdef_load (session, datasheet);
   }

   if (!datasheet) {
      wftk_call_adaptorlist (adlist, "taskreject", "", xml_attrval (task, "id"));
      wftk_free_adaptorlist (session, adlist);
      return 1; /* The task was ad-hoc with no process, so there's nothing left to do. */
   }

   wftk_task_update (session, task);

   /* If task is ad-hoc, it needs to be marked as rejected (I'm not really comfortable with this.) TODO: think. */
   task_id = xml_attrval (task, "id");
   if (*xml_attrval (task, "id")) {
      sprintf (sbuf, "datasheet.task[%s]", xml_attrval (task, "id"));
      mark = xml_loc (datasheet, sbuf);
      if (mark) {
         xml_set (mark, "status", "rejected");
         adhoc = 1;
         wftk_call_adaptorlist (adlist, "taskreject", xml_attrval (task, "process"), xml_attrval (task, "id"));
      }
   }

   wftk_status_set (session, datasheet, "error");

   /* Now save everything and clean up. */
   wftk_enactment_write (session, datasheet, task, "action", "reject");
   wftk_process_save (session, datasheet);
   if (!session) {
      if (procdef) xml_free (procdef);
      xml_free (datasheet);
   }
   wftk_free_adaptorlist (session, adlist);
   return 1;
}
int    wftk_request_new      (void * session, XML * request) {
   XML * datasheet = NULL;
   XML * req;
   XML * requestor = NULL;
   XML * mark;
   XML * alert = NULL;
   const char * alertcontent;
   char * newalertcontent = NULL;
   char sbuf[1024]; /* TODO: yadda yadda. */
   WFTK_ADAPTORLIST * adlist;

   if (!request) return 0;
   if (*xml_attrval (request, "process")) {
      datasheet = wftk_process_load (session, xml_attrval (request, "dsrep"), xml_attrval (request, "process"));
      if (!datasheet) return 0;
   }

   if (datasheet && !*xml_attrval (request, "of") && *xml_attrval (request, "of-role")) {
      xml_set (request, "of", wftk_role_user (session, datasheet, xml_attrval (request, "of-role")));
   }

   if (!*xml_attrval (request, "of")) {
      xml_set (request, "status", "error");
      xml_set (request, "status.reason", "The requestee could not be determined.");
      return 0;
   }

   /* Inform task indices of new request. */
   adlist = wftk_get_adaptorlist (session, TASKINDEX);
   wftk_call_adaptorlist (adlist, "reqnew", request);
   wftk_free_adaptorlist (session, adlist);

   if (!datasheet) return 1; /* The request was ad-hoc with no process, so there's nothing left to do. */
   req = xml_create ("request");
   if (datasheet) xml_setnum (req, "id", unique_id (datasheet, NULL));
   xml_set (req, "of", xml_attrval (request, "of"));

   if (*xml_attrval (request, "by")) {
      requestor = wftk_user_retrieve (session, datasheet, xml_attrval (request, "by"));
   } else {
      requestor = wftk_session_getuser (session);
   }
   if (requestor) {
      xml_set (req, "by", xml_attrval (requestor, "id"));
   }

   if (*xml_attrval (request, "label")) {
      if (!datasheet) {
         wftk_value_interpret (session, datasheet, xml_attrval (request, "label"), sbuf, sizeof(sbuf));
         xml_set (req, "label", sbuf);
      } else {
         xml_set (req, "label", xml_attrval (request, "label"));
      }
   } else {
      /* TODO: default label for request depends on kind of request. */
      if (requestor) {
         sprintf (sbuf, "Request from %s", *xml_attrval (requestor, "name") ? xml_attrval (requestor, "name") : xml_attrval (requestor, "id"));
         xml_set (req, "label", sbuf);
      } else {
         xml_set (req, "label", "Anonymous request");
      }
   }

   if (*xml_attrval (request, "task") == '?') {
      xml_set (req, "request", xml_attrval (request, "task") + 1);
   } else if (*xml_attrval (request, "task")) {
      xml_set (req, "task", xml_attrval (request, "task"));
   }
   if (*xml_attrval (request, "role"))    xml_set (req, "role", xml_attrval (request, "role"));
   if (*xml_attrval (request, "request")) xml_set (req, "request", xml_attrval (request, "request"));

   if (*xml_attrval (req, "request")) {
      /* TODO: some of this stuff should be done before the taskindex is updated with the request, right? */
      if (xml_attrvalnum (req, "request") >= xml_attrvalnum (req, "id")) {
         xml_set (request, "status", "error");
         xml_set (request, "status.reason", "Can't issue subrequest for nonexistent request");
         return 0;
      }
   }

   /* Transfer the content of the request, interpreting any alert found. */
   mark = xml_firstelem (request);
   while (mark) {
      if (!strcmp (mark->name, "alert") && !alert) {
         alert = xml_create ("alert");
         alertcontent = xml_stringcontent (mark);
         newalertcontent = (char *) malloc (strlen (alertcontent) + 1024); /* TODO: fixed lengths are dangerous! */
         wftk_value_interpret (session, datasheet, alertcontent, newalertcontent, strlen (alertcontent) + 1024);
         xml_append (alert, xml_createtext (newalertcontent));
         free (alertcontent);
         free (newalertcontent);
         xml_append (req, alert);
      } else {
         xml_append (req, xml_copy (mark));
      }
      mark = xml_nextelem (mark);
   }

   /* If there was no custom alert, build a logical one. */
   if (!alert) {
      alert = xml_create ("alert");
      xml_append (alert, xml_createtext ("A request has been made for your action.\nPlease visit the workflow app to accept or decline.\n"));
      /* TODO: Boy, that is a useless alert.  Write a better one. */
      xml_prepend (req, alert);
   }

   xml_set (alert, "to", xml_attrval (req, "of"));
   xml_set (alert, "from", xml_attrval (req, "by"));
   xml_set (alert, "subject", xml_attrval (req, "label"));

   /* Do the alert -- use custom content if found above, otherwise do something logical. */
   wftk_notify (session, datasheet, alert);

   xml_append (datasheet, req);
   wftk_enactment_write (session, datasheet, request, "action", "place");
   wftk_process_save (session, datasheet);

   return 1;
}
XML  * wftk_request_retrieve (void * session, XML * request) {
   XML * datasheet;
   WFTK_ADAPTOR * ad;
   XML * mark;

   if (!request) return 0;
   if (*xml_attrval (request, "process")) {
      datasheet = wftk_process_load (session, xml_attrval (request, "dsrep"), xml_attrval (request, "process"));
   }

   if (!datasheet) {
      /*ad = wftk_get_adaptor (session, TASKINDEX, NULL);
      wftk_call_adaptor (adlist, "reqget", xml_attrval (request, "id"));
      wftk_free_adaptor (session, adlist);
      */ /* TODO: the right thing. */
      return 0;
   }

   mark = xml_firstelem (datasheet);
   while (mark) {
      if (!strcmp (mark->name, "request") && !strcmp (xml_attrval (mark, "id"), xml_attrval (request, "id"))) {
         xml_copyinto (request, mark);
         return (request);
      }
      mark = xml_nextelem (mark);
   }

   return NULL;
}
int    wftk_request_update   (void * session, XML * request) { return 0; } /* TODO: figure out if this even makes sense. */
int    wftk_request_rescind  (void * session, XML * request) {
   XML * datasheet = NULL;
   XML * mark;
   char sbuf[256];
   WFTK_ADAPTORLIST * adlist;

   if (!request) return 0;
   if (*xml_attrval (request, "process")) {
      datasheet = wftk_process_load (session, xml_attrval (request, "dsrep"), xml_attrval (request, "process"));
      if (!datasheet) return 0;
   }

   /* Inform task indices of rescinded request. */
   adlist = wftk_get_adaptorlist (session, TASKINDEX);
   wftk_call_adaptorlist (adlist, "reqdel", xml_attrval (request, "process"), xml_attrval (request, "id"));
   wftk_free_adaptorlist (session, adlist);

   if (!datasheet) return 1; /* The task was ad-hoc with no process, so there's nothing left to do. */

   sprintf (sbuf, "datasheet.request[%s]", xml_attrval (request, "id"));
   mark = xml_loc (datasheet, sbuf);
   if (mark) {
      xml_delete (mark);
   }
   wftk_enactment_write (session, datasheet, request, "action", "rescind");
   wftk_process_save (session, datasheet);

   return 1;
}
int    wftk_request_accept   (void * session, XML * request) {
   XML * datasheet = NULL;
   XML * req;
   XML * mark;
   XML * task;
   int first = 1;
   char sbuf[256];
   WFTK_ADAPTORLIST * adlist;

   if (!request) return 0;
   if (*xml_attrval (request, "process")) {
      datasheet = wftk_process_load (session, xml_attrval (request, "dsrep"), xml_attrval (request, "process"));
      if (!datasheet) return 0;
   }


   if (datasheet) {
      sprintf (sbuf, "datasheet.request[%s]", xml_attrval (request, "id"));
      req = xml_loc (datasheet, sbuf);
      if (!req) {
         xml_set (request, "status", "error");
         xml_set (request, "status.reason", "No such request");
         return 0;
      }
      if (!strcmp (xml_attrval (req, "status"), "accepted")) {
         xml_set (request, "status", "error");
         xml_set (request, "status.reason", "Request has already been accepted.");
         return 0;
      }
   }

   /* Inform task indices of accepted request. */
   adlist = wftk_get_adaptorlist (session, TASKINDEX);
   wftk_call_adaptorlist (adlist, "reqaccept", xml_attrval (request, "process"), xml_attrval (request, "id"));

   if (!datasheet) {
      wftk_free_adaptorlist (session, adlist);
      return 1; /* The task was ad-hoc with no process, so there's nothing left to do. */
   }

   /* TODO: some of this stuff should happen before the taskindex is written. */
   if (!*xml_attrval (request, "accepting")) xml_set (request, "accepting", xml_attrval (req, "of"));
   xml_set (req, "accepting", xml_attrval (request, "accepting"));
   xml_set (req, "status", "accepted");
   if (*xml_attrval (req, "request")) {
      sprintf (sbuf, "datasheet.request[%s]", xml_attrval (req, "request"));
      task = xml_loc (datasheet, sbuf);
      if (task) {
         xml_set (task, "accepting", xml_attrval (req, "accepting")); /* Not "of", because then you couldn't send subsubreqs. */
         xml_set (task, "dsrep", xml_attrval (datasheet, "repository"));
         xml_set (task, "process", xml_attrval (datasheet, "id"));
         wftk_request_accept (session, task);
      }
   }
   if (*xml_attrval (req, "role")) {
      wftk_role_assign (session, datasheet, xml_attrval (req, "role"), xml_attrval (req, "accepting"));
   }
   if (*xml_attrval (req, "task")) {
      sprintf (sbuf, "datasheet.task[%s]", xml_attrval (req, "task"));
      task = xml_loc (datasheet, sbuf);
      if (!task) {
         sprintf (sbuf, "datasheet.state.queue.item[%s]", xml_attrval (req, "task"));
         task = xml_loc (datasheet, sbuf);
      }
      if (task) {
         xml_set (task, "user", xml_attrval (req, "accepting"));
         xml_set (task, "process", xml_attrval (datasheet, "id"));
         wftk_call_adaptorlist (adlist, "taskput", task);
      }
   }

   /* Now we handle the contents, if any.  The first alert is the one which went to the requestee at the outset, so we skip it. */
   mark = xml_firstelem (req);
   while (mark) {
      if (!strcmp (mark->name, "alert")) {
         if (first) {
            first = 0;
         } else {
            wftk_notify (session, datasheet, mark);
         }
      } else if (!strcmp (mark->name, "task")) {
         xml_set (mark, "user", xml_attrval (req, "accepting"));
         xml_set (mark, "dsrep", xml_attrval (datasheet, "repository"));
         xml_set (mark, "process", xml_attrval (datasheet, "id"));
         wftk_task_new (session, xml_copy (mark));
      } else {
         wftk_process_adhoc (session, datasheet, xml_copy (mark));
      }
      mark = xml_nextelem (mark);
   }

   wftk_enactment_write (session, datasheet, request, "action", "accepted"); /* Also saves datasheet. */
   wftk_process_save (session, datasheet);
   wftk_free_adaptorlist (session, adlist);

   return 1;
}
int    wftk_request_decline  (void * session, XML * request) {
   XML * datasheet = NULL;
   XML * mark;
   char sbuf[256];
   WFTK_ADAPTORLIST * adlist;

   if (!request) return 0;
   if (*xml_attrval (request, "process")) {
      datasheet = wftk_process_load (session, xml_attrval (request, "dsrep"), xml_attrval (request, "process"));
      if (!datasheet) return 0;
   }

   /* Inform task indices of declined request. */
   adlist = wftk_get_adaptorlist (session, TASKINDEX);
   wftk_call_adaptorlist (adlist, "reqdecline", xml_attrval (request, "process"), xml_attrval (request, "id"));
   wftk_free_adaptorlist (session, adlist);

   if (!datasheet) return 1; /* The task was ad-hoc with no process, so there's nothing left to do. */


   sprintf (sbuf, "datasheet.request[%s]", xml_attrval (request, "id"));
   mark = xml_loc (datasheet, sbuf);
   if (mark) {
      xml_set (mark, "status", "declined");
   }
   wftk_enactment_write (session, datasheet, request, "action", "declined"); /* Also saves datasheet. */
   wftk_process_save (session, datasheet);

   /* TODO: notify the requestor that something's amiss. */

   return 1;
}
int wftk_request_list (void * session, XML * list) {
   int count = 0;
   char sbuf[256];
   const char * userid;
   XML * datasheet = NULL;
   XML * procdef = NULL;
   XML * mark;
   XML * mark2;
   XML * hit;
   WFTK_ADAPTOR * ad;

   if (*xml_attrval (list, "process")) {
      /* This is a process list. */
      userid = xml_attrval (list, "user");

      datasheet = wftk_process_load (session, xml_attrval (list, "dsrep"), xml_attrval (list, "process"));
      if (!datasheet) { return 0; }

      mark = xml_firstelem (datasheet);
      while (mark) {
         if (!strcmp (mark->name, "request") 
             && strcmp ("declined", xml_attrval (mark, "status"))
             && strcmp ("accepted", xml_attrval (mark, "status"))
             && (!*userid || !strcmp (userid, xml_attrval (mark, "of")))) {
            hit = xml_create ("request");
            xml_set (hit, "id", xml_attrval (mark, "id"));
            xml_set (hit, "label", xml_attrval (mark, "label"));
            xml_set (hit, "of", xml_attrval (mark, "of"));
            xml_set (hit, "by", xml_attrval (mark, "by"));
            xml_set (hit, "request", xml_attrval (mark, "request"));
            xml_set (hit, "role", xml_attrval (mark, "role"));
            xml_set (hit, "task", xml_attrval (mark, "task"));
            xml_append (list, hit);
         }
         mark = xml_nextelem (mark);
      }
      xml_setnum (list, "count", count);

      /* TODO: sort the list. */
   } else {
      /* No process means we have to ask a task index.  Fortunately this is *very easy*. */
      ad = wftk_get_adaptor (session, TASKINDEX, NULL);
      count = 0;
      if (ad) {
         wftk_call_adaptor (ad, "reqlist", list);
         wftk_free_adaptor (session, ad);
         count = xml_attrvalnum (list, "count");
      }
   }

   return count;
}
int    wftk_role_list   (void * session, XML * datasheet, XML * list)
{
   int count = 0;
   XML * mark;
   XML * hit;
   ATTR * attr;

   if (!list) return 0;
   mark = xml_firstelem (datasheet);
   while (mark) {
      if (!strcmp (mark->name, "role")) {
         hit = xml_create ("role");
         attr = mark->attrs;
         while (attr) {
            xml_set (hit, attr->name, attr->value);
            attr = attr->next;
         }
         xml_append (list, hit);
      }
      mark = xml_nextelem (mark);
   }

   return count;
}

const char * wftk_role_user (void * session, XML * datasheet, const char * role)
{
   XML * mark;

   if (!role) return "";
   mark = xml_firstelem (datasheet);
   while (mark) {
      if (!strcmp (mark->name, "role") && !strcmp (xml_attrval (mark, "name"), role)) {
         return (xml_attrval (mark, "user"));
      }
      mark = xml_nextelem (mark);
   }

   return "";
}

int    wftk_role_assign (void * session, XML * datasheet, const char * role, const char * userid)
{
   XML * mark;

   if (!role || !userid) return 0;
   if (*userid) wftk_user_retrieve (session, datasheet, userid);

   /* TODO: reassign existing role-based tasks based on new role assignment. */

   mark = xml_firstelem (datasheet);
   while (mark) {
      if (!strcmp (mark->name, "role") && !strcmp (xml_attrval (mark, "name"), role)) {
         xml_set (mark, "user", userid);
         return 1;
      }
      mark = xml_nextelem (mark);
   }

   mark = xml_create ("role");
   xml_set (mark, "name", role);
   xml_set (mark, "user", userid);
   xml_append (datasheet, mark);
   return 1;
}
int wftk_user_list (void * session, XML * datasheet, XML * list)
{
   int count = 0;
   XML * mark;
   XML * hit;
   ATTR * attr;

   if (!list) return 0;
   mark = xml_firstelem (datasheet);
   while (mark) {
      if (!strcmp (mark->name, "user")) {
         hit = xml_create ("user");
         attr = mark->attrs;
         while (attr) {
            xml_set (hit, attr->name, attr->value);
            attr = attr->next;
         }
         xml_append (list, hit);
      }
      mark = xml_nextelem (mark);
   }

   return count;
}

int    wftk_user_add      (void * session, XML * datasheet, XML * user)
{
   XML * mark;

   mark = xml_firstelem (datasheet);
   while (mark) {
      if (!strcmp (mark->name, "user") && !strcmp (xml_attrval (mark, "id"), xml_attrval (user, "id"))) {
         return 0;
      }
      mark = xml_nextelem (mark);
   }

   xml_append (datasheet, user);
   return 1;
}

XML  * wftk_user_retrieve (void * session, XML * datasheet, const char * userid)
{
   XML * mark;

   if (!userid) return 0;
   mark = xml_firstelem (datasheet);
   while (mark) {
      if (!strcmp (mark->name, "user") && !strcmp (xml_attrval (mark, "id"), userid)) {
         return (mark);
      }
      mark = xml_nextelem (mark);
   }

   mark = xml_create ("user");
   xml_set (mark, "id", userid);
   xml_append (datasheet, mark);
   wftk_user_synch (session, mark);
   wftk_process_save (session, datasheet);
   return (mark);
}

int    wftk_user_update (void * session, XML * datasheet, XML * user)
{
   XML * mark;

   mark = xml_firstelem (datasheet);
   while (mark) {
      if (!strcmp (mark->name, "user") && !strcmp (xml_attrval (mark, "id"), xml_attrval (user, "id"))) {
         xml_replace (mark, user);
         return 1;
      }
      mark = xml_nextelem (mark);
   }

   return 0;
}

int    wftk_user_remove (void * session, XML * datasheet, const char * userid)
{
   XML * mark;

   if (!userid) return 0;
   mark = xml_firstelem (datasheet);
   while (mark) {
      if (!strcmp (mark->name, "user") && !strcmp (xml_attrval (mark, "id"), userid)) {
         xml_delete (mark);
         return 1;
      }
      mark = xml_nextelem (mark);
   }

   return 0;
}
int wftk_user_synch (void * session, XML * user)
{
   XML * directory_entry;
   ATTR * attr;
   WFTK_ADAPTOR * ad;

   if (!user) return 0;

   ad = wftk_get_adaptor (session, USER, xml_attrval (user, "directory"));
   if (!ad) return 0;

   directory_entry = wftk_call_adaptor (ad, "get", xml_attrval (user, "id"));
   wftk_free_adaptor (session, ad);

   if (directory_entry) {
      attr = directory_entry -> attrs;
      while (attr) {
         xml_set (user, attr->name, attr->value);
         attr = attr->next;
      }
   }
   return 1;
}
int wftk_user_auth (void * session, XML * user, const char * password)
{
   WFTK_ADAPTOR * ad;
   XML * ret;

   if (!user) return 0;

   ad = wftk_get_adaptor (session, USER, xml_attrval (user, "directory"));
   if (!ad) return 0;

   ret = wftk_call_adaptor (ad, "auth", user, password);
   wftk_free_adaptor (session, ad);

   if (ret) return 1;
   return 0;
}
int wftk_notify (void * session, XML * context, XML * alert) {
   WFTK_ADAPTOR * ad;
   WFTK_ADAPTORLIST * adlist;
   XML * user = context;
   XML * from;
   const char * preferred_modality = "";
   const char * always_list;
   char buf[1024]; /* TODO: the usual. */

   if (strcmp (context->name, "user")) {
      wftk_value_interpret (session, context, xml_attrval (alert, "to"), buf, sizeof(buf));
      user = xml_firstelem (user);
      while (user) {
         if (!strcmp (user->name, "user") && !strcmp (xml_attrval (user, "id"), buf)) break;
         user = xml_nextelem (user);
      }
   }

   if (*xml_attrval (alert, "from")) {
      wftk_value_interpret (session, context, xml_attrval (alert, "from"), buf, sizeof(buf));
      from = wftk_user_retrieve (session, context, buf);
      preferred_modality = xml_attrval (from, "notifyvia");
      if (!*preferred_modality) preferred_modality = config_get_value (session, "notify.default");
      if (*preferred_modality) {
         xml_set (alert, "from_addr", xml_attrval (from, preferred_modality));
      }
      xml_set (alert, "from_name", xml_attrval (from, "name"));
   } else {
      xml_set (alert, "from_addr", config_get_value (session, "notify.system_from"));
      wftk_value_interpret (session, context, config_get_value (session, "notify.system_name"), buf, sizeof(buf));
      xml_set (alert, "from_name", buf);
   }

   if (user) {
      wftk_user_synch (session, user);
      preferred_modality = xml_attrval (user, "notifyvia");
      if (!*preferred_modality) preferred_modality = config_get_value (session, "notify.default");
      if (*preferred_modality) {
         xml_set (alert, "to_addr", xml_attrval (user, preferred_modality));
         xml_set (alert, "to_name", xml_attrval (user, "name"));

         sprintf (buf, "notify.%s", preferred_modality);
         ad = wftk_get_adaptor (session, NOTIFY, config_get_value (session, buf));
         if (ad) {
            wftk_call_adaptor (ad, "send", context, alert);
            wftk_free_adaptor (session, ad);
         }
      }
   } else {
      preferred_modality = config_get_value (session, "notify.default");
      wftk_value_interpret (session, context, xml_attrval (alert, "to"), buf, sizeof(buf));
      xml_set (alert, "to_addr", buf);

      sprintf (buf, "notify.%s", preferred_modality);
      ad = wftk_get_adaptor (session, NOTIFY, config_get_value (session, buf));
      if (ad) {
         wftk_call_adaptor (ad, "send", context, alert);
         wftk_free_adaptor (session, ad);
      }
   }

   adlist = wftk_get_adaptorlist (session, NOTIFY);
   if (adlist) {
      wftk_call_adaptorlist (adlist, "send", context, alert);
      wftk_free_adaptorlist (session, adlist);
   }
   return 1;
}
int _wftk_decide_test (void * session, XML * datasheet, XML * element)
{
   int result = 0;
   char value[1024]; /* TODO: the same as always. */
   char * which;
   char test[1024];  /* TODO: except twice this time. */

   if (*xml_attrval (element, "equal")) {  /* Cutting corners.  TODO: maybe some more choices? */
      which = "equal";
   } else return result;

   wftk_value_interpret (session, datasheet, xml_attrval (element, "value"), value, sizeof(value));
   wftk_value_interpret (session, datasheet, xml_attrval (element, which),   test,  sizeof(test));

   if (!strcmp (which, "equal")) {
      result = !strcmp (value, test);
   }

   if (!strcmp (element->name, "unless")) return !result;
   return result;
}
int _wftk_decide_collapse (XML * result, XML * element)
{
   ATTR * attr;
   XML * mark;
   char locbuf[1024]; /* TODO: the usual */

   attr = element->attrs;
   while (attr) {
      if ((strcmp (attr->name, "value")) ||
          (strcmp (attr->name, "equal"))) {
         xml_set (result, attr->name, attr->value);
      }
      attr = attr->next;
   }
   xml_getloc (element, locbuf, sizeof(locbuf));
   xml_set (result, "loc", locbuf);
   mark = xml_first (element);
   while (mark) {
      xml_append (result, xml_copy (mark));
      mark = xml_next (mark);
   }
}
XML * wftk_decide (void * session, XML * datasheet, XML * decision) {
   XML * elem;
   XML * elem2;
   XML * mark;
   XML * result;
   ATTR * attr;
   int fire = 0;

   if (!decision) return 0;

   result = xml_create ("decide");
   attr = decision->attrs;
   while (attr) {
      xml_set (result, attr->name, attr->value);
      attr=attr->next;
   }
   elem = xml_firstelem (decision);
   while (elem) {
      if (!strcmp (elem->name, "if") || !strcmp (elem->name, "unless")) {
         if (_wftk_decide_test (session, datasheet, elem)) {
            _wftk_decide_collapse (result, elem);
            return (result);
         }
      } else if (!strcmp (elem->name, "else")) {
         _wftk_decide_collapse (result, elem);
         return (result);
      } else if (!strcmp (elem->name, "any")) {
         elem2 = xml_firstelem (elem);
         while (elem2) {
            fire = 0;
            if (!strcmp (elem2->name, "if") || !strcmp (elem2->name, "unless")) {
               if (_wftk_decide_test (session, datasheet, elem2)) {
                  _wftk_decide_collapse (result, elem);
                  _wftk_decide_collapse (result, elem2);
                  fire = 1;
               }
            } else if (!strcmp (elem2->name, "then") && fire) {
               _wftk_decide_collapse (result, elem);
               _wftk_decide_collapse (result, elem2);
               return (result);
            }
            elem2 = xml_nextelem (elem2);
         }
         if (fire) {
            _wftk_decide_collapse (result, elem);
            return (result);
         }
      } else if (!strcmp (elem->name, "all")) {
         elem2 = xml_firstelem (elem);
         while (elem2) {
            if (!strcmp (elem2->name, "if") || !strcmp (elem2->name, "unless")) {
               if (!_wftk_decide_test (session, datasheet, elem2)) {
                  break;
               }
            } else if (!strcmp (elem2->name, "then")) {
               _wftk_decide_collapse (result, elem);
               _wftk_decide_collapse (result, elem2);
               return (result);
            }
            elem2 = xml_nextelem (elem2);
         }
         if (!elem2) {
            _wftk_decide_collapse (result, elem);
            return (result);
         }
      } else if (!strcmp (elem->name, "decide")) {
         mark = wftk_decide (session, datasheet, elem);
         _wftk_decide_collapse (result, mark);
         xml_set (result, "loc", xml_attrval (mark, "loc"));
         xml_free (mark);
         return (result);
      }
      elem = xml_nextelem (elem);
   }
   return (result);
}
int wftk_action (void * session, XML * action) {
   XML * permission;
   WFTK_ADAPTOR * ad;
   XML * approval;
   XML * start;
   XML * user;

   if (!action) return 0;

   /* First, we check permissions. */
   ad = wftk_get_adaptor (session, PERMS, "localxml");  /* The specific permission adaptor is ignored anyway. */
   user = wftk_session_getuser (session);
   permission = wftk_call_adaptor (ad, "perm", action, user);
   wftk_free_adaptor (session, ad);
   if (!permission) {
      xml_set (action, "status", "error");
      return 0;
   }

   if (!strcmp (xml_attrval (permission, "value"), "ok")) {
      ad = wftk_get_adaptor (session, ACTION, xml_attrval (action, "handler"));
      wftk_call_adaptor (ad, "do", action);
      wftk_free_adaptor (session, ad);
      xml_set (action, "status", "ok");
      xml_set (action, "status.reason", xml_attrval (permission, "reason"));
      xml_free (permission);
      return 1;
   }

   if (!strcmp (xml_attrval (permission, "value"), "no")) {
      xml_set (action, "status", "no");
      xml_set (action, "status.reason", xml_attrval (permission, "reason"));
      xml_free (permission);
      return 1;
   }

   /* Start up approval process. */
   approval = wftk_process_new (session, NULL, NULL);
   xml_append (approval, xml_copy (action));
   wftk_process_define (session, approval, NULL, xml_attrval (permission, "value"));
   wftk_process_save (session, approval);
   if (user) {
      wftk_user_add (session, approval, user);
      wftk_role_assign (session, approval, "Requester", xml_attrval (user, "id"));
   }
   start = wftk_task_retrieve (session, approval);
   wftk_task_complete (session, start);
   xml_free (start);

   /* Tell the user what happened. */
   xml_set (action, "status", "deferred");
   xml_set (action, "status.reason", xml_attrval (permission, "reason"));
   xml_set (action, "dsrep", xml_attrval (approval, "repository"));
   xml_set (action, "process", xml_attrval (approval, "id"));

   xml_free (permission);
   return 1;
}
XML * wftk_enactment (void * session, XML * datasheet)
{
   XML * en;

   en = xml_loc (datasheet, "datasheet.enactment");
   if (!en) {
      en = xml_create ("enactment");
      xml_append (datasheet, en);
   }

   return (en);
}
int wftk_enactment_write (void * session, XML * datasheet, XML * xml, const char * attribute, const char * value)
{
   XML * copy;
   ATTR * attr;
   XML * en = wftk_enactment (session, datasheet);
   if (!en) return 0;

   if (!xml) return 0;

   copy = xml_create (xml->name);
   attr = xml->attrs;
   while (attr) {
      xml_set (copy, attr->name, attr->value);
      attr = attr->next;
   }
   if (wftk_session_getuser(session)) {
      xml_set (copy, "by", xml_attrval (wftk_session_getuser(session), "id"));
   }
   xml_set (copy, "at", _wftk_value_special (session, datasheet, "!now"));
   if (attribute && value) {
      xml_set (copy, attribute, value);
   }
   xml_append (en, copy);

   return 1;
}
int wftk_log (void * session, XML * datasheet, char * log)
{
   int retval;
   XML * logxml = xml_create ("log");

   xml_set (logxml, "text", log);
   retval = wftk_enactment_write (session, datasheet, logxml, NULL, NULL);
   xml_free (logxml);
   return (retval);
}
void * wftk_session_alloc ()
{
   WFTK_SESSION * sess;
   sess = (WFTK_SESSION *) malloc (sizeof (struct wftk_session));
   sess->ads = NULL;
   sess->datasheet = NULL;
   sess->procdef = NULL;
   sess->values = NULL;
   sess->cache = NULL;

   return (void *) sess;
}
void wftk_session_free (void * session)
{
   WFTK_SESSION * sess = session;
   WFTK_ADAPTOR_LIST * list;
   WFTK_CACHE_LIST * cachelist;
   if (!session) return;

   while (sess->ads) {
      wftk_free_adaptor (NULL, sess->ads->ad);
      list = sess->ads->next;
      free (sess->ads);
      sess->ads = list;
   }

   while (sess->cache) {
      xml_free (sess->cache->cached);
      cachelist = sess->cache->next;
      free(sess->cache);
      sess->cache = cachelist;
   }

   if (sess->user)      xml_free (sess->user);
   if (sess->config)    xml_free (sess->config);
   if (sess->datasheet) xml_free (sess->datasheet);
   if (sess->procdef)   xml_free (sess->procdef);
   if (sess->values)    xml_free (sess->values);
}
void wftk_session_configure (void * session, XML * config)
{
   WFTK_SESSION * sess = session;
   sess->config = config;
}
void wftk_session_current (void * session, XML * object)
{
   WFTK_SESSION * sess = session;

   if (!session) return;
   if (!object) return;

   if (!strcmp (object->name, "datasheet")) {
      sess->datasheet = object;
      return;
   }
   if (!strcmp (object->name, "procdef")) {
      sess->procdef = object;
      return;
   }
}
XML * wftk_session_cache (void * session, XML * key, int * flag)
{
   WFTK_SESSION * sess = session;
   WFTK_CACHE_LIST * list;
   ATTR * attr;

   if (flag) *flag = 0;

   if (!session) return key;
   if (!key) return key;

   list = sess->cache;
   while (list) {
      if (!strcmp (key->name, list->cached->name)) {
         attr = key->attrs;
         while (attr) {
            if (strcmp (attr->value, xml_attrval (list->cached, attr->name))) break;
            attr = attr->next;
         }
         if (!attr) break;
      }
      list = list->next;
   }

   if (list) {
      if (flag) *flag = 1;
      xml_free (key);
      return list->cached;
   }

   list = sess->cache;
   sess->cache = (WFTK_CACHE_LIST *) malloc (sizeof (WFTK_CACHE_LIST));
   sess->cache->cached = key;
   sess->cache->next = list;

   return key;
}

XML * wftk_session_cachecheck (void * session, XML * key)
{
   WFTK_SESSION * sess = session;
   WFTK_CACHE_LIST * list;
   ATTR * attr;

   if (!session) return NULL;
   if (!key) return NULL;

   list = sess->cache;
   while (list) {
      if (!strcmp (key->name, list->cached->name)) {
         attr = key->attrs;
         while (attr) {
            if (strcmp (attr->value, xml_attrval (list->cached, attr->name))) break;
            attr = attr->next;
         }
         if (!attr) break;
      }
      list = list->next;
   }

   if (list) {
      return list->cached;  /* Found. */
   }

   return NULL; /* Not found. */
}
void wftk_session_setuser (void * session, char * userid)
{
   WFTK_SESSION * sess = session;

   if (!session) return;
   if (sess->user) { xml_free (sess->user); }

   sess->user = xml_create ("user");
   xml_set (sess->user, "id", userid);
}
XML * wftk_session_getuser (void *session)
{
   WFTK_SESSION * sess = session;

   if (!session) return NULL;
   return (sess->user);
}
XML * wftk_session_stashvalue (void * session, const char * value)
{
   WFTK_SESSION * sess = session;
   XML * holder;

   if (!sess->values) sess->values = xml_create ("list");

   holder = xml_create ("value");
   xml_set (holder, "value", value);
   xml_append (sess->values, holder);

   return holder;
}

void wftk_session_freevalue (void * session, const char * value)
{
   WFTK_SESSION * sess = session;
   ATTR * attribute;
   XML * pointer = xml_firstelem (sess->values);

   while (pointer) {
      attribute = pointer->attrs;
      while (attribute) {
         if (attribute->value == value) {
            xml_delete (pointer);
            return;
         }
         attribute = attribute->next;
      }
      pointer = xml_nextelem (pointer);
   }
}
XML * wftk_info ()
{
   XML * ret = xml_create ("libinfo");
   xml_set (ret, "lib", "wftk");
   xml_set (ret, "ver", "1.0");
   xml_set (ret, "compiled", __TIME__ " " __DATE__);

   /*config_info (ret);  TODO: finish up the config info portion of this. */

   return (ret);
}
