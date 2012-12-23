#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <errno.h>
#include <windows.h> /* Boy, *this* feels weird! */
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
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

XML * TASKINDEX_odbc_init (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_odbc_free (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_odbc_info (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_odbc_procnew (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_odbc_procdel (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_odbc_procget (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_odbc_procput (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_odbc_proclist (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_odbc_proccomplete (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_odbc_procerror (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_odbc_tasknew (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_odbc_taskdel (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_odbc_taskget (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_odbc_taskput (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_odbc_tasklist (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_odbc_taskcomplete (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_odbc_taskreject (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_odbc_reqnew (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_odbc_reqdel (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_odbc_reqget (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_odbc_reqput (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_odbc_reqlist (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_odbc_reqaccept (WFTK_ADAPTOR * ad, va_list args);
XML * TASKINDEX_odbc_reqdecline (WFTK_ADAPTOR * ad, va_list args);

static WFTK_API_FUNC vtab[] = 
{
   TASKINDEX_odbc_init,
   TASKINDEX_odbc_free,
   TASKINDEX_odbc_info,
   TASKINDEX_odbc_procnew,
   TASKINDEX_odbc_procdel,
   TASKINDEX_odbc_procget,
   TASKINDEX_odbc_procput,
   TASKINDEX_odbc_proclist,
   TASKINDEX_odbc_proccomplete,
   TASKINDEX_odbc_procerror,
   TASKINDEX_odbc_tasknew,
   TASKINDEX_odbc_taskdel,
   TASKINDEX_odbc_taskget,
   TASKINDEX_odbc_taskput,
   TASKINDEX_odbc_tasklist,
   TASKINDEX_odbc_taskcomplete,
   TASKINDEX_odbc_taskreject,
   TASKINDEX_odbc_reqnew,
   TASKINDEX_odbc_reqdel,
   TASKINDEX_odbc_reqget,
   TASKINDEX_odbc_reqput,
   TASKINDEX_odbc_reqlist,
   TASKINDEX_odbc_reqaccept,
   TASKINDEX_odbc_reqdecline
};

static struct adaptor_info _TASKINDEX_odbc_info =
{
   24,
   names,
   vtab
};
struct adaptor_info * TASKINDEX_odbc_get_info ()
{
   return & _TASKINDEX_odbc_info;
}
struct _TASKINDEX_odbc_handles {
   SQLHENV henv;
   SQLHDBC hdbc;
   int ok;
};
XML * TASKINDEX_odbc_init (WFTK_ADAPTOR * ad, va_list args) {
   SQLRETURN rslt;
   const char * parms;
   char spec[256];
   char buf[1024];
   SQLSMALLINT junk;
   struct _TASKINDEX_odbc_handles *handles = (struct _TASKINDEX_odbc_handles *) malloc (sizeof (struct _TASKINDEX_odbc_handles));

   if (!handles) {
      xml_set (ad->parms, "error", "Unable to allocate binary stash.");
      return (XML *) 0;
   }
   ad->bindata = (void *) handles;
   handles->henv = SQL_NULL_HANDLE;
   handles->hdbc = SQL_NULL_HANDLE;
   handles->ok = 0;

   parms = xml_attrval (ad->parms, "parm");
   if (!*parms) parms = config_get_value (ad->session, "taskindex.odbc.default");
   if (!*parms) {
      xml_set (ad->parms, "error", "No connection specified and no default connection configured.");
      return (XML *) 0;
   }

   strcpy (spec, "odbc:");
   strcat (spec, parms);
   xml_set (ad->parms, "spec", spec);
   if (strchr (parms, '=')) {
      xml_set (ad->parms, "conn", parms);
   } else {
      sprintf (buf, "taskindex.odbc.%s.conn", parms);
      xml_set (ad->parms, "conn", config_get_value (ad->session, buf));
   }

   /* OK, let's connect to this database specified. */
   rslt = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &(handles->henv));  /* My first C ODBC statement ever! */
   if (rslt == SQL_ERROR) {
      xml_set (ad->parms, "error", "Unable to allocate ODBC environment handle.");
      return (XML *) 0;
   }
   SQLSetEnvAttr (handles->henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC2, 0);

   rslt = SQLAllocHandle (SQL_HANDLE_DBC, handles->henv, &(handles->hdbc));
   if (rslt == SQL_ERROR) {
      xml_set (ad->parms, "error", "Unable to allocate ODBC connection handle.");
      return (XML *) 0;
   }

   rslt = SQLDriverConnect (handles->hdbc, NULL, (char *) xml_attrval (ad->parms, "conn"), (SQLSMALLINT) strlen (xml_attrval (ad->parms, "conn")), buf, (SQLSMALLINT) sizeof(buf), &junk, SQL_DRIVER_NOPROMPT);
   if (rslt == SQL_ERROR) {
      xml_set (ad->parms, "error", "Unable to connect to database specified.");
      return (XML *) 0;
   }

   handles->ok = 1;   
   return (XML *) 0;
}
XML * TASKINDEX_odbc_free (WFTK_ADAPTOR * ad, va_list args) {
   struct _TASKINDEX_odbc_handles * handles = (struct _TASKINDEX_odbc_handles *) (ad->bindata);
   if (handles) {
      SQLDisconnect (handles->hdbc);
      SQLFreeHandle (SQL_HANDLE_DBC, handles->hdbc);
      SQLFreeHandle (SQL_HANDLE_ENV, handles->henv);
      free (handles);
      ad->bindata = (void *) 0;
   }
   return (XML *) 0;
}
XML * TASKINDEX_odbc_info (WFTK_ADAPTOR * ad, va_list args) {
   XML * info;

   info = xml_create ("info");
   xml_set (info, "type", "taskindex");
   xml_set (info, "name", "odbc");
   xml_set (info, "ver", "1.0.0");
   xml_set (info, "compiled", __TIMESTAMP__);
   xml_set (info, "author", "Michael Roberts");
   xml_set (info, "contact", "wftk@vivtek.com");
   xml_set (info, "extra_functions", "0");

   return (info);
}
void _TASKINDEX_odbc_builderrmsg (XML * ap, SQLHSTMT stmt)
{
   char buf[1024];
   char msg[128];
   SQLSMALLINT len;
   char code[16];
   SQLINTEGER native;

   SQLGetDiagRec (SQL_HANDLE_STMT, stmt, 1, code, &native, msg, sizeof(msg), &len);
   sprintf (buf, "%s[%s %d] %s", xml_attrval (ap, "error"), code, native, msg);
   xml_set (ap, "error", buf);
}
XML * TASKINDEX_odbc_procnew (WFTK_ADAPTOR * ad, va_list args)
{
   struct _TASKINDEX_odbc_handles *handles = (struct _TASKINDEX_odbc_handles *) ad->bindata;
   XML * datasheet = (XML *) 0;
   SQLHSTMT stmt;
   SQLRETURN rslt;
   char * id;      int id_len;
   char * label;   int label_len;
   char * status;  int status_len;

   if (!handles) return (XML *) 0;
   if (!handles->ok) return (XML *) 0;
   if (args) datasheet = va_arg (args, XML *);
   if (!datasheet) {
      xml_set (ad->parms, "error", "No process given.");
      return (XML *) 0;
   }

   rslt = SQLAllocHandle (SQL_HANDLE_STMT, handles->hdbc, &stmt);
   if (rslt != SQL_ERROR) {
      id = (char *) xml_attrval (datasheet, "id");
      id_len = strlen (id);
      label = (char *) xml_attrval (datasheet, "label");
      label_len = strlen (label);
      status = (char *) xml_attrval (datasheet, "status");
      if (!*status) status = "active"; /* Little sneaky here, eh? */
      status_len = strlen (status);

      SQLBindParameter (stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, id_len, 0, id, id_len, &id_len);
      SQLBindParameter (stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, label_len, 0, label, label_len, &label_len);
      SQLBindParameter (stmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, status_len, 0, status, status_len, &status_len);

      rslt = SQLExecDirect (stmt, "insert into process (id, title, status, started) values (?, ?, ?, {fn NOW()});", SQL_NTS);
      if (rslt == SQL_ERROR) {
         xml_set (ad->parms, "error", "SQL error inserting process ");
         _TASKINDEX_odbc_builderrmsg (ad->parms, stmt);
      }
      SQLFreeHandle (SQL_HANDLE_STMT, stmt);
   }
   return (XML *) 0;
}
XML * TASKINDEX_odbc_procdel (WFTK_ADAPTOR * ad, va_list args)
{
   struct _TASKINDEX_odbc_handles *handles = (struct _TASKINDEX_odbc_handles *) ad->bindata;
   char *id;
   SQLHSTMT stmt;
   SQLRETURN rslt;
   int id_len;

   if (!handles) return (XML *) 0;
   if (!handles->ok) return (XML *) 0;
   if (args) id = va_arg (args, char *);
   if (!id) {
      xml_set (ad->parms, "error", "No process given.");
      return (XML *) 0;
   }

   rslt = SQLAllocHandle (SQL_HANDLE_STMT, handles->hdbc, &stmt);
   if (rslt != SQL_ERROR) {
      id_len = strlen (id);

      SQLBindParameter (stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, id_len, 0, id, id_len, &id_len);

      rslt = SQLExecDirect (stmt, "delete from process where id=?;", SQL_NTS);
      if (rslt == SQL_ERROR) {
         xml_set (ad->parms, "error", "SQL error deleting process ");
         _TASKINDEX_odbc_builderrmsg (ad->parms, stmt);
      }
      SQLFreeHandle (SQL_HANDLE_STMT, stmt);
   }
   return (XML *) 0;
}
XML * TASKINDEX_odbc_procget (WFTK_ADAPTOR * ad, va_list args) { return (XML *) 0; }
XML * TASKINDEX_odbc_procput (WFTK_ADAPTOR * ad, va_list args)
{
   struct _TASKINDEX_odbc_handles *handles = (struct _TASKINDEX_odbc_handles *) ad->bindata;
   XML * datasheet = (XML *) 0;
   SQLHSTMT stmt;
   SQLRETURN rslt;
   int already_there = 0;
   char * id;      int id_len;
   char * label;   int label_len;
   char * status;  int status_len;

   if (!handles) return (XML *) 0;
   if (!handles->ok) return (XML *) 0;
   if (args) datasheet = va_arg (args, XML *);
   if (!datasheet) {
      xml_set (ad->parms, "error", "No process given.");
      return (XML *) 0;
   }

   rslt = SQLAllocHandle (SQL_HANDLE_STMT, handles->hdbc, &stmt);
   if (rslt == SQL_ERROR) {
      xml_set (ad->parms, "error", "Unable to check existence of process");
   }
   id = (char *) xml_attrval (datasheet, "id");
   id_len = strlen (id);
   label = (char *) xml_attrval (datasheet, "label");
   label_len = strlen (label);
   status = (char *) xml_attrval (datasheet, "status");
   if (!*status) status = "active"; /* Little sneaky, eh? */
   status_len = strlen (status);

   SQLBindParameter (stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, id_len, 0, id, id_len, &id_len);
   rslt = SQLExecDirect (stmt, "select * from process where id=?;", SQL_NTS);
   if (rslt == SQL_ERROR) {
      xml_set (ad->parms, "error", "SQL error checking process existence ");
      _TASKINDEX_odbc_builderrmsg (ad->parms, stmt);
   }
   if (SQL_SUCCESS == SQLFetch (stmt)) already_there = 1;
   SQLFreeHandle (SQL_HANDLE_STMT, stmt);

   rslt = SQLAllocHandle (SQL_HANDLE_STMT, handles->hdbc, &stmt);
   if (rslt != SQL_ERROR) {
      SQLBindParameter (stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, label_len, 0, label, label_len, &label_len);
      SQLBindParameter (stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, status_len, 0, status, status_len, &status_len);
      SQLBindParameter (stmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, id_len, 0, id, id_len, &id_len);

      if (already_there == 0) {
         rslt = SQLExecDirect (stmt, "insert into process (title, status, id, started) values (?, ?, ?, {fn NOW()});", SQL_NTS);
      } else {
         rslt = SQLExecDirect (stmt, "update process set title=?, status=? where id=?;", SQL_NTS);
      }
      if (rslt == SQL_ERROR) {
         if (already_there == 0) {
            xml_set (ad->parms, "error", "SQL error inserting process ");
         } else {
            xml_set (ad->parms, "error", "SQL error updating process ");
         }
         _TASKINDEX_odbc_builderrmsg (ad->parms, stmt);
      }
      SQLFreeHandle (SQL_HANDLE_STMT, stmt);
   }
   return (XML *) 0;
}
XML * TASKINDEX_odbc_proclist (WFTK_ADAPTOR * ad, va_list args) {
   struct _TASKINDEX_odbc_handles *handles = (struct _TASKINDEX_odbc_handles *) ad->bindata;
   XML * list = (XML *) 0;
   XML * proc;
   SQLHSTMT stmt;
   SQLRETURN rslt;
   char * status;  int status_len;
   char * user;    int user_len;
   char query[1024];
   char column[64];  SQLSMALLINT column_len;
   char value[256];  long value_len;
   SQLUSMALLINT binding = 1;
   SQLSMALLINT numcols;
   SQLSMALLINT col;
   int count = 0;

   if (!handles) return (XML *) 0;
   if (!handles->ok) return (XML *) 0;
   if (args) list = va_arg (args, XML *);
   if (!list) {
      xml_set (ad->parms, "error", "No list specified.");
      return (XML *) 0;
   }
   rslt = SQLAllocHandle (SQL_HANDLE_STMT, handles->hdbc, &stmt);
   if (rslt != SQL_ERROR) {
      strcpy (query, "select * from process where ");

      status = (char *) xml_attrval (list, "status");
      if (*status) {
         status_len = strlen (status);
         /* if (binding > 1) strcat (query, "and "); */
         strcat (query, "status=? ");
         SQLBindParameter (stmt, binding++, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, status_len, 0, status, status_len, &status_len);
      }

      user = (char *) xml_attrval (list, "user");
      if (*user) {
         user_len = strlen (user);
         if (binding > 1) strcat (query, "and ");
         strcat (query, "owner=? ");
         SQLBindParameter (stmt, binding++, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, user_len, 0, user, user_len, &user_len);
      }

      if (*xml_attrval (list, "where")) {
         if (binding > 1) strcat (query, "and ");
         strcat (query, xml_attrval (list, "where"));
         binding++;
      }

      if (binding == 1) {
         strcpy (query, "select * from process");
      }

      rslt = SQLExecDirect (stmt, query, SQL_NTS);
      if (rslt == SQL_ERROR) {
         xml_set (ad->parms, "error", "SQL error listing processes ");
         _TASKINDEX_odbc_builderrmsg (ad->parms, stmt);
      } else {
         /* Here's where we build the list. */
         SQLNumResultCols (stmt, &numcols);
         while (SQL_SUCCESS == SQLFetch (stmt)) {
            proc = xml_create ("process");
            count++;
            for (col=1; col < numcols; col++) {
               SQLColAttribute (stmt, col, SQL_DESC_NAME, column, sizeof(column), &column_len, NULL);
               strlwr (column);
               if (!strcmp (column, "owner")) strcpy (column, "user");
               SQLGetData (stmt, col, SQL_C_CHAR, value, sizeof(value), &value_len);
               xml_set (proc, column, value);
            }
            xml_append (list, proc);
         }
      }

      xml_setnum (list, "count", count);
      SQLFreeHandle (SQL_HANDLE_STMT, stmt);
   } else {
      xml_set (ad->parms, "error", "Unable to allocate statement handle.");
   }
   return (XML *) 0;
}
XML * TASKINDEX_odbc_proccomplete (WFTK_ADAPTOR * ad, va_list args)
{
   struct _TASKINDEX_odbc_handles *handles = (struct _TASKINDEX_odbc_handles *) ad->bindata;
   char *id;
   SQLHSTMT stmt;
   SQLRETURN rslt;
   int id_len;

   if (!handles) return (XML *) 0;
   if (!handles->ok) return (XML *) 0;
   if (args) id = va_arg (args, char *);
   if (!id) {
      xml_set (ad->parms, "error", "No process given.");
      return (XML *) 0;
   }

   rslt = SQLAllocHandle (SQL_HANDLE_STMT, handles->hdbc, &stmt);
   if (rslt != SQL_ERROR) {
      id_len = strlen (id);

      SQLBindParameter (stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, id_len, 0, id, id_len, &id_len);

      rslt = SQLExecDirect (stmt, "update process set status='complete' where id=?;", SQL_NTS);
      if (rslt == SQL_ERROR) {
         xml_set (ad->parms, "error", "SQL error marking process complete ");
         _TASKINDEX_odbc_builderrmsg (ad->parms, stmt);
      }
      SQLFreeHandle (SQL_HANDLE_STMT, stmt);
   }
   return (XML *) 0;
}
XML * TASKINDEX_odbc_procerror (WFTK_ADAPTOR * ad, va_list args)
{
   struct _TASKINDEX_odbc_handles *handles = (struct _TASKINDEX_odbc_handles *) ad->bindata;
   char *id;
   SQLHSTMT stmt;
   SQLRETURN rslt;
   int id_len;

   if (!handles) return (XML *) 0;
   if (!handles->ok) return (XML *) 0;
   if (args) id = va_arg (args, char *);
   if (!id) {
      xml_set (ad->parms, "error", "No process given.");
      return (XML *) 0;
   }

   rslt = SQLAllocHandle (SQL_HANDLE_STMT, handles->hdbc, &stmt);
   if (rslt != SQL_ERROR) {
      id_len = strlen (id);

      SQLBindParameter (stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, id_len, 0, id, id_len, &id_len);

      rslt = SQLExecDirect (stmt, "update process set status='error' where id=?;", SQL_NTS);
      if (rslt == SQL_ERROR) {
         xml_set (ad->parms, "error", "SQL error setting process status to error (ironic) ");
         _TASKINDEX_odbc_builderrmsg (ad->parms, stmt);
      }
      SQLFreeHandle (SQL_HANDLE_STMT, stmt);
   }
   return (XML *) 0;
}
XML * TASKINDEX_odbc_tasknew (WFTK_ADAPTOR * ad, va_list args)
{
   struct _TASKINDEX_odbc_handles *handles = (struct _TASKINDEX_odbc_handles *) ad->bindata;
   XML * task = (XML *) 0;
   SQLHSTMT stmt;
   SQLRETURN rslt;
   char * process; int process_len;
   char * id;      int id_len;
   char * label;   int label_len;
   char * role;    int role_len;
   char * user;    int user_len;

   if (!handles) return (XML *) 0;
   if (!handles->ok) return (XML *) 0;
   if (args) task = va_arg (args, XML *);
   if (!task) {
      xml_set (ad->parms, "error", "No task given.");
      return (XML *) 0;
   }
   rslt = SQLAllocHandle (SQL_HANDLE_STMT, handles->hdbc, &stmt);
   if (rslt != SQL_ERROR) {
      process = (char *) xml_attrval (task, "process");
      process_len = strlen (process);
      id = (char *) xml_attrval (task, "id");
      id_len = strlen (id);
      label = (char *) xml_attrval (task, "label");
      label_len = strlen (label);
      role = (char *) xml_attrval (task, "role");
      role_len = strlen (role);
      user = (char *) xml_attrval (task, "user");
      user_len = strlen (user);

      SQLBindParameter (stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, id_len, 0, id, id_len, &id_len);
      SQLBindParameter (stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, process_len, 0, process, process_len, &process_len); 
      SQLBindParameter (stmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, label_len, 0, label, label_len, &label_len);
      SQLBindParameter (stmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, role_len, 0, role, role_len, &role_len);
      SQLBindParameter (stmt, 5, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, user_len, 0, user, user_len, &user_len);

      rslt = SQLExecDirect (stmt, "insert into task (id, process, status, description, role, owner, created) values (?, ?, 'active', ?, ?, ?, {fn NOW()});", SQL_NTS);
      if (rslt == SQL_ERROR) {
         xml_set (ad->parms, "error", "SQL error inserting task ");
         _TASKINDEX_odbc_builderrmsg (ad->parms, stmt);
      }
      SQLFreeHandle (SQL_HANDLE_STMT, stmt);
   }
   return (XML *) 0;
}
XML * TASKINDEX_odbc_taskdel (WFTK_ADAPTOR * ad, va_list args)
{
   struct _TASKINDEX_odbc_handles *handles = (struct _TASKINDEX_odbc_handles *) ad->bindata;
   XML * task = (XML *) 0;
   SQLHSTMT stmt;
   SQLRETURN rslt;

   char *process;
   char *id;
   int process_len, id_len;

   if (!handles) return (XML *) 0;
   if (!handles->ok) return (XML *) 0;
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
   rslt = SQLAllocHandle (SQL_HANDLE_STMT, handles->hdbc, &stmt);
   if (rslt != SQL_ERROR) {
      process_len = strlen (process);
      id_len = strlen (id);

      SQLBindParameter (stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, process_len, 0, process, process_len, &process_len); 
      SQLBindParameter (stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, id_len, 0, id, id_len, &id_len);

      rslt = SQLExecDirect (stmt, "delete from task where process=? and id=?;", SQL_NTS);
      if (rslt == SQL_ERROR) {
         xml_set (ad->parms, "error", "SQL error deleting task ");
         _TASKINDEX_odbc_builderrmsg (ad->parms, stmt);
      }
      SQLFreeHandle (SQL_HANDLE_STMT, stmt);
   }
   return (XML *) 0;
}
XML * TASKINDEX_odbc_taskget (WFTK_ADAPTOR * ad, va_list args) { return (XML *) 0; }
XML * TASKINDEX_odbc_taskput (WFTK_ADAPTOR * ad, va_list args)
{
   struct _TASKINDEX_odbc_handles *handles = (struct _TASKINDEX_odbc_handles *) ad->bindata;
   XML * task = (XML *) 0;
   SQLHSTMT stmt;
   SQLRETURN rslt;
   char * process; int process_len;
   char * id;      int id_len;
   char * label;   int label_len;
   char * role;    int role_len;
   char * user;    int user_len;

   if (!handles) return (XML *) 0;
   if (!handles->ok) return (XML *) 0;
   if (args) task = va_arg (args, XML *);
   if (!task) {
      xml_set (ad->parms, "error", "No task given.");
      return (XML *) 0;
   }
   rslt = SQLAllocHandle (SQL_HANDLE_STMT, handles->hdbc, &stmt);
   if (rslt != SQL_ERROR) {
      process = (char *) xml_attrval (task, "process");
      process_len = strlen (process);
      id = (char *) xml_attrval (task, "id");
      id_len = strlen (id);
      label = (char *) xml_attrval (task, "label");
      label_len = strlen (label);
      role = (char *) xml_attrval (task, "role");
      role_len = strlen (role);
      user = (char *) xml_attrval (task, "user");
      user_len = strlen (user);

      SQLBindParameter (stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, label_len, 0, label, label_len, &label_len);
      SQLBindParameter (stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, role_len, 0, role, role_len, &role_len);
      SQLBindParameter (stmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, user_len, 0, user, user_len, &user_len);
      SQLBindParameter (stmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, process_len, 0, process, process_len, &process_len); 
      SQLBindParameter (stmt, 5, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, id_len, 0, id, id_len, &id_len);

      rslt = SQLExecDirect (stmt, "update task set description=?, role=?, owner=? where process=? and id=?", SQL_NTS);
      if (rslt == SQL_ERROR) {
         xml_set (ad->parms, "error", "SQL error inserting task ");
         _TASKINDEX_odbc_builderrmsg (ad->parms, stmt);
      }
      SQLFreeHandle (SQL_HANDLE_STMT, stmt);
   }
   return (XML *) 0;
}
XML * TASKINDEX_odbc_tasklist (WFTK_ADAPTOR * ad, va_list args) {
   struct _TASKINDEX_odbc_handles *handles = (struct _TASKINDEX_odbc_handles *) ad->bindata;
   XML * list = (XML *) 0;
   XML * task;
   SQLHSTMT stmt;
   SQLRETURN rslt;
   char * process; int process_len;
   char * status;  int status_len;
   char * role;    int role_len;
   char * user;    int user_len;
   char query[1024];
   char column[64];  SQLSMALLINT column_len;
   char value[256];  long value_len;
   SQLUSMALLINT binding = 1;
   SQLSMALLINT numcols;
   SQLSMALLINT col;
   int count = 0;

   if (!handles) return (XML *) 0;
   if (!handles->ok) return (XML *) 0;
   if (args) list = va_arg (args, XML *);
   if (!list) {
      xml_set (ad->parms, "error", "No list specified.");
      return (XML *) 0;
   }
   rslt = SQLAllocHandle (SQL_HANDLE_STMT, handles->hdbc, &stmt);
   if (rslt != SQL_ERROR) {
      strcpy (query, "select * from task where ");

      process = (char *) xml_attrval (list, "process");
      if (*process) {
         process_len = strlen (process);
         /* if (binding > 1) strcat (query, "and "); */
         strcat (query, "process=? ");
         SQLBindParameter (stmt, binding++, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, process_len, 0, process, process_len, &process_len);
      }

      status = (char *) xml_attrval (list, "status");
      if (*status) {
         status_len = strlen (status);
         /* if (binding > 1) strcat (query, "and "); */
         strcat (query, "status=? ");
         SQLBindParameter (stmt, binding++, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, status_len, 0, status, status_len, &status_len);
      }

      role = (char *) xml_attrval (list, "role");
      if (*role) {
         role_len = strlen (role);
         if (binding > 1) strcat (query, "and ");
         strcat (query, "role=? ");
         SQLBindParameter (stmt, binding++, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, role_len, 0, role, role_len, &role_len);
      }

      user = (char *) xml_attrval (list, "user");
      if (*user) {
         user_len = strlen (user);
         if (binding > 1) strcat (query, "and ");
         strcat (query, "owner=? ");
         SQLBindParameter (stmt, binding++, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, user_len, 0, user, user_len, &user_len);
      }

      if (*xml_attrval (list, "where")) {
         if (binding > 1) strcat (query, "and ");
         strcat (query, xml_attrval (list, "where"));
         binding++;
      }

      if (binding == 1) {
         strcpy (query, "select * from task");
      }

      rslt = SQLExecDirect (stmt, query, SQL_NTS);
      if (rslt == SQL_ERROR) {
         xml_set (ad->parms, "error", "SQL error listing tasks ");
         _TASKINDEX_odbc_builderrmsg (ad->parms, stmt);
      } else {
         /* Here's where we build the list. */
         SQLNumResultCols (stmt, &numcols);
         while (SQL_SUCCESS == SQLFetch (stmt)) {
            task = xml_create ("task");
            count++;
            for (col=1; col < numcols; col++) {
               SQLColAttribute (stmt, col, SQL_DESC_NAME, column, sizeof(column), &column_len, NULL);
               strlwr (column);
               if (!strcmp (column, "description")) strcpy (column, "label");
               if (!strcmp (column, "owner")) strcpy (column, "user");
               SQLGetData (stmt, col, SQL_C_CHAR, value, sizeof(value), &value_len);
               xml_set (task, column, value);
            }
            xml_append (list, task);
         }
      }

      xml_setnum (list, "count", count);
      SQLFreeHandle (SQL_HANDLE_STMT, stmt);
   } else {
      xml_set (ad->parms, "error", "Unable to allocate statement handle.");
   }
   return (XML *) 0;
}
XML * TASKINDEX_odbc_taskcomplete (WFTK_ADAPTOR * ad, va_list args)
{
   struct _TASKINDEX_odbc_handles *handles = (struct _TASKINDEX_odbc_handles *) ad->bindata;
   XML * task = (XML *) 0;
   SQLHSTMT stmt;
   SQLRETURN rslt;

   char *process = NULL;
   char *id = NULL;
   int process_len, id_len;

   if (!handles) return (XML *) 0;
   if (!handles->ok) return (XML *) 0;
   if (args) process = va_arg (args, char *);
   if (!process) process = "";
   if (args) id = va_arg (args, char *);
   if (!id) {
      xml_set (ad->parms, "error", "No task given.");
      return (XML *) 0;
   }
   rslt = SQLAllocHandle (SQL_HANDLE_STMT, handles->hdbc, &stmt);
   if (rslt != SQL_ERROR) {
      process_len = strlen (process);
      id_len = strlen (id);

      SQLBindParameter (stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, process_len, 0, process, process_len, &process_len); 
      SQLBindParameter (stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, id_len, 0, id, id_len, &id_len);

      rslt = SQLExecDirect (stmt, "update task set status='complete', complete={fn NOW()} where process=? and id=? and status='active';", SQL_NTS);
      if (rslt == SQL_ERROR) {
         xml_set (ad->parms, "error", "SQL error updating task as complete ");
         _TASKINDEX_odbc_builderrmsg (ad->parms, stmt);
      }
      SQLFreeHandle (SQL_HANDLE_STMT, stmt);
   }
   return (XML *) 0;
}
XML * TASKINDEX_odbc_taskreject (WFTK_ADAPTOR * ad, va_list args)
{
   struct _TASKINDEX_odbc_handles *handles = (struct _TASKINDEX_odbc_handles *) ad->bindata;
   XML * task = (XML *) 0;
   SQLHSTMT stmt;
   SQLRETURN rslt;

   char *process = NULL;
   char *id = NULL;
   int process_len, id_len;

   if (!handles) return (XML *) 0;
   if (!handles->ok) return (XML *) 0;
   if (args) process = va_arg (args, char *);
   if (!process) process = "";
   if (args) id = va_arg (args, char *);
   if (!id) {
      xml_set (ad->parms, "error", "No task given.");
      return (XML *) 0;
   }
   rslt = SQLAllocHandle (SQL_HANDLE_STMT, handles->hdbc, &stmt);
   if (rslt != SQL_ERROR) {
      process_len = strlen (process);
      id_len = strlen (id);

      SQLBindParameter (stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, process_len, 0, process, process_len, &process_len); 
      SQLBindParameter (stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, id_len, 0, id, id_len, &id_len);

      rslt = SQLExecDirect (stmt, "update task set status='reject' where process=? and id=? and status='active';", SQL_NTS);
      if (rslt == SQL_ERROR) {
         xml_set (ad->parms, "error", "SQL error updating task as complete ");
         _TASKINDEX_odbc_builderrmsg (ad->parms, stmt);
      }
      SQLFreeHandle (SQL_HANDLE_STMT, stmt);
   }
   return (XML *) 0;
}
XML * TASKINDEX_odbc_reqnew (WFTK_ADAPTOR * ad, va_list args)
{
   struct _TASKINDEX_odbc_handles *handles = (struct _TASKINDEX_odbc_handles *) ad->bindata;
   XML * task = (XML *) 0;
   SQLHSTMT stmt;
   SQLRETURN rslt;
   char * process; int process_len;
   char * id;      int id_len;
   char * label;   int label_len;
   char * role;    int role_len;
   char * user;    int user_len;

   if (!handles) return (XML *) 0;
   if (!handles->ok) return (XML *) 0;
   if (args) task = va_arg (args, XML *);
   if (!task) {
      xml_set (ad->parms, "error", "No request given.");
      return (XML *) 0;
   }
   rslt = SQLAllocHandle (SQL_HANDLE_STMT, handles->hdbc, &stmt);
   if (rslt != SQL_ERROR) {
      process = (char *) xml_attrval (task, "process");
      process_len = strlen (process);
      id = (char *) xml_attrval (task, "id");
      id_len = strlen (id);
      label = (char *) xml_attrval (task, "label");
      label_len = strlen (label);
      user = (char *) xml_attrval (task, "of");
      user_len = strlen (user);

      SQLBindParameter (stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, id_len, 0, id, id_len, &id_len);
      SQLBindParameter (stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, process_len, 0, process, process_len, &process_len); 
      SQLBindParameter (stmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, label_len, 0, label, label_len, &label_len);
      SQLBindParameter (stmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, user_len, 0, user, user_len, &user_len);

      rslt = SQLExecDirect (stmt, "insert into task (id, process, status, description, owner, created) values (?, ?, 'request', ?, ?, {fn NOW()});", SQL_NTS);
      if (rslt == SQL_ERROR) {
         xml_set (ad->parms, "error", "SQL error inserting request ");
         _TASKINDEX_odbc_builderrmsg (ad->parms, stmt);
      }
      SQLFreeHandle (SQL_HANDLE_STMT, stmt);
   }
   return (XML *) 0;
}
XML * TASKINDEX_odbc_reqdel (WFTK_ADAPTOR * ad, va_list args)
{
   struct _TASKINDEX_odbc_handles *handles = (struct _TASKINDEX_odbc_handles *) ad->bindata;
   XML * task = (XML *) 0;
   SQLHSTMT stmt;
   SQLRETURN rslt;

   char *process = NULL;
   char *id = NULL;
   int process_len, id_len;

   if (!handles) return (XML *) 0;
   if (!handles->ok) return (XML *) 0;
   if (args) process = va_arg (args, char *);
   if (!process) process = "";
   if (args) id = va_arg (args, char *);
   if (!id) {
      xml_set (ad->parms, "error", "No request given.");
      return (XML *) 0;
   }
   rslt = SQLAllocHandle (SQL_HANDLE_STMT, handles->hdbc, &stmt);
   if (rslt != SQL_ERROR) {
      process_len = strlen (process);
      id_len = strlen (id);

      SQLBindParameter (stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, process_len, 0, process, process_len, &process_len); 
      SQLBindParameter (stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, id_len, 0, id, id_len, &id_len);

      rslt = SQLExecDirect (stmt, "delete from task where process=? and id=?;", SQL_NTS);
      if (rslt == SQL_ERROR) {
         xml_set (ad->parms, "error", "SQL error deleting request ");
         _TASKINDEX_odbc_builderrmsg (ad->parms, stmt);
      }
      SQLFreeHandle (SQL_HANDLE_STMT, stmt);
   }
   return (XML *) 0;
}
XML * TASKINDEX_odbc_reqget (WFTK_ADAPTOR * ad, va_list args) { return (XML *) 0; }
XML * TASKINDEX_odbc_reqput (WFTK_ADAPTOR * ad, va_list args)
{
   struct _TASKINDEX_odbc_handles *handles = (struct _TASKINDEX_odbc_handles *) ad->bindata;
   XML * task = (XML *) 0;
   SQLHSTMT stmt;
   SQLRETURN rslt;
   char * process; int process_len;
   char * id;      int id_len;
   char * label;   int label_len;
   char * user;    int user_len;

   if (!handles) return (XML *) 0;
   if (!handles->ok) return (XML *) 0;
   if (args) task = va_arg (args, XML *);
   if (!task) {
      xml_set (ad->parms, "error", "No request given.");
      return (XML *) 0;
   }
   rslt = SQLAllocHandle (SQL_HANDLE_STMT, handles->hdbc, &stmt);
   if (rslt != SQL_ERROR) {
      process = (char *) xml_attrval (task, "process");
      process_len = strlen (process);
      id = (char *) xml_attrval (task, "id");
      id_len = strlen (id);
      label = (char *) xml_attrval (task, "label");
      label_len = strlen (label);
      user = (char *) xml_attrval (task, "of");
      user_len = strlen (user);

      SQLBindParameter (stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, label_len, 0, label, label_len, &label_len);
      SQLBindParameter (stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, user_len, 0, user, user_len, &user_len);
      SQLBindParameter (stmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, process_len, 0, process, process_len, &process_len); 
      SQLBindParameter (stmt, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, id_len, 0, id, id_len, &id_len);

      rslt = SQLExecDirect (stmt, "update task set description=?, owner=? where process=? and id=?", SQL_NTS);
      if (rslt == SQL_ERROR) {
         xml_set (ad->parms, "error", "SQL error inserting task ");
         _TASKINDEX_odbc_builderrmsg (ad->parms, stmt);
      }
      SQLFreeHandle (SQL_HANDLE_STMT, stmt);
   }
   return (XML *) 0;
}
XML * TASKINDEX_odbc_reqlist (WFTK_ADAPTOR * ad, va_list args) {
   struct _TASKINDEX_odbc_handles *handles = (struct _TASKINDEX_odbc_handles *) ad->bindata;
   XML * list = (XML *) 0;
   XML * task;
   SQLHSTMT stmt;
   SQLRETURN rslt;
   char * process; int process_len;
   char * user;    int user_len;
   char query[1024];
   char column[64];  SQLSMALLINT column_len;
   char value[256];  long value_len;
   SQLUSMALLINT binding = 1;
   SQLSMALLINT numcols;
   SQLSMALLINT col;
   int count = 0;

   if (!handles) return (XML *) 0;
   if (!handles->ok) return (XML *) 0;
   if (args) list = va_arg (args, XML *);
   if (!list) {
      xml_set (ad->parms, "error", "No list specified.");
      return (XML *) 0;
   }
   rslt = SQLAllocHandle (SQL_HANDLE_STMT, handles->hdbc, &stmt);
   if (rslt != SQL_ERROR) {
      strcpy (query, "select * from task where status='request' ");

      process = (char *) xml_attrval (list, "process");
      if (*process) {
         process_len = strlen (process);
         strcat (query, "and process=? ");
         SQLBindParameter (stmt, binding++, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, process_len, 0, process, process_len, &process_len);
      }

      user = (char *) xml_attrval (list, "user");
      if (*user) {
         user_len = strlen (user);
         strcat (query, "and owner=? ");
         SQLBindParameter (stmt, binding++, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, user_len, 0, user, user_len, &user_len);
      }

      if (*xml_attrval (list, "where")) {
         strcat (query, "and ");
         strcat (query, xml_attrval (list, "where"));
         binding++;
      }

      rslt = SQLExecDirect (stmt, query, SQL_NTS);
      if (rslt == SQL_ERROR) {
         xml_set (ad->parms, "error", "SQL error listing tasks ");
         _TASKINDEX_odbc_builderrmsg (ad->parms, stmt);
      } else {
         /* Here's where we build the list. */
         SQLNumResultCols (stmt, &numcols);
         while (SQL_SUCCESS == SQLFetch (stmt)) {
            task = xml_create ("task");
            count++;
            for (col=1; col < numcols; col++) {
               SQLColAttribute (stmt, col, SQL_DESC_NAME, column, sizeof(column), &column_len, NULL);
               strlwr (column);
               if (!strcmp (column, "description")) strcpy (column, "label");
               if (!strcmp (column, "owner")) strcpy (column, "user");
               SQLGetData (stmt, col, SQL_C_CHAR, value, sizeof(value), &value_len);
               xml_set (task, column, value);
            }
            xml_append (list, task);
         }
      }

      xml_setnum (list, "count", count);
      SQLFreeHandle (SQL_HANDLE_STMT, stmt);
   } else {
      xml_set (ad->parms, "error", "Unable to allocate statement handle.");
   }
   return (XML *) 0;
}
XML * TASKINDEX_odbc_reqaccept (WFTK_ADAPTOR * ad, va_list args)
{
   struct _TASKINDEX_odbc_handles *handles = (struct _TASKINDEX_odbc_handles *) ad->bindata;
   XML * task = (XML *) 0;
   SQLHSTMT stmt;
   SQLRETURN rslt;

   char *process = NULL;
   char *id = NULL;
   int process_len, id_len;

   if (!handles) return (XML *) 0;
   if (!handles->ok) return (XML *) 0;
   if (args) process = va_arg (args, char *);
   if (!process) process = "";
   if (args) id = va_arg (args, char *);
   if (!id) {
      xml_set (ad->parms, "error", "No request given.");
      return (XML *) 0;
   }
   rslt = SQLAllocHandle (SQL_HANDLE_STMT, handles->hdbc, &stmt);
   if (rslt != SQL_ERROR) {
      process_len = strlen (process);
      id_len = strlen (id);

      SQLBindParameter (stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, process_len, 0, process, process_len, &process_len); 
      SQLBindParameter (stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, id_len, 0, id, id_len, &id_len);

      if (*process) {
         rslt = SQLExecDirect (stmt, "delete from task where process=? and id=? and status='request';", SQL_NTS);
      } else {
         rslt = SQLExecDirect (stmt, "update task set status='active' where process=? and id=? and status='request';", SQL_NTS);
      }
      if (rslt == SQL_ERROR) {
         xml_set (ad->parms, "error", "SQL error accepting request ");
         _TASKINDEX_odbc_builderrmsg (ad->parms, stmt);
      }
      SQLFreeHandle (SQL_HANDLE_STMT, stmt);
   }
   return (XML *) 0;
}
XML * TASKINDEX_odbc_reqdecline (WFTK_ADAPTOR * ad, va_list args)
{
   struct _TASKINDEX_odbc_handles *handles = (struct _TASKINDEX_odbc_handles *) ad->bindata;
   XML * task = (XML *) 0;
   SQLHSTMT stmt;
   SQLRETURN rslt;

   char *process = NULL;
   char *id = NULL;
   int process_len, id_len;

   if (!handles) return (XML *) 0;
   if (!handles->ok) return (XML *) 0;
   if (args) process = va_arg (args, char *);
   if (!process) process = "";
   if (args) id = va_arg (args, char *);
   if (!id) {
      xml_set (ad->parms, "error", "No request given.");
      return (XML *) 0;
   }
   rslt = SQLAllocHandle (SQL_HANDLE_STMT, handles->hdbc, &stmt);
   if (rslt != SQL_ERROR) {
      process_len = strlen (process);
      id_len = strlen (id);

      SQLBindParameter (stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, process_len, 0, process, process_len, &process_len); 
      SQLBindParameter (stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, id_len, 0, id, id_len, &id_len);

      rslt = SQLExecDirect (stmt, "delete task where process=? and id=? and status='request';", SQL_NTS);
      if (rslt == SQL_ERROR) {
         xml_set (ad->parms, "error", "SQL error declining request ");
         _TASKINDEX_odbc_builderrmsg (ad->parms, stmt);
      }
      SQLFreeHandle (SQL_HANDLE_STMT, stmt);
   }
   return (XML *) 0;
}
