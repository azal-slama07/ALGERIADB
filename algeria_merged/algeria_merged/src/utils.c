#include "../include/algeria_db.h"

/* ── Parse a date string "DD/MM/YYYY" or "YYYY" into a Date struct ── */
void parseDate(const char *str, Date *d) {
    d->day = 0; d->month = 0; d->year = 0;
    if (!str || str[0] == '\0') return;
    /* try DD/MM/YYYY */
    if (sscanf(str, "%d/%d/%d", &d->day, &d->month, &d->year) == 3) return;
    /* try YYYY */
    sscanf(str, "%d", &d->year);
}

int dateCmp(Date *a, Date *b) {
    if (a->year  != b->year)  return a->year  - b->year;
    if (a->month != b->month) return a->month - b->month;
    return a->day - b->day;
}

int extractYear(const char *dateStr) {
    if (!dateStr || dateStr[0] == '\0') return 0;
    Date d; parseDate(dateStr, &d);
    return d.year;
}

int computeAge(const char *dob, const char *dod) {
    int y1 = extractYear(dob);
    int y2 = extractYear(dod);
    if (y1 == 0 || y2 == 0) return 0;
    return y2 - y1;
}

void trimWhitespace(char *s) {
    /* leading */
    int i = 0;
    while (s[i] && isspace((unsigned char)s[i])) i++;
    if (i > 0) memmove(s, s + i, strlen(s) - i + 1);
    /* trailing */
    int len = (int)strlen(s);
    while (len > 0 && isspace((unsigned char)s[len-1])) len--;
    s[len] = '\0';
}

int isPalindrome_str(const char *s) {
    int l = 0, r = (int)strlen(s) - 1;
    while (l < r) {
        if (tolower((unsigned char)s[l]) != tolower((unsigned char)s[r]))
            return 0;
        l++; r--;
    }
    return 1;
}

int wordCount(const char *s) {
    int count = 0, inWord = 0;
    for (int i = 0; s[i]; i++) {
        if (!isspace((unsigned char)s[i])) {
            if (!inWord) { count++; inWord = 1; }
        } else inWord = 0;
    }
    return count;
}
