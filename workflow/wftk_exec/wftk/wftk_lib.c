#include "wftk.h"
#include "wftk_lib.h"
XML *queue_procdef(XML * datasheet, XML * state, XML * queue, XML * action)
{
   XML *item;
   int idcount;
   char action_loc[1024];

   if (action == NULL)
      return NULL;

   idcount = atoi(xml_attrval(state, "idcount"));
   idcount++;
   xml_setnum(state, "idcount", idcount);

   item = xml_create("item");
   xml_setnum(item, "id", idcount);
   xml_set(item, "type", action->name);
   xml_getloc(action, action_loc, 1023);
   xml_set(item, "loc", action_loc);
   xml_append(queue, item);
   return (item);
}

void process_procdef(XML * datasheet, XML * state, XML * queue,
                     XML * procdef)
{
   XML *item;
   XML *def;
   XML *holder;
   XML *task;
   XML *data;
   XML *next;
   const char *type;
   int count;
   int keep;
   char sbuf[1024];

   item = xml_firstelem(queue);

   while (item != NULL) {
      if (!strcmp("yes", xml_attrval(item, "block"))) {
         item = xml_nextelem(item);
         continue;
      }
      def = xml_loc(procdef, xml_attrval(item, "loc"));
      type = xml_attrval(item, "type");

      keep = 0;
      if (!strcmp(type, "workflow") || !strcmp(type, "sequence")) {
         if (!strcmp("", xml_attrval(item, "cur"))) {
            next = xml_firstelem(def);
         } else {
            next = xml_loc(procdef, xml_attrval(item, "cur"));
            next = xml_nextelem(next);
         }

         if (next) {
            xml_set(queue_procdef(datasheet, state, queue, next), "parent",
                    xml_attrval(item, "id"));
            xml_getloc(next, sbuf, sizeof(sbuf) - 1);
            xml_set(item, "cur", sbuf);
            keep = 1;
         } else if (!strcmp(type, "workflow")) {
            /* TODO: notify database about completion? */
         }
      } else if (!strcmp(type, "parallel")) {
         if (!strcmp("", xml_attrval(item, "remaining"))) {
            count = 0;
            next = xml_firstelem(def);
            while (next != NULL) {
               count++;
               xml_set(queue_procdef(datasheet, state, queue, next),
                       "parent", xml_attrval(item, "id"));
               next = xml_nextelem(next);
            }
         } else {
            count = xml_attrvalnum(item, "remaining");
            count--;
         }
         xml_setnum(item, "remaining", count);
         if (count > 0)
            keep = 1;
      } else if (!strcmp(type, "task")) {
         if (strcmp(xml_attrval(item, "block"), "resume")) {
            sprintf(sbuf, "%s:%s", xml_attrval(datasheet, "process"),
                    xml_attrval(item, "id"));
            task = xml_create("task");
            xml_set(task, "id", sbuf);
            xml_append(datasheet, task);
            /*output ('A', "%s-%s-%s", sbuf, xml_attrval (def, "role"), xml_attrval (def, "label")); */

            holder = xml_firstelem(def);
            while (holder != NULL) {
               if (!strcmp(holder->name, "data")) {
                  data = xml_create("data");
                  xml_set(data, "id", xml_attrval(holder, "name"));
                  xml_set(data, "type", xml_attrval(holder, "type"));
                  xml_append(task, data);
               }
               holder = xml_nextelem(holder);
            }
            keep = 1;
         }
      } else if (!strcmp(type, "data")) {
      } else if (!strcmp(type, "situation")) {
      } else if (!strcmp(type, "if") || !strcmp(type, "elseif")) {
      } else if (!strcmp(type, "alert")) {
         wftk_notify(xml_attrval(item, "type"), xml_attrval(item, "to"),
                     xml_attrval(item, "from"), xml_attrval(item,
                                                            "subject"),
                     item);
      } else if (!strcmp(type, "start")) {
      }

      if (keep) {
         xml_set(item, "block", "yes");
         item = xml_nextelem(item);
      } else {
         if (strcmp("workflow", type)) {
            sprintf(sbuf, "queue.item[%d]",
                    xml_attrvalnum(item, "parent"));
            next = xml_loc(queue, sbuf);
            xml_delete(item);
            item = next;
            xml_set(item, "block", "no");
         } else {
            xml_delete(item);
            item = NULL;
         }
      }
   }
}
XML *wftk_action_create(char *dsrep, char *datasheet_id)
{
   WFTK_ADAPTOR *ad;

   ad = wftk_get_adaptor(DSREP, dsrep);
   if (!ad)
      return NULL;

   return (wftk_call_adaptor(ad, "create", datasheet_id));
}
int wftk_action_define(XML * datasheet, char *pdrep, char *procdef_id)
{
   WFTK_ADAPTOR *ad;
   XML *version;

   ad = wftk_get_adaptor(PDREP, pdrep);
   if (!ad)
      return 0;

   version = wftk_call_adaptor(ad, "version", procdef_id);
   if (!version)
      return 0;

   xml_set(datasheet, "pdrep", pdrep);
   xml_set(datasheet, "procdef", procdef_id);
   xml_set(datasheet, "version", xml_attrval(version, "value"));

   xml_free(version);
}
void wftk_action_complete(XML * datasheet, XML * procdef, char *task_id)
{
   XML *state;
   XML *queue;
   int queue_fresh = 0;
   char tasklocbuf[64];
   XML *item;

   /* TODO: Inform databases of change to task status. */

   state = xml_loc(datasheet, "datasheet.state");
   if (!state) {
      state = xml_create("state");
      xml_append(datasheet, state);
   }
   queue = xml_loc(datasheet, "datasheet.state.queue");
   if (!queue) {
      queue = xml_create("queue");
      xml_append(state, queue);
      queue_fresh = 1;
   }

   if (task_id && *task_id) {
      sprintf(tasklocbuf, "queue.item[%s]", task_id);
      item = xml_loc(queue, tasklocbuf);
      if (item) {
         xml_set(item, "block", "resume");
      }
   } else {
      if (queue_fresh) {
         queue_procdef(datasheet, state, queue, procdef);
      } else {
         /* TODO: Ask queued tasks who's now satisfied. */
      }
   }

   process_procdef(datasheet, state, queue, procdef);

   /* TODO: Inform databases of any changes to process status. */
}
void wftk_action_abort(XML * datasheet, XML * procdef, char *task_id)
{

}
void wftk_action_setvalue(XML * datasheet, char *name, char *value)
{
   XML *data;
   char valuelocbuf[128];

   sprintf(valuelocbuf, "datasheet.data[%s]", name);
   data = xml_loc(datasheet, valuelocbuf);

   if (!data) {
      data = xml_create("data");
      xml_set(data, "id", name);
      xml_append(datasheet, data);
   }

   xml_append(data, xml_createtext(value));
}

/* TODO: Implement wftk_action, if anybody really cares. */
void wftk_action(XML * datasheet, XML * procdef, XML * command)
{
}
XML *wftk_datasheet_load(char *dsrep, char *datasheet_id)
{
   WFTK_ADAPTOR *ad;

   ad = wftk_get_adaptor(DSREP, dsrep);
   if (!ad)
      return NULL;

   return (wftk_call_adaptor(ad, "load", datasheet_id));
}

void wftk_datasheet_save(XML * datasheet)
{
   WFTK_ADAPTOR *ad;
   ad = wftk_get_adaptor(DSREP, xml_attrval(datasheet, "repository"));
   if (!ad)
      return;

   wftk_call_adaptor(ad, "save", datasheet);
}

XML *wftk_procdef_load(XML * datasheet)
{
   WFTK_ADAPTOR *ad;
   ad = wftk_get_adaptor(PDREP, xml_attrval(datasheet, "pdrep"));
   if (!ad)
      return NULL;

   return (wftk_call_adaptor
           (ad, "load", xml_attrval(datasheet, "procdef"),
            xml_attrval(datasheet, "version")));
}
