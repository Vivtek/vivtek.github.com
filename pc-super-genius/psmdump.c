/* This little ditty is in the public domain.  However, it was written
   by me, Michael Roberts, of Vivtek (http://www.vivtek.com), and I will not
   be amused if I find that you removed this header.
*/

/* Oh -- its *purpose*?  It reads the contact info data files from Ruben
   Garcia's PC-Super-MLM program.

   It outputs its findings to stdout (redirectable, of course) in tab-delimited
   format for your viewing pleasure and suitable for import into your favorite
   database.

   If run with no argument, it assumes that psg1.dat is in the current directory.
   Otherwise you may give a path to psg1.dat (or whatever else you may call it.)

   I think that will suffice for documentation.  Questions?  Email me!
   >> michael@vivtek.com <<  01/25/99
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char filename[256];
FILE * file;

long records;    /* The types here are chosen to make sense under DOS. */
int  dataoffset; /* They should work OK under Unix, too, though. */
int  reclen;

int  i, j;

char field[41];

char *mark;

int main(int argc, char *argv[])
{
   if (argc > 2)
   {
      printf ("Usage: psgdump [<file>]\n");
      exit (1);
   }

   if (argc == 1) { strcpy (filename, "psm\xff.com"); } else { strcpy (filename, argv[1]); }

   file = fopen (filename, "rb"); /* The 'b' is required by DOS to open in binary mode; in Unix, should have no effect. */

   fseek (file, 4, SEEK_SET);

   records = fgetc(file) + 256 * fgetc(file) + 256 * 256 * fgetc(file) + 256 * 256 * 256 * fgetc(file);
   dataoffset = fgetc(file) + 256 * fgetc(file);
   reclen = fgetc(file) + 256 * fgetc(file);

   /* All right, let's dump the data! */
   for (i=0; i < records; i++)
   {
      fseek (file, dataoffset + i * reclen + 1, SEEK_SET);

      /* Get those contact fields, each 40 chars in length. */
      for (j=0; j<10; j++)
      {
         readfield (40, 0x64);
         printf ("%s\t", field);
      }
      readfield (25, 0x64); /* Now the "message to your users." */
      printf ("%s\t", field);

      /* Now let's get the other fields in case we want to figure out what they are.
         No trick for these. */
      readfield (4, 0x64);  printf ("%s\t", field);
      readfield (1, 0x64);  printf ("%s\t", field);
      readfield (1, 0x64);  printf ("%s\t", field);
      readfield (4, 0x64);  printf ("%s\t", field);
      readfield (8, 0x64);  printf ("%s\t", field);
      readfield (8, 0x64);  printf ("%s\t", field);
      readfield (8, 0x64);  printf ("%s\t", field);
      readfield (10, 0x64);  printf ("%s\t", field);
      readfield (8, 0);  printf ("%s\t", field);

      printf ("\n");
   }
}


void readfield (int size, int trick) /* Yeah, great trick... */
{
   fread (field, size, 1, file);
   field[size] = '\0';
   mark = field + size - 1;
   while (*mark == ' ') { *mark = '\0'; mark--; } /* Trim the trailing spaces. */

   mark = field;
   while (*mark) { *mark = *mark - trick; mark++; }
}
