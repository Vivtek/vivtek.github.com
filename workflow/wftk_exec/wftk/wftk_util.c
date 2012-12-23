#include "wftk.h"
#include <string.h>

main(int argc, char *argv[])
{
   if (argc < 2) {
      printf
          ("Loser!  You need a command!  create, define, complete, reject, or setvalue (for now).\n");
      exit(1);
   }

   if (!strcmp(argv[1], "create")) {
   } else if (!strcmp(argv[1], "define")) {
   } else if (!strcmp(argv[1], "complete")) {
   } else if (!strcmp(argv[1], "reject")) {
   } else if (!strcmp(argv[1], "setvalue")) {
   }
}
