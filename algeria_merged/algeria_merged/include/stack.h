#ifndef STACK_H
#define STACK_H

#include "types.h"

/* ---- Construction ---- */
TStack* toStack(TList *merged);

/* ---- Search ---- */
TStack* getInfoPersonality(TStack *stk, const char *name);

/* ---- Sort ---- */
TStack* sortNameStack(TStack *s);

/* ---- CRUD ---- */
TStack* deleteName(TStack *stk, const char *name);
TStack* updateStack(TStack *stk, const char *name,
                    const char *def, const char *DoB, const char *DoD);
TStack* addNameStack(TStack *stk, const char *name,
                     const char *definition, const char *DoB, const char *DoD);

/* ---- Conversions ---- */
TQueue* stackToQueue(TStack *stk);
TList*  stackToList(TStack *stk);

/* ---- Special ---- */
TStack* definitionStack(TStack *stk);
TStack* pronunciationStack(TStack *stk);
char*   getSmallest(TStack *stk);
void    continuousSearch(TStack *stk);
bool    isPersonalityKilled(const char *word);
TStack* recRevStack(TStack *stk);

/* ---- Helpers ---- */
TStack* pushStack(TStack *top, TList *node);
TStack* pushStackRaw(TStack *top, const char *name, const char *def,
                     const char *dob, const char *dod);
TStack* popStack(TStack **top);
void    printStack(TStack *stk);
void    freeStack(TStack *stk);
int     stackSize(TStack *stk);

#endif
