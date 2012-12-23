#include <stdio.h>
#include "xmlapi.h"

XML * xml;

int main (int argc, char *argv[])
{
   xml=  xml_read(stdin);
   xml_prepend (xml, xml_create("boogida"));
   xml_write (stdout, xml);
}

