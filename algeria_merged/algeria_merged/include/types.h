#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_NAME  256
#define MAX_DEF   1024
#define MAX_DATE  20
#define MAX_LINE  2048
#define DATA_FILE "data/history.txt"

/* --- Windows compatibility --- */
#ifdef _WIN32
  #define strcasecmp  _stricmp
  #define strncasecmp _strnicmp
#endif

//Date
typedef struct {
    int day;
    int month;
    int year;
} Date;

//Doubly linked list node (personality or event)
typedef struct TNode {
    char name[MAX_NAME];
    char definition[MAX_DEF];
    char dob[MAX_DATE];
    char dod[MAX_DATE];
    int  type;          // 0 = personality, 1 = event
    struct TNode *next;
    struct TNode *prev;
} TNode;

typedef TNode TList;

//Stack node
typedef struct SNode {
    char name[MAX_NAME];
    char definition[MAX_DEF];
    char dob[MAX_DATE];
    char dod[MAX_DATE];
    int  type;
    struct SNode *next;
} SNode;

typedef SNode TStack;

//Queue node
typedef struct QNode {
    char name[MAX_NAME];
    char definition[MAX_DEF];
    char dob[MAX_DATE];
    char dod[MAX_DATE];
    int  type;
    struct QNode *next;
} QNode;

typedef QNode TQueueNode;  

typedef struct {
    QNode *head;
    QNode *tail;
    int    size;
} TQueue;

//BST node
typedef struct TTree {
    char name[MAX_NAME];
    char definition[MAX_DEF];
    char dob[MAX_DATE];
    char dod[MAX_DATE];
    int  type;
    struct TTree *left;
    struct TTree *right;
} TTree;

//Helper functions (defined in file_parser.c)
Date parseDate(const char *s);
int  dateToYear(const char *s);
int  personalityAge(const char *dob, const char *dod);

#endif
