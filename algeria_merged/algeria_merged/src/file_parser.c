#include "file_parser.h"

/* ---- Date helpers ---- */

Date parseDate(const char *s) {
    Date d;
    d.day = 0; d.month = 0; d.year = 0;
    if (!s || s[0] == '\0') return d;
    if (sscanf(s, "%d/%d/%d", &d.day, &d.month, &d.year) == 3) return d;
    if (sscanf(s, "%d", &d.year) == 1) return d;
    return d;
}

int dateToYear(const char *s) {
    Date d = parseDate(s);
    return d.year;
}

int personalityAge(const char *dob, const char *dod) {
    int y1 = dateToYear(dob);
    int y2 = dateToYear(dod);
    if (y1 == 0 || y2 == 0) return 0;
    return y2 - y1;
}

bool isPersonalityLine(const char *line) {
    return strstr(line, "=") != NULL && strstr(line, "{") != NULL;
}

bool isEventLine(const char *line) {
    /* has { } and : but no = */
    return strstr(line, ":") != NULL && strstr(line, "{") != NULL
           && strstr(line, "=") == NULL;
}

/* Personality format:  Name {DoB-DoD}= definition
   OR:                  Name {DoB}= definition  (no death date yet)  */
bool parsePersonalityLine(const char *line,
                          char *name, char *dob, char *dod, char *def) {
    name[0] = dob[0] = dod[0] = def[0] = '\0';

    const char *brace = strchr(line, '{');
    const char *close = strchr(line, '}');
    const char *eq    = strchr(line, '=');

    if (!brace || !close || !eq) return false;

    /* name = everything before '{', trimmed */
    int nameLen = (int)(brace - line);
    while (nameLen > 0 && isspace((unsigned char)line[nameLen-1])) nameLen--;
    strncpy(name, line, nameLen);
    name[nameLen] = '\0';

    /* dates inside braces */
    char dates[64] = {0};
    int dLen = (int)(close - brace - 1);
    if (dLen > 0 && dLen < 63) {
        strncpy(dates, brace + 1, dLen);
        dates[dLen] = '\0';
    }
    /* split by '-'  but be careful: DD/MM/YYYY-DD/MM/YYYY */
    char *dash = NULL;
    /* find last '-' that is not part of a date number sequence */
    for (int i = (int)strlen(dates)-1; i >= 0; i--) {
        if (dates[i] == '-' && (i == 0 || !isdigit((unsigned char)dates[i-1]) || i < 5)) {
            /* heuristic: if preceded by a year (4 digits end) this is separator */
            dash = dates + i;
            break;
        }
        /* simpler: split at '-' that is not surrounded by digits on both sides */
        if (dates[i] == '-') {
            bool leftDigit  = (i > 0)  && isdigit((unsigned char)dates[i-1]);
            bool rightDigit = (i < (int)strlen(dates)-1) && isdigit((unsigned char)dates[i+1]);
            if (leftDigit && rightDigit && i > 2) { /* likely a year separator */ 
                dash = dates + i;
                break;
            }
        }
    }
    if (dash) {
        int d1 = (int)(dash - dates);
        strncpy(dob, dates, d1); dob[d1] = '\0';
        strcpy(dod, dash + 1);
    } else {
        strcpy(dob, dates);
    }

    /* definition = everything after '=' */
    const char *defStart = eq + 1;
    while (*defStart == ' ') defStart++;
    strncpy(def, defStart, MAX_DEF - 1);
    def[MAX_DEF-1] = '\0';
    /* strip trailing newline */
    int dfl = (int)strlen(def);
    while (dfl > 0 && (def[dfl-1] == '\n' || def[dfl-1] == '\r')) def[--dfl] = '\0';

    return true;
}

/* Event format:  Event name {date}: description */
bool parseEventLine(const char *line,
                    char *name, char *date, char *def) {
    name[0] = date[0] = def[0] = '\0';

    const char *brace = strchr(line, '{');
    const char *close = strchr(line, '}');
    const char *colon = strchr(line, ':');

    if (!brace || !close || !colon) return false;

    int nameLen = (int)(brace - line);
    while (nameLen > 0 && isspace((unsigned char)line[nameLen-1])) nameLen--;
    strncpy(name, line, nameLen);
    name[nameLen] = '\0';

    int dLen = (int)(close - brace - 1);
    if (dLen > 0 && dLen < MAX_DATE-1) {
        strncpy(date, brace + 1, dLen);
        date[dLen] = '\0';
    }

    /* colon after '}' */
    const char *defStart = close + 1;
    while (*defStart == ':' || *defStart == ' ') defStart++;
    strncpy(def, defStart, MAX_DEF - 1);
    def[MAX_DEF-1] = '\0';
    int dfl = (int)strlen(def);
    while (dfl > 0 && (def[dfl-1] == '\n' || def[dfl-1] == '\r')) def[--dfl] = '\0';

    return true;
}

void saveListToFile(const char *filename, TList *personalities, TList *events) {
    FILE *f = fopen(filename, "w");
    if (!f) { perror("saveListToFile"); return; }

    TList *cur = personalities;
    while (cur) {
        if (cur->dod[0])
            fprintf(f, "%s {%s-%s}= %s\n", cur->name, cur->dob, cur->dod, cur->definition);
        else
            fprintf(f, "%s {%s}= %s\n", cur->name, cur->dob, cur->definition);
        cur = cur->next;
    }
    cur = events;
    while (cur) {
        fprintf(f, "%s {%s}: %s\n", cur->name, cur->dob, cur->definition);
        cur = cur->next;
    }
    fclose(f);
}