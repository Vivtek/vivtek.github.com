#define DSREP         1
#define DATASTORE     2
#define DATATYPE      3
#define PDREP         4
#define USER          5
#define PERMS         6
#define TASKINDEX     7
#define NOTIFY        8
#define ACTION        9
#define DEBUG_MSG    10
#include <stdarg.h>
typedef struct wftk_adaptor WFTK_ADAPTOR;
typedef XML * (*WFTK_API_FUNC) (WFTK_ADAPTOR * ad, va_list args);
struct wftk_adaptor {
   int num;                /* Supplied by the adaptor class. */
   XML * parms;            /* Supplied by the caller. */
   int  nfuncs;            /* Supplied by the adaptor class. */
   char ** names;          /* Supplied by the adaptor class. */
   WFTK_API_FUNC * vtab;   /* Supplied by the adaptor driver. */
   void * bindata;         /* Supplied by the adaptor driver (a general stash pointer.) */
   void * session;         /* Supplied by the config module on allocation. */
};
struct adaptor_info {
   int  nfuncs;
   char ** names;
   WFTK_API_FUNC * vtab;
};
typedef struct wftk_adaptorlist WFTK_ADAPTORLIST;
struct wftk_adaptorlist {
   int count; /* The number of adaptors in this list. */
   WFTK_ADAPTOR * ads[1];
};
const char * config_get_value (void * session, const char *name);
void config_debug_message (char type, const char * message, ...);
#ifdef DEBUG
#define DBG(x,y) config_debug_message (x, y);
#define DBG1(x,y,z) config_debug_message (x, y, z);
#define DBG2(x,y,z,now) config_debug_message (x, y, z, now);
#else
#define DBG(x,y) ;
#define DBG1(x,y,z) ;
#define DBG2(x,y,z,now) ;
#endif
WFTK_ADAPTOR * wftk_get_adaptor (void * session, int type, const char * name);
XML * wftk_call_adaptor (WFTK_ADAPTOR * ad, const char * funcname, ...);
void wftk_free_adaptor (void * session, WFTK_ADAPTOR * ad);
WFTK_ADAPTORLIST * wftk_get_adaptorlist (void * session, int type);
int wftk_call_adaptorlist (WFTK_ADAPTORLIST * ad, const char * funcname, ...);
void wftk_free_adaptorlist (void * session, WFTK_ADAPTORLIST * list);
XML * _procdef_load (void * session, XML * datasheet);
const char * _wftk_value_special (void * session, XML * datasheet, const char * name);
typedef struct wftk_adaptor_list WFTK_ADAPTOR_LIST;
struct wftk_adaptor_list {
   WFTK_ADAPTOR * ad;
   WFTK_ADAPTOR_LIST *next;
};
typedef struct wftk_cache_list WFTK_CACHE_LIST;
struct wftk_cache_list {
   XML * cached;
   WFTK_CACHE_LIST * next;
};
typedef struct wftk_session WFTK_SESSION;
struct wftk_session {
   WFTK_ADAPTOR_LIST *ads;
   XML * user;
   XML * config;
   XML * datasheet;
   XML * procdef;

   XML * values;
   WFTK_CACHE_LIST * cache;
};
