#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../user.h"
#include "../localdefs.h"

XML *user;
XML *list;
XML *item;
XML *elem;
XML *directory;

FILE *file;

char sbuf[1024];

int main(int argc, char *argv[])
{
   if (argc < 3) {
      printf("Usage: pdmlist <user> <permission>\n\n");
      exit(1);
   }

   user = user_get(argv[1]);
   if (!user) {
      printf("User %s unknown.\n", argv[1]);
      exit(1);
   }

   sprintf(sbuf, "%sindex.xml", PROCESS_DEFINITION_REPOSITORY);
   file = fopen(sbuf, "r");
   if (!file) {
      directory = xml_create("index");
   } else {
      directory = xml_read(file);
      fclose(file);
   }

   list = user_list(user, "workflow", NULL, argv[2]);

   elem = xml_firstelem(list);
   while (elem) {
      sprintf(sbuf, "index.item[%s]", xml_attrval(elem, "object"));
      item = xml_loc(directory, sbuf);

      if (item) {
         printf("%s:", xml_attrval(elem, "object"));
         printf("%s\n", xml_attrval(item, "title"));
         xml_writecontent(stdout, item);
         printf("\nEOF\n");
      }

      elem = xml_nextelem(elem);
   }

   xml_free(directory);
   xml_free(user);

   exit(0);
}
