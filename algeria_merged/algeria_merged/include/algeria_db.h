#ifndef ALGERIA_DB_H
#define ALGERIA_DB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

//CONSTANTS
#define MAX_NAME   256
#define MAX_DEF    1024
#define MAX_DATE   20
#define MAX_LINE   2048
#define DATA_FILE  "data/history.txt"

//DATE TYPE
typedef struct {
    int day;
    int month;
    int year;
} Date;

//LINKED LIST
typedef struct TNode {
    char name[MAX_NAME];
    char definition[MAX_DEF];
    char dob[MAX_DATE];   //date of birth or event date 
    char dod[MAX_DATE];   // date of death 
    int  type;            // 0=personality, 1=event 
    struct TNode *next;
    struct TNode *prev;   // used for bidirectional list
} TNode;

typedef TNode TList;

//STACK
typedef struct SNode {
    char name[MAX_NAME];
    char definition[MAX_DEF];
    char dob[MAX_DATE];
    char dod[MAX_DATE];
    int  type;
    struct SNode *next;
} SNode;

typedef SNode TStack;

//QUEUE
typedef struct QNode {
    char name[MAX_NAME];
    char definition[MAX_DEF];
    char dob[MAX_DATE];
    char dod[MAX_DATE];
    int  type;
    struct QNode *next;
} QNode;

typedef struct {
    QNode *front;
    QNode *rear;
    int    size;
} TQueue;

//BST
typedef struct TTree {
    char name[MAX_NAME];
    char definition[MAX_DEF];
    char dob[MAX_DATE];
    char dod[MAX_DATE];
    int  type;
    struct TTree *left;
    struct TTree *right;
} TTree;

//PARSE HELPERS
void  parseDate(const char *str, Date *d);
int   dateCmp(Date *a, Date *b);
int   extractYear(const char *dateStr);
int   computeAge(const char *dob, const char *dod);
void  trimWhitespace(char *s);
int   isPalindrome_str(const char *s);
int   wordCount(const char *s);

//FILE PARSING
TList* getPersonality(FILE *f);
TList* getDatePersonality(FILE *f);
TList* getEvents(FILE *f);

//LINKED LIST OPERATIONS
TNode* createNode(const char *name, const char *def, const char *dob, const char *dod, int type);
TList* insertTail(TList *head, TNode *node);
TList* insertSorted(TList *head, TNode *node); // alphabetical

void   getInfoByDates(TList *s, const char *dob);
void   getInfoByDates2(TList *s, const char *dod);
TList* sortWord(TList *syn);
TList* sortWord2(TList *syn);
TList* sortPersonality(TList *syn);
TList* deletePersonality(FILE **f, TList *s, TList *a, const char *name);
TList* updatePersonality(FILE **f, TList *s, TList *a, const char *name, const char *definition, const char *dob, const char *dod);
TList* similarPersonality(TList *s, const char *word);
TList* countPersonality(TList *s, const char *prt);
TList* palindromeName(TList *s);
TList* mergeNodes(TList *s, TList *a);   /* bidirectional */
TList* merge2Nodes(TList *s, TList *a);  /* circular */
TList* addPersonality(TList *s, TList *a,
                      const char *name, const char *dob, const char *dod);
TList* addEvents(TList *b, const char *nameEvent, const char *date);
TQueue* sName(TList *s);
TQueue* ageP(TList *a);
TQueue* toQueue(TList *merged);
void   printList(TList *head);
void   freeList(TList *head);

/* ===================== STACK OPERATIONS ===================== */
TStack* pushStack(TStack *top, const char *name, const char *def,
                  const char *dob, const char *dod, int type);
TStack* popStack(TStack *top, SNode **out);
TStack* toStack(TList *merged);
TStack* getInfoPersonality(TStack *stk, const char *name);
TStack* sortNameStack(TStack *s);
TStack* deleteName(TStack *stk, const char *name);
TStack* updateStack(TStack *stk, const char *name,
                    const char *def, const char *dob, const char *dod);
TQueue* stackToQueue(TStack *stk);
TList*  stackToList(TStack *stk);
TStack* addNameStack(TStack *stk, const char *name,
                     const char *definition, const char *dob, const char *dod);
TStack* definitionStack(TStack *stk);
char*   getSmallest(TStack *stk);
void    continuousSearch(TStack *stk);
bool    isPersonalityKilled(const char *word);
TStack* recRevStack(TStack *stk);
void    printStack(TStack *top);
void    freeStack(TStack *top);

/* ===================== QUEUE OPERATIONS ===================== */
TQueue* createQueue(void);
void    enqueue(TQueue *q, const char *name, const char *def,
                const char *dob, const char *dod, int type);
QNode*  dequeue(TQueue *q);
void    printQueue(TQueue *q);
void    freeQueue(TQueue *q);

/* ===================== BST OPERATIONS ===================== */
TTree* createTreeNode(const char *name, const char *def,
                      const char *dob, const char *dod, int type);
TTree* insertBST(TTree *root, TTree *node);
TTree* toTree(TStack *stk);
TTree* fillTree(FILE *f);
TTree* getInfoNameTree(TTree *tr, const char *name);
TTree* addNameBST(TTree *tr, const char *name,
                  const char *dob, const char *dod);
TTree* deleteNameBST(TTree *tr, const char *name);
TTree* updateNameBST(TTree *tr, const char *name,
                     const char *s, const char *dob, const char *dod);
void   traversalBSTinOrder(TTree *tr);
void   traversalBSTpreOrder(TTree *tr);
void   traversalBSTpostOrder(TTree *tr);
void   heightSizeBST(TTree *tr);
int    heightBST(TTree *tr);
int    sizeBST(TTree *tr);
TTree* lowestCommonAncestor(TTree *tr,
                            const char *word1, const char *word2);
int    countNodesRange(TTree *tr, int l, int h);
TTree* inOrderSuccessor(TTree *tr, const char *word);
TTree* BSTMirror(TTree *tr);
bool   isBalancedBST(TTree *tr);
TTree* BTSMerge(TTree *tr1, TTree *tr2);
void   freeBST(TTree *tr);

/* ===================== RECURSIVE MODULES ===================== */
int   countOccurrence(FILE *f, const char *name);
void  namePermutation(char *name, int start, int len);
void  subseqName(const char *word, char *buf, int idx, int bufIdx);
int   distinctSubseqWord(const char *event);
bool  isPalindromeWord(const char *event);

/* ===================== FILE UTILITIES ===================== */
void  rewriteFile(FILE **f, TList *personalities, TList *events);
void  appendPersonalityToFile(const char *filename,
                               const char *name, const char *def,
                               const char *dob, const char *dod);
void  appendEventToFile(const char *filename,
                        const char *nameEvent, const char *date);

/* ===================== MENU ===================== */
void  mainMenu(void);

#endif /* ALGERIA_DB_H */
