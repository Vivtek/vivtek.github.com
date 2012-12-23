#ifndef WFTK_H
#define WFTK_H
#include <stdio.h>
#include "../xmlapi.h"

void wftk_htmlout_summary (FILE * out, XML * datasheet);
void wftk_htmlout_active (FILE * out, XML * datasheet);
void wftk_htmlout_task (FILE * out, XML * datasheet);
void wftk_htmlout_progress (FILE * out, XML * datasheet);

XML * wftk_list_users (XML * datasheet);
XML * wftk_list_active (XML * datasheet);
XML * wftk_list_enactment (XML * datasheet);

XML * wftk_task_requirements (XML * datasheet, char * task_id);

XML * wftk_action_create (char * dprep, char * datasheet_id);
int   wftk_action_define (XML * datasheet, char * pdrep, char * procdef_id);
void wftk_action_complete (XML * datasheet, XML * procdef, char * task_id);
void wftk_action_abort (XML * datasheet, XML * procdef, char * task_id);
void wftk_action_setvalue (XML * datasheet, char * name, char * value);

void wftk_action (XML * datasheet, XML * procdef, XML * command);

XML * wftk_datasheet_load (char * dprep, char * datasheet_id);
void  wftk_datasheet_save (XML * datasheet);

XML * wftk_procdef_load (XML * datasheet);

void wftk_notify (const char * type, const char * to, const char * from, const char * subject, XML * message);
#endif
