#include "../include/algeria_db.h"

/*
 * File format:
 *   Personality:  Name {DOB-DOD}= definition
 *   Event:        Name {DATE}: description
 *
 * Rules from project spec:
 *   '=' separates personality name+dates from definition
 *   ':' separates event name+date from description
 *   {} encloses important dates
 */

/* ── helpers ── */
static void extractDates(const char *src, char *dob, char *dod) {
    /* src looks like "Name {DOB-DOD}" or "Name {DATE}" */
    dob[0] = dod[0] = '\0';
    const char *ob = strchr(src, '{');
    const char *cb = strchr(src, '}');
    if (!ob || !cb || cb <= ob) return;
    char buf[64];
    int len = (int)(cb - ob - 1);
    if (len <= 0 || len >= 64) return;
    strncpy(buf, ob + 1, len); buf[len] = '\0';
    /* check for dash separating DOB and DOD */
    char *dash = strchr(buf, '-');
    if (dash) {
        strncpy(dob, buf, dash - buf);
        dob[dash - buf] = '\0';
        strncpy(dod, dash + 1, MAX_DATE - 1);
    } else {
        strncpy(dob, buf, MAX_DATE - 1); /* single date = event date */
    }
    trimWhitespace(dob);
    trimWhitespace(dod);
}

static void extractName(const char *src, char *name) {
    /* name is everything before '{' */
    name[0] = '\0';
    const char *ob = strchr(src, '{');
    if (!ob) {
        strncpy(name, src, MAX_NAME - 1);
    } else {
        int len = (int)(ob - src);
        if (len >= MAX_NAME) len = MAX_NAME - 1;
        strncpy(name, src, len);
        name[len] = '\0';
    }
    trimWhitespace(name);
}

/* ── getPersonality: builds list of personalities (lines with '=') ── */
TList* getPersonality(FILE *f) {
    if (!f) return NULL;
    rewind(f);
    TList *head = NULL;
    char line[MAX_LINE];
    while (fgets(line, MAX_LINE, f)) {
        trimWhitespace(line);
        if (line[0] == '\0') continue;
        char *eq = strchr(line, '=');
        if (!eq) continue;                /* not a personality line */
        /* split into left part (name+dates) and right part (definition) */
        *eq = '\0';
        char *left = line;
        char *def  = eq + 1;
        trimWhitespace(left);
        trimWhitespace(def);
        char name[MAX_NAME], dob[MAX_DATE], dod[MAX_DATE];
        extractName(left, name);
        extractDates(left, dob, dod);
        TNode *node = createNode(name, def, dob, dod, 0);
        head = insertTail(head, node);
    }
    return head;
}

/* ── getDatePersonality: same but stores dates only, no definition ── */
TList* getDatePersonality(FILE *f) {
    if (!f) return NULL;
    rewind(f);
    TList *head = NULL;
    char line[MAX_LINE];
    while (fgets(line, MAX_LINE, f)) {
        trimWhitespace(line);
        if (line[0] == '\0') continue;
        char *eq = strchr(line, '=');
        if (!eq) continue;
        *eq = '\0';
        char *left = line;
        trimWhitespace(left);
        char name[MAX_NAME], dob[MAX_DATE], dod[MAX_DATE];
        extractName(left, name);
        extractDates(left, dob, dod);
        TNode *node = createNode(name, "", dob, dod, 0);
        head = insertTail(head, node);
    }
    return head;
}

/* ── getEvents: builds list of events (lines with ':' but not '=') ── */
TList* getEvents(FILE *f) {
    if (!f) return NULL;
    rewind(f);
    TList *head = NULL;
    char line[MAX_LINE];
    while (fgets(line, MAX_LINE, f)) {
        trimWhitespace(line);
        if (line[0] == '\0') continue;
        if (strchr(line, '=')) continue;  /* skip personalities */
        char *col = strchr(line, ':');
        if (!col) continue;
        *col = '\0';
        char *left = line;
        char *desc  = col + 1;
        trimWhitespace(left);
        trimWhitespace(desc);
        char name[MAX_NAME], dob[MAX_DATE], dod[MAX_DATE];
        extractName(left, name);
        extractDates(left, dob, dod);
        TNode *node = createNode(name, desc, dob, dod, 1);
        head = insertTail(head, node);
    }
    return head;
}

/* ── rewriteFile: save both lists back to file ── */
void rewriteFile(FILE **f, TList *personalities, TList *events) {
    if (!f || !*f) return;
    fclose(*f);
    *f = fopen(DATA_FILE, "w");
    if (!*f) { perror("fopen"); return; }
    TNode *cur = personalities;
    while (cur) {
        if (strlen(cur->dod) > 0)
            fprintf(*f, "%s {%s-%s}= %s\n", cur->name, cur->dob, cur->dod, cur->definition);
        else
            fprintf(*f, "%s {%s}= %s\n", cur->name, cur->dob, cur->definition);
        cur = cur->next;
    }
    cur = events;
    while (cur) {
        fprintf(*f, "%s {%s}: %s\n", cur->name, cur->dob, cur->definition);
        cur = cur->next;
    }
}

void appendPersonalityToFile(const char *filename, const char *name,
                              const char *def, const char *dob, const char *dod) {
    FILE *f = fopen(filename, "a");
    if (!f) { perror("fopen"); return; }
    if (dod && strlen(dod) > 0)
        fprintf(f, "%s {%s-%s}= %s\n", name, dob, dod, def);
    else
        fprintf(f, "%s {%s}= %s\n", name, dob, def);
    fclose(f);
}

void appendEventToFile(const char *filename,
                       const char *nameEvent, const char *date) {
    FILE *f = fopen(filename, "a");
    if (!f) { perror("fopen"); return; }
    fprintf(f, "%s {%s}: event\n", nameEvent, date);
    fclose(f);
}
