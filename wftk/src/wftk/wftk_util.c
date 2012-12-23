#include "wftk.h"
#include <string.h>
#include <stdio.h>

#define PERIOD_TO_NULL(x) (strcmp(x, ".") ? x : (char *)0)

main (int argc, char * argv[])
{
   char config_path[512];
   char * chmark;
   FILE * config_file;
   FILE * file;
   int len;
   int  complain_if_no_file = 0;
   int argp = 1; /* We'll use this to skip flags like -c for location of the config file. */
   #define argsleft (argc - argp)
   XML * datasheet = (XML *) 0;
   XML * task = (XML *) 0;
   XML * list = (XML *) 0;
   void * session = (void *) 0;
   XML * mark;
   XML * action = (XML *) 0;

   if (argc < 2) {
      printf ("usage: wftk [command] -- type wftk help for more information.\n");
      exit (1);
   }

   session = wftk_session_alloc();

   while (*argv[argp] == '-') {
      if (!strcmp (argv[argp], "-c")) {
         argp++;
         strcpy (config_path, argv[argp++]);
         complain_if_no_file = 1;
      } else if (!strcmp (argv[argp], "-u")) {
         argp++;
         wftk_session_setuser (session, argv[argp++]);
      }
   }

   if (!complain_if_no_file) {
      strcpy (config_path, argv[0]);
      chmark = strrchr (config_path, '\\');
      if (chmark) chmark[1] = '\0';
      strcat (config_path, "config.xml");
   }

   config_file = fopen (config_path, "r");
   if (!config_file && complain_if_no_file) {
      printf ("wftk: can't open config file '%s'\n", config_path);
      exit (1);
   }
   if (config_file) {
      mark = xml_read_error (config_file);
      if (!strcmp (mark->name, "xml-error")) {
         printf ("wftk: error reading config file '%s'; '%s' in line %s\n", config_path, xml_attrval (mark, "message"), xml_attrval (mark, "line"));
         exit (1);
      } else wftk_session_configure (session, mark);
      fclose (config_file);
   }

   argp++;
   if (!strcmp (argv[argp-1], "help")) {
printf ("WFTK command-line interface v1.0 2001/02/18\n");
if (config_file) {
   printf ("Using configuration file in %s\n\n", config_path);
} else {
   printf ("Using precompiled configuration.\n\n");
}
printf (" -c <file> : specify alternate configuration file\n");
printf (" -u <user> : specify username for session (not authenticated)\n");
printf ("\n");

printf ("BASIC INFO:\n-----------\n");
printf (" help     -- this list\n");
printf (" info     -- (not implemented) library version and installation information\n");

printf ("\nPROCESSES:\n----------\n");
printf (" create   -- create a new process/datasheet\n");
printf (" delete   -- delete an existing process/datasheet\n");
printf (" define   -- associate a procdef with a process\n");
printf (" show     -- show the state of a process\n");
printf (" adhoc    -- attach and run ad-hoc workflow\n");

printf ("\nTASKS:\n------\n");
printf (" task     -- show a task (explicit or potential)\n");
printf (" tasks    -- list tasks\n");
printf (" todo     -- list active indexed tasks\n");
printf (" complete -- start process or complete a task\n");
printf (" reject   -- reject a task\n");
printf (" newtask  -- create a new ad-hoc task\n");
printf (" rescind  -- delete an ad-hoc task\n");
printf (" assign   -- assign a user to a task\n");

printf ("\nREQUESTS:\n---------\n");
printf (" ask      -- make a request\n");
printf (" requests -- list requests\n");
printf (" request  -- show a request\n");
printf (" accept   -- accept a request\n");
printf (" decline  -- decline a request\n");
printf (" forget   -- rescind a request\n");

printf ("\nVALUES:\n-------\n");
printf (" set       -- set a named value\n");
printf (" get       -- get a named value\n");
printf (" values    -- list the values in a datasheet\n");
printf (" html      -- show the HTML form field for a value\n");
printf (" htmlblank -- show a blank HTML form field for a value\n");

printf ("\nENACTMENT:\n----------\n");
printf (" log      -- show enactment history or write a log entry to the history\n");

printf ("\nROLES:\n------\n");
printf (" roles    -- list roles of process\n");
printf (" role     -- show or assign a role\n");

printf ("\nUSERS:\n------\n");
printf (" users    -- list involved users of process\n");
printf (" user     -- show a user or assign a user attribute\n");

printf ("\nACTIONS:\n--------\n");
printf (" action   -- perform an action with full permission/deferment protection\n");

   } else if (!strcmp (argv[argp-1], "create")) {
datasheet = wftk_process_new (session, argsleft > 0 ? PERIOD_TO_NULL(argv[argp]) : (char *) 0, argsleft > 1 ? PERIOD_TO_NULL(argv[argp+1]) : (char *) 0);  argp++; argp++;
if (datasheet) {
   wftk_process_save (session, datasheet);
   printf ("Created process %s\n", xml_attrval (datasheet, "id"));
}
   } else if (!strcmp (argv[argp-1], "delete")) {
if (argsleft < 2) {
   printf ("wftk delete: repository and ID required (use . for default repository)\n");
   exit(1);
}

wftk_process_delete (session, argsleft > 0 ? PERIOD_TO_NULL(argv[argp]) : (char *) 0, argv[argp+1]); argp++; argp++;
   } else if (!strcmp (argv[argp-1], "define")) {
if (argsleft < 4) {
   printf ("wftk define: dsrep and ID, pdrep and ID are all required (use . for default repositories)\n");
   exit (1);
}

datasheet = wftk_process_load (session, PERIOD_TO_NULL(argv[argp]), argv[argp+1]); argp++; argp++;
if (!datasheet) {
   printf ("Datasheet repository %s can't find datasheet %s", argv[argp-2], argv[argp-1]);
} else {
   if (wftk_process_define (session, datasheet, PERIOD_TO_NULL (argv[argp]), argv[argp+1])) {
      wftk_process_save (session, datasheet);
      printf ("Procdef %s, current version %s\n", xml_attrval (datasheet, "procdef"), xml_attrval (datasheet, "version"));
   }
}
   } else if (!strcmp (argv[argp-1], "show")) {
   } else if (!strcmp (argv[argp-1], "adhoc")) {
if (argsleft < 2) {
   printf ("wftk adhoc: dsrep and ID are required.  Use . for default repository\n");
   exit (1);
}

datasheet = wftk_process_load (session, PERIOD_TO_NULL(argv[argp]), argv[argp+1]); argp++; argp++;
if (!datasheet) {
   printf ("Datasheet repository %s can't find datasheet %s", argv[argp-2], argv[argp-1]);
} else {
   file = stdin;
   if (argsleft > 0) {
      file = fopen (argv[argp], "r");
      if (!file) {
         printf ("Can't open ad-hoc code file %s\n", argv[argp]);
      }
   }
   if (file) {
      mark = xml_read (file);
      if (!mark) {
         printf ("Can't read XML in ad-hoc input file %s\n", argv[argp]);
      }
      if (file != stdin) fclose (file);
      if (mark) {
         wftk_process_adhoc (session, datasheet, mark);
      }
   }
}

   } else if (!strcmp (argv[argp-1], "task")) {
if (argsleft < 2) {
   printf ("wftk task: dsrep and process ID required (use . for default repository)\n");
   exit (1);
}

datasheet = wftk_process_load (session, PERIOD_TO_NULL (argv[argp]), argv[argp+1]); argp++; argp++;
if (datasheet) {
   if (argsleft < 1) {
      task = wftk_task_retrieve (session, datasheet);
   } else {
      task = xml_create ("task");
      xml_set (task, "id", argv[argp++]);
      xml_set (task, "dsrep", xml_attrval (datasheet, "repository"));
      xml_set (task, "process", xml_attrval (datasheet, "id"));
      wftk_task_retrieve (session, task);
   }

   if (!strcmp (xml_attrval (task, "status"), "none")) {
      printf ("No task found.\n");
   } else {
      printf ("Task '%s'\n", *xml_attrval (task, "label") ? xml_attrval (task, "label") : xml_attrval (task, "id"));
      if (*xml_attrval (task, "role")) printf ("Role: %s\n", xml_attrval (task, "role"));
      if (*xml_attrval (task, "user")) printf ("Assigned to %s\n", xml_attrval (task, "user"));
      mark = xml_firstelem (task);
      while (mark) {
         if (!strcmp (mark->name, "data")) {
            printf (" %c %s: %s\n", strcmp (xml_attrval (mark, "mode"), "input") ? ' ' : '*',
                                    xml_attrval (mark, "id"), xml_attrval (mark, "value"));
         }
         mark = xml_nextelem (mark);
      }
   }
}
   } else if (!strcmp (argv[argp-1], "tasks")) {
if (argsleft < 2) {
   printf ("wftk task: dsrep and process ID required (use . for default repository)\n");
   exit (1);
}

list = xml_create ("list");
if (strcmp (argv[argp++], ".")) xml_set (list, "dsrep", argv[argp-1]);
xml_set (list, "process", argv[argp++]);
wftk_task_list (session, list);
mark = xml_firstelem (list);
if (!mark) {
   printf ("No tasks found.\n");
} else {
   while (mark) {
      printf ("%s: %s", xml_attrval (mark, "id"), xml_attrval (mark, "label"));
      if (*xml_attrval (mark, "role")) printf (" [%s]", xml_attrval (mark, "role"));
      if (*xml_attrval (mark, "user")) printf (" (%s)", xml_attrval (mark, "user"));
      printf ("\n");
      mark = xml_nextelem (mark);
   }
}
   } else if (!strcmp (argv[argp-1], "todo")) {
list = xml_create ("list");
if (argsleft > 0) xml_set (list, "user", argv[argp++]);
xml_set (list, "status", "active");
wftk_task_list (session, list);
mark = xml_firstelem (list);
if (!mark) {
   printf ("No tasks found.\n");
} else {
   while (mark) {
      printf ("%s > %s: %s", xml_attrval (mark, "process"), xml_attrval (mark, "id"), xml_attrval (mark, "label"));
      if (*xml_attrval (mark, "role")) printf (" [%s]", xml_attrval (mark, "role"));
      if (argc == 2 && *xml_attrval (mark, "user")) printf (" (%s)", xml_attrval (mark, "user"));
      printf ("\n");
      mark = xml_nextelem (mark);
   }
}
   } else if (!strcmp (argv[argp-1], "complete")) {
if (argsleft < 2) {
   printf ("wftk task: dsrep and process ID required (use . for default repository)\n");
   exit (1);
}

datasheet = wftk_process_load (session, PERIOD_TO_NULL (argv[argp]), argv[argp+1]); argp++; argp++;
if (datasheet) {
   if (argsleft < 1) {
      task = wftk_task_retrieve (session, datasheet);
   } else {
      task = xml_create ("task");
      xml_set (task, "id", argv[argp++]);
      xml_set (task, "dsrep", xml_attrval (datasheet, "repository"));
      xml_set (task, "process", xml_attrval (datasheet, "id"));
      wftk_task_retrieve (session, task);
   }

   if (wftk_task_complete (session, task)) {
      printf ("Completed.\n");
   } else {
      printf ("Not completed.\n");
   }
}
   } else if (!strcmp (argv[argp-1], "reject")) {
if (argsleft < 3) {
   printf ("wftk reject: dsrep, process ID, and task ID required (use . for default repository)\n");
   exit (1);
}

datasheet = wftk_process_load (session, PERIOD_TO_NULL (argv[argp]), argv[argp+1]); argp++; argp++;
if (!datasheet) {
   printf ("Datasheet repository %s can't find datasheet %s", argv[argp-2], argv[argp-1]);
} else {
   task = xml_create ("task");
   xml_set (task, "id", argv[argp++]);
   xml_set (task, "dsrep", xml_attrval (datasheet, "repository"));
   xml_set (task, "process", xml_attrval (datasheet, "id"));
   wftk_task_retrieve (session, task);

   if (wftk_task_reject (session, task)) {
      printf ("Rejected.\n");
   } else {
      printf ("No action taken.\n");
   }
}
   } else if (!strcmp (argv[argp-1], "newtask")) {
if (argsleft < 3) {
   printf ("wftk task: dsrep, process ID and task ID required (use . for default repository)\n");
   exit (1);
}

task = xml_create ("task");
if (strcmp (argv[argp++], ".")) xml_set (task, "dsrep", argv[argp-1]);
xml_set (task, "process", argv[argp++]);
xml_set (task, "id", argv[argp++]);
if (argsleft > 0) xml_set (task, "label", argv[argp++]);

if (wftk_task_new (session, task)) {
   printf ("Task added.\n");
} else {
   printf ("No task added.\n");
}
   } else if (!strcmp (argv[argp-1], "rescind")) {
if (argsleft < 3) {
   printf ("wftk task: dsrep, process ID and task ID required (use . for default repository)\n");
   exit (1);
}

task = xml_create ("task");
if (strcmp (argv[argp++], ".")) xml_set (task, "dsrep", argv[argp-1]);
xml_set (task, "process", argv[argp++]);
xml_set (task, "id", argv[argp++]);

if (wftk_task_rescind (session, task)) {
   printf ("Task rescinded.\n");
} else {
   printf ("No task rescinded.\n");
}
   } else if (!strcmp (argv[argp-1], "assign")) {
if (argsleft < 3) {
   printf ("wftk assign: dsrep, process ID and task ID required (use . for default repository)\n");
   exit (1);
}

task = xml_create("task");
xml_set (task, "dsrep", PERIOD_TO_NULL (argv[argp]) ? argv[argp] : ""); argp++;
xml_set (task, "process", argv[argp++]);
xml_set (task, "id", argv[argp++]);
   
if (!wftk_task_retrieve (session, task)) {
   printf ("Task %s is not active", argv[argp-1]);
} else {
   if (argsleft > 0) {
      xml_set (task, "user", argv[argp++]);
   } else {
      xml_set (task, "user", "");
   }
   wftk_task_update (session, task);
}

   } else if (!strcmp (argv[argp-1], "ask")) {
if (argsleft < 4) {
   printf ("wftk ask: dsrep, process ID, user ID of requestee, and requested object required (use . for default repository)\n");
   printf ("          For request object, use ?xx for request, xx or !xx for task assignment, @xxx.xxx for request file,\n");
   printf ("          or - for request on stdin.\n");
   printf ("          Add another optional argument to set a custom label (notification subject) for request.\n");
   exit (1);
}

task = xml_create ("request");
if (strcmp (argv[argp++], ".")) xml_set (task, "dsrep", argv[argp-1]);
xml_set (task, "process", argv[argp++]);
xml_set (task, "of", argv[argp++]);
chmark = argv[argp++];
if (argsleft > 0) xml_set (task, "label", argv[argp++]);

if (*chmark == '?') {
   xml_set (task, "request", chmark + 1);
} else if (*chmark == '@' || *chmark == '-') {
   if (*chmark == '@') {
      file = fopen (chmark + 1, "r");
      if (!file) {
         printf ("Unable to open request file %s.\n", chmark + 1);
         xml_free (task);
         exit (1);
      }
   } else {
      file = stdin;
   }
   mark = xml_read (file);
   if (file != stdin) fclose (file);
   if (!mark) {
      printf ("Bad XML in request.\n");
      xml_free (task);
      exit (1);
   }

   xml_copyinto (task, mark);
   xml_free (mark);
} else {
   xml_set (task, "task", chmark);
}

if (wftk_request_new (session, task)) {
   printf ("Request made.\n");
} else {
   printf ("No request made. %s\n", xml_attrval (task, "status.reason"));
}
   } else if (!strcmp (argv[argp-1], "requests")) {
if (argsleft < 2) {
   printf ("wftk requests: dsrep and process ID required (use . for default repository)\n");
   exit (1);
}

list = xml_create ("list");
if (strcmp (argv[argp++], ".")) xml_set (list, "dsrep", argv[argp-1]);
xml_set (list, "process", argv[argp++]);
wftk_request_list (session, list);
mark = xml_firstelem (list);
if (!mark) {
   printf ("No requests found.\n");
} else {
   while (mark) {
      printf ("%s: %s", xml_attrval (mark, "id"), xml_attrval (mark, "label"));
      if (*xml_attrval (mark, "of")) printf (" (%s)", xml_attrval (mark, "of"));
      if (*xml_attrval (mark, "request")) {
         printf (" - reassignment of request %s", xml_attrval (mark, "request"));
      }
      if (*xml_attrval (mark, "role")) {
         printf (" - assignment of role %s", xml_attrval (mark, "role"));
      }
      if (*xml_attrval (mark, "task")) {
         printf (" - reassignment of task %s", xml_attrval (mark, "task"));
      }
      printf ("\n");
      mark = xml_nextelem (mark);
   }
}
   } else if (!strcmp (argv[argp-1], "request")) {
if (argsleft < 3) {
   printf ("wftk request: dsrep, process ID, and request ID required (use . for default repository)\n");
   exit (1);
}

task = xml_create ("request");
if (strcmp (argv[argp++], ".")) xml_set (task, "dsrep", argv[argp-1]);
xml_set (task, "process", argv[argp++]);
xml_set (task, "id", argv[argp++]);
wftk_request_retrieve (session, task);

if (!strcmp (xml_attrval (task, "status"), "none")) {
   printf ("No request found.\n");
} else {
   printf ("Request '%s'\n", *xml_attrval (task, "label") ? xml_attrval (task, "label") : xml_attrval (task, "id"));
   if (*xml_attrval (task, "request")) {
      printf ("Subrequest for reassignment of request %s\n", xml_attrval (task, "request"));
   }
   if (*xml_attrval (task, "role")) {
      printf ("Request for assignment of role %s\n", xml_attrval (task, "role"));
   }
   if (*xml_attrval (task, "task")) {
      printf ("Request for reassignment of task %s\n", xml_attrval (task, "task"));
   }
   if (*xml_attrval (task, "by")) {
      printf ("Requested by: %s\n", xml_attrval (task, "by"));
   } else {
      printf ("Anonymous request\n");
   }
   if (*xml_attrval (task, "of")) printf ("Requestee: %s\n", xml_attrval (task, "of"));
}
   } else if (!strcmp (argv[argp-1], "accept")) {
if (argsleft < 3) {
   printf ("wftk task: dsrep, process ID, and request ID required (use . for default repository)\n");
   exit (1);
}

task = xml_create ("request");
if (strcmp (argv[argp++], ".")) xml_set (task, "dsrep", argv[argp-1]);
xml_set (task, "process", argv[argp++]);
xml_set (task, "id", argv[argp++]);

if (wftk_request_accept (session, task)) {
   printf ("Accepted.\n");
} else {
   printf ("Not accepted. %s\n", xml_attrval (task, "status.reason"));
}
   } else if (!strcmp (argv[argp-1], "decline")) {
if (argsleft < 3) {
   printf ("wftk task: dsrep, process ID, and request ID required (use . for default repository)\n");
   exit (1);
}

task = xml_create ("request");
if (strcmp (argv[argp++], ".")) xml_set (task, "dsrep", argv[argp-1]);
xml_set (task, "process", argv[argp++]);
xml_set (task, "id", argv[argp++]);

if (wftk_request_decline (session, task)) {
   printf ("Accepted.\n");
} else {
   printf ("Not accepted.\n");
}
   } else if (!strcmp (argv[argp-1], "forget")) {
if (argsleft < 3) {
   printf ("wftk task: dsrep, process ID and request ID required (use . for default repository)\n");
   exit (1);
}

task = xml_create ("request");
if (strcmp (argv[argp++], ".")) xml_set (task, "dsrep", argv[argp-1]);
xml_set (task, "process", argv[argp++]);
xml_set (task, "id", argv[argp++]);

if (wftk_request_rescind (session, task)) {
   printf ("Request rescinded.\n");
} else {
   printf ("No request rescinded.\n");
}

   } else if (!strcmp (argv[argp-1], "set")) {
if (argsleft < 4) {
   printf ("wftk set: repository and ID of the datasheet, the value name, and the value are all required.\n");
   exit (1);
}

datasheet = wftk_process_load (session, PERIOD_TO_NULL(argv[argp]), argv[argp+1]); argp++; argp++;
if (!datasheet) {
   printf ("Datasheet repository %s can't find datasheet %s", argv[argp-2], argv[argp-1]);
   exit (1);
}
wftk_value_set (session, datasheet, argv[argp], argv[argp+1]); argp++; argp++;
wftk_process_save (session, datasheet);
printf ("%s = %s\n", argv[argp-2], argv[argp-1]);
   } else if (!strcmp (argv[argp-1], "get")) {
if (argsleft < 3) {
   printf ("wftk get: repository and ID of the datasheet and the value name are all required.\n");
   exit (1);
}

datasheet = wftk_process_load (session, PERIOD_TO_NULL(argv[argp]), argv[argp+1]); argp++; argp++;
if (!datasheet) {
   printf ("Datasheet repository %s can't find datasheet %s", argv[argp-2], argv[argp-1]);
   exit (1);
}
printf ("%s\n", wftk_value_get (session, datasheet, argv[argp++]));
   } else if (!strcmp (argv[argp-1], "values")) {
if (argsleft < 2) {
   printf ("wftk list: repository and ID of the datasheet are required.\n");
   exit (1);
}

datasheet = wftk_process_load (session, PERIOD_TO_NULL(argv[argp]), argv[argp+1]); argp++; argp++;
if (!datasheet) {
   printf ("Datasheet repository %s can't find datasheet %s", argv[argp-2], argv[argp-1]);
   exit (1);
}

list = xml_create ("list");
wftk_value_list (session, datasheet, list);
mark = xml_firstelem (list);
if (!mark) {
   printf ("No values set.\n");
} else {
   while (mark) {
      printf ("%s = %s", xml_attrval (mark, "id"), xml_attrval (mark, "value"));
      printf ("\n");
      mark = xml_nextelem (mark);
   }
}
   } else if (!strcmp (argv[argp-1], "html")) {
if (argsleft < 3) {
   printf ("wftk get: repository and ID of the datasheet and the value name are all required.\n");
   exit (1);
}

datasheet = wftk_process_load (session, PERIOD_TO_NULL(argv[argp]), argv[argp+1]); argp++; argp++;
if (!datasheet) {
   printf ("Datasheet repository %s can't find datasheet %s", argv[argp-2], argv[argp-1]);
   exit (1);
}
mark = wftk_value_html (session, datasheet, argv[argp++]);
if (mark) {
   xml_write (stdout, mark);
   printf ("\n");
   xml_free (mark);
}
   } else if (!strcmp (argv[argp-1], "htmlblank")) {
if (argsleft < 3) {
   printf ("wftk get: repository and ID of the datasheet and the value name are all required.\n");
   exit (1);
}

datasheet = wftk_process_load (session, PERIOD_TO_NULL(argv[argp]), argv[argp+1]); argp++; argp++;
if (!datasheet) {
   printf ("Datasheet repository %s can't find datasheet %s", argv[argp-2], argv[argp-1]);
   exit (1);
}
mark = wftk_value_htmlblank (session, datasheet, argv[argp++]);
if (mark) {
   xml_write (stdout, mark);
   printf ("\n");
   xml_free (mark);
}

   } else if (!strcmp (argv[argp-1], "log")) {
if (argsleft < 2) {
   printf ("wftk log: repository and ID of the datasheet are required.\n");
   exit (1);
}

datasheet = wftk_process_load (session, PERIOD_TO_NULL(argv[argp]), argv[argp+1]); argp++; argp++;
if (!datasheet) {
   printf ("Datasheet repository %s can't find datasheet %s", argv[argp-2], argv[argp-1]);
   exit (1);
}

if (argsleft > 0) {
   /* Write log line. */
   wftk_log (session, datasheet, argv[argp++]);
   wftk_process_save (session, datasheet);
   exit (0);
}

list = wftk_enactment (session, datasheet);
if (list) {
   mark = xml_firstelem (list);
   while (mark) {
      printf ("%s [%s]: ", xml_attrval (mark, "at"), xml_attrval (mark, "by"));
      if (!strcmp ("log", mark->name)) {
         printf ("%s\n", xml_attrval (mark, "text"));
      } else if (!strcmp ("task", mark->name)) {
         if (!strcmp ("reject", xml_attrval (mark, "action"))) {
            printf ("REJECT ");
         }
         printf ("task %s %s\n", xml_attrval (mark, "id"), xml_attrval (mark, "label"));
      } else if (!strcmp ("data", mark->name)) {
         printf ("value %s (was '%s')\n", xml_attrval (mark, "id"), xml_attrval (mark, "was"));
      } else {
         printf ("entry '%s'\n", mark->name);
      }
      mark = xml_nextelem (mark);
   }
}

   } else if (!strcmp (argv[argp-1], "roles")) {
if (argsleft < 2) {
   printf ("wftk roles: repository and ID of the datasheet are required.\n");
   exit (1);
}

datasheet = wftk_process_load (session, PERIOD_TO_NULL(argv[argp]), argv[argp+1]); argp++; argp++;
if (!datasheet) {
   printf ("Datasheet repository %s can't find datasheet %s", argv[argp-2], argv[argp-1]);
   exit (1);
}

list = xml_create ("list");
wftk_role_list (session, datasheet, list);
mark = xml_firstelem (list);
while (mark) {
   printf ("%s : ", xml_attrval (mark, "name"));
   if (!*xml_attrval (mark, "user")) {
      printf ("[unassigned]\n");
   } else {
      printf ("%s\n", xml_attrval (mark, "user"));
   }
   mark = xml_nextelem (mark);
}
   } else if (!strcmp (argv[argp-1], "role")) {
if (argsleft < 3) {
   printf ("wftk role: repository and ID of the datasheet are required, plus the name of a role.\n");
   exit (1);
}

datasheet = wftk_process_load (session, PERIOD_TO_NULL(argv[argp]), argv[argp+1]); argp++; argp++;
if (!datasheet) {
   printf ("Datasheet repository %s can't find datasheet %s", argv[argp-2], argv[argp-1]);
   exit (1);
}

if (argsleft < 2) {
   printf ("%s : ", argv[argp]);
   if (!*wftk_role_user (session, datasheet, argv[argp])) {
      printf ("[unassigned]\n");
   } else {
      printf ("%s\n", wftk_role_user (session, datasheet, argv[argp]));
   }
} else {
   wftk_role_assign (session, datasheet, argv[argp], argv[argp+1]); argp++; argp++;
   wftk_process_save (session, datasheet);
}

   } else if (!strcmp (argv[argp-1], "users")) {
if (argsleft < 2) {
   printf ("wftk users: repository and ID of the datasheet are required.\n");
   exit (1);
}

datasheet = wftk_process_load (session, PERIOD_TO_NULL(argv[argp]), argv[argp+1]); argp++; argp++;
if (!datasheet) {
   printf ("Datasheet repository %s can't find datasheet %s", argv[argp-2], argv[argp-1]);
   exit (1);
}

list = xml_create ("list");
wftk_user_list (session, datasheet, list);
mark = xml_firstelem (list);
while (mark) {
   printf ("%s ", xml_attrval (mark, "id"));
   if (*xml_attrval (mark, "name")) {
      printf ("(%s) ", xml_attrval (mark, "name"));
   }
   if (*xml_attrval (mark, "email")) {
      printf (": %s", xml_attrval (mark, "email"));
   }
   printf ("\n");
   mark = xml_nextelem (mark);
}
   } else if (!strcmp (argv[argp-1], "user")) {
if (argsleft < 3) {
   printf ("wftk user: repository and ID of the datasheet and a userid are all required.\n");
   exit (1);
}

datasheet = wftk_process_load (session, PERIOD_TO_NULL(argv[argp]), argv[argp+1]); argp++; argp++;
if (!datasheet) {
   printf ("Datasheet repository %s can't find datasheet %s", argv[argp-2], argv[argp-1]);
   exit (1);
}

mark = wftk_user_retrieve (session, datasheet, argv[argp++]);
if (argsleft < 1) {
   if (!mark) {
      printf ("User %s apparently not involved with process %s\n", argv[argp-1], argv[argp-2]);
   } else {
      printf ("%s ", xml_attrval (mark, "id"));
      if (*xml_attrval (mark, "name")) {
         printf ("(%s) ", xml_attrval (mark, "name"));
      }
      if (*xml_attrval (mark, "email")) {
         printf (": %s", xml_attrval (mark, "email"));
      }
      printf ("\n");
   }
} else {
   if (argsleft < 2) {
      printf ("wftk user: to set attribute, both attribute and value are required.\n");
   } else {
      xml_set (mark, argv[argp], argv[argp+1]);
      wftk_process_save (session, datasheet);
   }
}

   } else if (!strcmp (argv[argp-1], "action")) {
if (argsleft < 1) {
   file = stdin;
} else {
   file = fopen (argv[argp], "r");
   if (!file) {
      printf ("Unable to open action definition file '%s' for reading.\n", argv[argp]);
      exit (1);
   }
}

action = xml_read (file);
if (argsleft > 0) fclose (file);
if (!action) {
   printf ("Unable to read action -- was your XML valid?\n");
   exit (1);
}

wftk_action (session, action);
if (!strcmp (xml_attrval (action, "status"), "error")) {
   printf ("Error while attempting action.\n");
} else if (!strcmp (xml_attrval (action, "status"), "ok")) {
   printf ("Action taken.\n");
} else if (!strcmp (xml_attrval (action, "status"), "no")) {
   if (!*xml_attrval (action, "status.reason")) {
      printf ("Action denied.\n");
   } else {
      printf ("Action denied: %s\n", xml_attrval (action, "status.reason"));
   }
} else {
   printf ("Action deferred pending approval.\nRepository %s\nProcess %s\n", xml_attrval (action, "dsrep"), xml_attrval (action, "process"));
}

   } else {
      printf ("Unknown command %s.\n", argv[argp-1]);
   }

   wftk_session_free (session);
   if (task) xml_free (task);
   if (list) xml_free (list);
   if (action) xml_free (action);
}
