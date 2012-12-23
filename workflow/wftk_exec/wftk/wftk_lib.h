#define DSREP         0
#define PDREP         1
#define USER          2
#define DB            3
#define NOTIFY        4
#define SCRIPT        5
#define DATATYPE      6
#define DEBUG         7
#define CONFIG        8

typedef struct _wftk_adaptor WFTK_ADAPTOR;
struct _wftk_adaptor {
   int dummy;
};

WFTK_ADAPTOR * wftk_get_adaptor (int type, const char * name);
XML * wftk_call_adaptor (WFTK_ADAPTOR * ad, const char * funcname, ...);
