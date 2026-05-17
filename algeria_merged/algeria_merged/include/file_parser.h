#ifndef FILE_PARSER_H
#define FILE_PARSER_H

#include "types.h"

//Date helpers
Date parseDate(const char *s);
int  dateToYear(const char *s);
int  personalityAge(const char *dob, const char *dod);

//Check what type a line is
bool isPersonalityLine(const char *line);
bool isEventLine(const char *line);

//Extract name, dob, dod, definition from a personality line.
//  Format:  Name {DoB-DoD}= definition
bool parsePersonalityLine(const char *line, char *name, char *dob, char *dod, char *def);

//Extract name, date, definition from an event line.
//  Format:  Event name {date}: description
bool parseEventLine(const char *line, char *name, char *date, char *def);

//Rewrite the whole file from a linked list
void saveListToFile(const char *filename, TList *personalities, TList *events);

#endif
