#include <stdio.h>
#include <string.h>
#include "xmlapi.h"
#include "localdefs.h"

XML *directory;
XML *item;
XML *version;
XML *datasheet;

FILE *file;

char sbuf[1024];
char *mark;
char *format;
XML *xml;
XML *holder;

int main(int argc, char *argv[])
{
   if (argc < 2) {
      printf("Usage: pdm <command> [<args>]\n");
      return (1);
   }
   directory = NULL;
   item = NULL;
   version = NULL;

   if (!strcmp(argv[1], "list")) {
      sprintf(sbuf, "%s%s", PROCESS_DEFINITION_REPOSITORY, "index.xml");
      file = fopen(sbuf, "r");
      if (!file) {
         printf("Unable to open directory file %s\n", sbuf);
         return (1);
      }

      directory = xml_read(file);
      fclose(file);
      if (!directory) {
         printf("Corrupt directory file.\n");
         return (1);
      }

      if (argc > 2) {
         format = argv[2];
      } else {
         format = "edit?item=%s";
      }

      item = xml_firstelem(directory);
      while (item) {
         if (!strcmp(item->name, "item")) {
            mark = (char *) xml_attrval(item, "title");
            if (!*mark)
               mark = (char *) xml_attrval(item, "id");
            printf("<li><strong><a href=\"");
            printf(format, xml_attrval(item, "id"));
            printf("\">%s</a></strong><br>\n", mark);
            xml_writecontent(stdout, item);
            printf("<br>\n");
         }
         item = xml_nextelem(item);
      }
   } else if (!strcmp(argv[1], "starter")) {
      if (argc < 3) {
         printf("Missing procdef identifier in starter command.\n");
         return (1);
      }
      sprintf(sbuf, "%s%s.xml", PROCESS_DEFINITION_REPOSITORY, argv[2]);
      file = fopen(sbuf, "r");
      if (!file) {
         printf("Unable to open procdef file %s.\n", sbuf);
         return (1);
      }

      item = xml_read(file);
      fclose(file);

      if (!item) {
         printf("Corrupt procdef file %s.\n", sbuf);
         return (1);
      }

      sprintf(sbuf, "%s%s_%s.xml", PROCESS_DEFINITION_REPOSITORY, argv[2],
              xml_attrval(item, "ver"));
      file = fopen(sbuf, "r");
      if (!file) {
         printf("Unable to open procdef version file %s.\n", sbuf);
         return (1);
      }

      version = xml_read(file);
      fclose(file);

      if (!version) {
         printf("Corrupt version file %s.\n", sbuf);
         return (1);
      }

      if (strcmp(xml_attrval(version, "name"), "")) {
         printf("%s\n", xml_attrval(version, "name"));
      } else {
         printf("%s\n", argv[2]);
      }
      printf("%s\n", xml_attrval(item, "ver"));

      xml = xml_firstelem(version);
      while (xml) {
         if (!strcmp(xml->name, "data")) {
            printf("<tr><td>%s</td>\n", xml_attrval(xml, "name"));
            printf("<td>");
            if (!strcmp(xml_attrval(xml, "type"), "text")) {
               printf("<textarea name=\"%s\" rows=5 cols=40>",
                      xml_attrval(xml, "name"));
               xml_writecontent(stdout, xml);
               printf("</textarea>\n");
            } else {
               printf("<input name=\"%s\" value=\"",
                      xml_attrval(xml, "name"));
               xml_writecontent(stdout, xml);
               printf("\">\n");
            }
            printf("</td></tr>\n");
         } else if (!strcmp(xml->name, "sequence")) {
            break;
         } else if (!strcmp(xml->name, "parallel")) {
            break;
         } else if (!strcmp(xml->name, "task")) {
            break;
         }

         xml = xml_nextelem(xml);
      }
   } else if (!strcmp(argv[1], "datasheet")) {
      if (argc < 5) {
         printf("Missing arguments in datasheet command.\n");
         return (1);
      }

      sprintf(sbuf, "%s%s_%s.xml", PROCESS_DEFINITION_REPOSITORY, argv[2],
              argv[3]);
      file = fopen(sbuf, "r");
      if (!file) {
         printf("Unable to open procdef version file %s.\n", sbuf);
         return (1);
      }

      version = xml_read(file);
      fclose(file);

      if (!version) {
         printf("Corrupt version file %s.\n", sbuf);
         return (1);
      }

      datasheet = xml_create("datasheet");
      sprintf(sbuf, "%s_%s.xml", argv[2], argv[3]);
      xml_set(datasheet, "procdef", sbuf);
      xml_set(datasheet, "process", argv[4]);

      xml = xml_firstelem(version);
      while (xml) {
         if (!strcmp(xml->name, "data")) {
            holder = xml_create("data");
            xml_set(holder, "id", xml_attrval(xml, "name"));
            xml_set(holder, "type", xml_attrval(xml, "type"));
            xml_append(datasheet, holder);
         } else if (!strcmp(xml->name, "sequence")) {
            break;
         } else if (!strcmp(xml->name, "parallel")) {
            break;
         } else if (!strcmp(xml->name, "task")) {
            break;
         }

         xml = xml_nextelem(xml);
      }

      xml_write(stdout, datasheet);
   } else if (!strcmp(argv[1], "editor")) {
   } else {
      printf("Unknown command '%s'\n", argv[1]);
   }

   if (directory)
      xml_free(directory);
   if (item)
      xml_free(item);
   if (version)
      xml_free(version);

   return (0);
}
