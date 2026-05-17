#include "recursion.h"
#include "file_parser.h"

// Count occurrences of name in file
int countRec(FILE *f, const char *name, char *line, int maxLen) {
    int found;
    if (!fgets(line, maxLen, f)) return 0;
    found = (strstr(line, name) != NULL) ? 1 : 0;
    return found + countRec(f, name, line, maxLen);
}

int countOccurrence(FILE *f, const char *name) {
    char line[MAX_LINE];
    rewind(f);
    int count = countRec(f, name, line, MAX_LINE);
    printf("  '%s' appears %d time(s) in the file.\n", name, count);
    return count;
}

// Remove all occurrences
void removeOccurrence(const char *filename, const char *word) {
    FILE *f = fopen(filename, "r");
    if (!f) { perror("removeOccurrence"); return; }
    char tmp[512]; snprintf(tmp, sizeof(tmp), "%s.tmp", filename);
    FILE *out = fopen(tmp, "w");
    if (!out) { fclose(f); return; }
    char line[MAX_LINE];
    while (fgets(line, MAX_LINE, f)) {
        if (!strstr(line, word)) fputs(line, out);
    }
    fclose(f); fclose(out);
    remove(filename); rename(tmp, filename);
    printf("  Removed all lines containing '%s'.\n", word);
}

/* Replace all occurrences of name in file with new dates */
void replaceOccurrence(const char *filename, const char *name,
                       const char *DoB, const char *DoD) {
    FILE *f = fopen(filename, "r");
    if (!f) { perror("replaceOccurrence"); return; }
    char tmp[512]; snprintf(tmp, sizeof(tmp), "%s.tmp", filename);
    FILE *out = fopen(tmp, "w");
    if (!out) { fclose(f); return; }
    char line[MAX_LINE];
    while (fgets(line, MAX_LINE, f)) {
        if (strstr(line, name) && isPersonalityLine(line)) {
            char pname[MAX_NAME], pdob[MAX_DATE], pdod[MAX_DATE], pdef[MAX_DEF];
            if (parsePersonalityLine(line, pname, pdob, pdod, pdef)) {
                if(DoB&&DoB[0]) strncpy(pdob,DoB,MAX_DATE-1);
                if(DoD&&DoD[0]) strncpy(pdod,DoD,MAX_DATE-1);
                if(pdod[0]) fprintf(out,"%s {%s-%s}= %s\n",pname,pdob,pdod,pdef);
                else fprintf(out,"%s {%s}= %s\n",pname,pdob,pdef);
                continue;
            }
        }
        fputs(line, out);
    }
    fclose(f); fclose(out);
    remove(filename); rename(tmp, filename);
    printf("  Replaced dates for '%s'.\n", name);
}

/* All permutations of a name */
void permHelper(char *s, int l, int r) {
    if (l == r) { printf("    %s\n", s); return; }
    for (int i = l; i <= r; i++) {
        char t = s[l]; s[l] = s[i]; s[i] = t;
        permHelper(s, l+1, r);
        t = s[l]; s[l] = s[i]; s[i] = t;
    }
}
void namePermutation(char *name) {
    printf("  Permutations of '%s':\n", name);
    permHelper(name, 0, (int)strlen(name)-1);
}

/* All subsequences of a word */
void subseqHelper(const char *word, char *buf, int wi, int bi) {
    buf[bi] = '\0';
    if (bi > 0) printf("    %s\n", buf);
    for (int i = wi; word[i]; i++) {
        buf[bi] = word[i];
        subseqHelper(word, buf, i+1, bi+1);
    }
}
void subseqName(const char *word) {
    char buf[MAX_NAME] = {0};
    printf("  Subsequences of '%s':\n", word);
    subseqHelper(word, buf, 0, 0);
}

/* Events that overlap with given dates */
void overlapHelper(TList *events, int y1, int y2) {
    if (!events) return;
    int ey = dateToYear(events->dob);
    if (ey >= y1 && ey <= y2)
        printf("    %s (%s)\n", events->name, events->dob);
    overlapHelper(events->next, y1, y2);
}
void longestSubyear(TList *events, const char *date1, const char *date2) {
    int y1 = dateToYear(date1), y2 = dateToYear(date2);
    printf("  Events between %s and %s:\n", date1, date2);
    overlapHelper(events, y1 < y2 ? y1 : y2, y1 > y2 ? y1 : y2);
}

/* Count distinct subsequences */
int distinctSubseqWord(const char *event) {
    int n = (int)strlen(event);
    /* Using DP approach */
    int *dp = (int*)calloc(n+1, sizeof(int));
    dp[0] = 1;
    for (int i = 1; i <= n; i++) {
        dp[i] = dp[i-1] * 2;
        /* subtract duplicates */
        for (int j = i-1; j >= 1; j--)
            if (event[j-1] == event[i-1]) { dp[i] -= dp[j-1]; break; }
    }
    int result = dp[n];
    free(dp);
    printf("  Distinct subsequences of '%s': %d\n", event, result);
    return result;
}

bool isPalindromeWord(const char *event) {
    int l=0, r=(int)strlen(event)-1;
    if (l >= r) return true;
    if (tolower((unsigned char)event[l]) != tolower((unsigned char)event[r]))
        return false;
    char sub[MAX_DEF]; strncpy(sub, event+1, r-1); sub[r-1]='\0';
    return isPalindromeWord(sub);
}
