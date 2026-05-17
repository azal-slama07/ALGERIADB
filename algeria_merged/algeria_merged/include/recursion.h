#ifndef RECURSION_H
#define RECURSION_H

#include "types.h"

int   countOccurrence(FILE *f, const char *name);
void  removeOccurrence(const char *filename, const char *word);
void  replaceOccurrence(const char *filename, const char *name,
                        const char *DoB, const char *DoD);
void  namePermutation(char *name);
void  subseqName(const char *word);
void  longestSubyear(TList *events, const char *date1, const char *date2);
int   distinctSubseqWord(const char *event);
bool  isPalindromeWord(const char *event);

#endif
