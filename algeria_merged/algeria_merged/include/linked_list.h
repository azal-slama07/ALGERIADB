#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "types.h"

//List helpers
TList* newNode(const char *name, const char *dob, const char *dod, const char *def);
void   appendNode(TList **head, TList *node);
void   printList(TList *head);
void   freeList(TList *head);
int    listLength(TList *head);

//Queue helpers
TQueue*    newQueue();
void       enqueue(TQueue *q, TList *node);
TQueueNode* dequeue(TQueue *q);
bool       isQueueEmpty(TQueue *q);
void       printQueue(TQueue *q);
void       freeQueue(TQueue *q);

//Load from file
TList* getPersonality(FILE *f);
TList* getDatePersonality(FILE *f);
TList* getEvents(FILE *f);

//Search
void   getInfoByDates(TList *s, const char *DoB);
void   getInfoByDates2(TList *s, const char *DoD);

//Sort helpers
TList* insertSortedInto(TList *sorted, TList *node, int(*cmp)(TList*,TList*));
int    cmpAlpha(TList *a, TList *b);
int    cmpLen(TList *a, TList *b);
int    cmpAge(TList *a, TList *b);
TList* sortListBy(TList *head, int(*cmp)(TList*,TList*));
TList* sortWord(TList *lst);
TList* sortWord2(TList *lst);
TList* sortPersonality(TList *lst);

//CRUD
TList* addPersonality(TList *s, TList *a, const char *name, const char *DoB, const char *DoD, const char *def, const char *filename);
TList* addEvents(TList *b, const char *nameEvent, const char *date, const char *def, const char *filename);
TList* deletePersonality(const char *filename, TList *s, TList *a, const char *name);
TList* updatePersonality(const char *filename, TList *s, TList *a, const char *name, const char *definition, const char *DoB, const char *DoD);

//Special queries
TList* similarPersonality(TList *s, const char *word);
TList* countPersonality(TList *s, Date *prt);
TList* palindromeName(TList *s);

//Merge
TList* mergeNodes(TList *s, TList *a);
TList* merge2Nodes(TList *s, TList *a);

//Queue conversions
TQueue* sName(TList *s);
TQueue* ageP(TList *a);
TQueue* toQueue(TList *merged);

#endif
