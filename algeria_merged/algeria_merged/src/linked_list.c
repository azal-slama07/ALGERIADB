#include "linked_list.h"
#include "file_parser.h"

//LLL Helpers
TList* newNode(const char *name, const char *dob, const char *dod, const char *def) {
    TList *n = (TList*)malloc(sizeof *n);
    if (!n) return NULL;
    n->next = n->prev = NULL;
    n->type = 0;
    strncpy(n->name, name ? name : "", MAX_NAME-1);
    n->name[MAX_NAME-1] = '\0';
    strncpy(n->dob, dob ? dob : "", MAX_DATE-1);
    n->dob[MAX_DATE-1] = '\0';
    strncpy(n->dod, dod ? dod : "", MAX_DATE-1);
    n->dod[MAX_DATE-1] = '\0';
    strncpy(n->definition, def ? def : "", MAX_DEF-1);
    n->definition[MAX_DEF-1] = '\0';
    return n;
}

void appendNode(TList **head, TList *node) {
    TList *cur;
    if (!node) return;
    node->next = NULL;
    node->prev = NULL;
    if (!*head) {
        *head = node;
        return;
    }
    cur = *head;
    while (cur->next) cur = cur->next;
    cur->next = node;
    node->prev = cur;
}

void printList(TList *head) {
    int i;
    if (!head) {
        printf("  (empty list)\n");
        return;
    }
    i = 1;
    while (head) {
        printf("%d/ Name: %s\n", i++, head->name);
        if (head->dob[0]) printf("DoB : %s\n", head->dob);
        if (head->dod[0]) printf("DoD : %s\n", head->dod);
        if (head->definition[0]) printf("Def : %s\n", head->definition);
        head = head->next;
    }
}

void freeList(TList *head) {
    TList *t;
    while (head) {
        t = head->next;
        free(head);
        head = t;
    }
}

int listLength(TList *head) {
    int c;
    c = 0;
    while (head) {
        c++;
        head = head->next;
    }
    return c;
}

//Queue helpers
TQueue* newQueue(void) {
    TQueue *q = (TQueue*)calloc(1, sizeof(TQueue)); /* calloc zeros head/tail/size */
    return q;
}

void enqueue(TQueue *q, TList *node) {
    TQueueNode *qn;
    qn = (TQueueNode*)malloc(sizeof(TQueueNode));
    strncpy(qn->name, node->name, MAX_NAME-1);
    strncpy(qn->dob, node->dob, MAX_DATE-1);
    strncpy(qn->dod, node->dod, MAX_DATE-1);
    strncpy(qn->definition, node->definition, MAX_DEF-1);
    qn->next = NULL;
    if (!q->head) { q->head = q->tail = qn; }
    else { q->tail->next = qn; q->tail = qn; }
}

TQueueNode* dequeue(TQueue *q) {
    TQueueNode *n;
    if (!q->head) return NULL;
    n = q->head;
    q->head = q->head->next;
    if (!q->head) q->tail = NULL;
    n->next = NULL;
    return n;
}

bool isQueueEmpty(TQueue *q) { return q->head == NULL; }

void printQueue(TQueue *q) {
    TQueueNode *cur;
    int i;
    if (!q || !q->head) {
        printf("empty queue\n");
        return;
    }
    cur = q->head;
    i = 1;
    while (cur) {
        printf("%d/ %s  DoB:%s  DoD:%s\n", i++, cur->name, cur->dob, cur->dod);
        cur = cur->next;
    }
}

void freeQueue(TQueue *q) {
    TQueueNode *cur;
    TQueueNode *t;
    if (!q) return;
    cur = q->head;
    while (cur) {
        t = cur->next;
        free(cur);
        cur = t;
    }
    free(q);
}

//File loading : project functions
TList* getPersonality(FILE *f) {
    TList *head = NULL;
    char line[MAX_LINE];
    char name[MAX_NAME];
    char dob[MAX_DATE];
    char dod[MAX_DATE];
    char def[MAX_DEF];
    rewind(f);
    while (fgets(line, MAX_LINE, f)) {
        if (!isPersonalityLine(line)) continue;
        if (parsePersonalityLine(line, name, dob, dod, def))
            appendNode(&head, newNode(name, "", "", def));
    }
    return head;
}

TList* getDatePersonality(FILE *f) {
    TList *head = NULL;
    char line[MAX_LINE];
    char name[MAX_NAME];
    char dob[MAX_DATE];
    char dod[MAX_DATE];
    char def[MAX_DEF];
    rewind(f);
    while (fgets(line, MAX_LINE, f)) {
        if (!isPersonalityLine(line)) continue;
        if (parsePersonalityLine(line, name, dob, dod, def))
            appendNode(&head, newNode(name, dob, dod, ""));
    }
    return head;
}

TList* getEvents(FILE *f) {
    TList *head = NULL;
    char line[MAX_LINE];
    char name[MAX_NAME];
    char date[MAX_DATE];
    char def[MAX_DEF];
    rewind(f);
    while (fgets(line, MAX_LINE, f)) {
        if (!isEventLine(line)) continue;
        if (parseEventLine(line, name, date, def)) {
            TList *node = newNode(name, date, "", def);
            node->type = 1;
            appendNode(&head, node);
        }
    }
    return head;
}

void getInfoByDates(TList *s, const char *DoB) {
    bool found = false;
    while (s) {
        if (strcmp(s->dob, DoB) == 0) {
            printf("Name: %s\n  DoB : %s\n  DoD : %s\n  Def : %s\n\n",
                   s->name, s->dob, s->dod, s->definition);
            found = true;
        }
        s = s->next;
    }
    if (!found) printf("No personality found with DoB: %s\n", DoB);
}
void getInfoByDates2(TList *s, const char *DoD) {
    bool found = false;
    while (s) {
        if (strcmp(s->dod, DoD) == 0) {
            printf("Name: %s\n  DoB : %s\n  DoD : %s\n  Def : %s\n\n", s->name, s->dob, s->dod, s->definition);
            found = true;
        }
        s = s->next;
    }
    if (!found) printf("No personality found with DoD: %s\n", DoD);
}

TList* insertSortedInto(TList *sorted, TList *node, int(*cmp)(TList*,TList*)) {
    TList *cur;
    node->next = node->prev = NULL;
    if (!sorted || cmp(node, sorted) <= 0) {
        node->next = sorted;
        if (sorted) sorted->prev = node;
        return node;
    }
    cur = sorted;
    while (cur->next && cmp(node, cur->next) > 0)
        cur = cur->next;
    node->next = cur->next;
    if (cur->next) cur->next->prev = node;
    cur->next = node;
    node->prev = cur;
    return sorted;
}

int cmpAlpha(TList *a, TList *b) { return strcasecmp(a->name, b->name); }
int cmpLen(TList *a, TList *b) { return (int)strlen(a->name)-(int)strlen(b->name); }
int cmpAge(TList *a, TList *b) {
    return personalityAge(a->dob,a->dod) - personalityAge(b->dob,b->dod);
}
TList* sortListBy(TList *head, int(*cmp)(TList*,TList*)) {
    TList *sorted = NULL;
    TList *cur = head;
    TList *next;
    while (cur) { next = cur->next; cur->next=cur->prev=NULL; sorted=insertSortedInto(sorted,cur,cmp); cur=next; }
    return sorted;
}
TList* sortWord(TList *lst)        { return sortListBy(lst, cmpAlpha); }
TList* sortWord2(TList *lst)       { return sortListBy(lst, cmpLen);   }
TList* sortPersonality(TList *lst) { return sortListBy(lst, cmpAge);   }


/* ========== CRUD ========== */
TList* deletePersonality(const char *filename, TList *s, TList *a, const char *name) {
    TList *cur = s, *prev2 = NULL;
    while (cur) {
        if (strcasecmp(cur->name, name) == 0) {
            if (prev2) prev2->next = cur->next; else s = cur->next;
            free(cur); printf("  Deleted '%s'.\n", name); break;
        }
        prev2 = cur; cur = cur->next;
    }
    (void)a;
    if (filename) saveListToFile(filename, s, NULL);
    return s;
}
TList* updatePersonality(const char *filename, TList *s, TList *a,
                         const char *name, const char *definition,
                         const char *DoB, const char *DoD) {
    TList *cur = s;
    while (cur) {
        if (strcasecmp(cur->name, name) == 0) {
            if (definition && definition[0]) strncpy(cur->definition, definition, MAX_DEF-1);
            if (DoB && DoB[0]) strncpy(cur->dob, DoB, MAX_DATE-1);
            if (DoD && DoD[0]) strncpy(cur->dod, DoD, MAX_DATE-1);
            printf("  Updated '%s'.\n", name); break;
        }
        cur = cur->next;
    }
    (void)a;
    if (filename) saveListToFile(filename, s, NULL);
    return s;
}
TList* addPersonality(TList *s, TList *a, const char *name,
                      const char *DoB, const char *DoD,
                      const char *def, const char *filename) {
    (void)a;
    appendNode(&s, newNode(name, DoB, DoD, def));
    if (filename) saveListToFile(filename, s, NULL);
    printf("  Added '%s'.\n", name);
    return s;
}
TList* addEvents(TList *b, const char *nameEvent, const char *date,
                 const char *def, const char *filename) {
    appendNode(&b, newNode(nameEvent, date, "", def));
    if (filename) saveListToFile(filename, NULL, b);
    printf("  Added event '%s'.\n", nameEvent);
    return b;
}

/* ========== Special queries ========== */
TList* similarPersonality(TList *s, const char *word) {
    TList *result = NULL; int target = dateToYear(word);
    while (s) {
        if (dateToYear(s->dob)==target || dateToYear(s->dod)==target)
            appendNode(&result, newNode(s->name, s->dob, s->dod, s->definition));
        s = s->next;
    }
    return result;
}
TList* countPersonality(TList *s, Date *prt) {
    TList *result = NULL;
    Date db;
    Date dd;
    bool match;
    while (s) {
        db = parseDate(s->dob);
        dd = parseDate(s->dod);
        match = false;
        if (prt->year && (db.year==prt->year || dd.year==prt->year)) match = true;
        if (prt->month && prt->day)
            if ((db.month==prt->month && db.day==prt->day) || (dd.month==prt->month && dd.day==prt->day)) match = true;
        if (match) appendNode(&result, newNode(s->name, s->dob, s->dod, s->definition));
        s = s->next;
    }
    return result;
}
static bool wordIsPalin(const char *s) {
    char low[MAX_NAME];
    int i;
    int l;
    int r;
    strncpy(low, s, MAX_NAME-1);
    low[MAX_NAME-1] = '\0';
    for (i = 0; low[i]; i++)
        low[i] = (char)tolower((unsigned char)low[i]);
    l = 0;
    r = (int)strlen(low) - 1;
    while (l < r) {
        if (low[l] != low[r]) return false;
        l++;
        r--;
    }
    return true;
}
TList* palindromeName(TList *s) {
    TList *result = NULL;
    char buf[MAX_DEF];
    char *tok;
    while (s) {
        strncpy(buf, s->definition, MAX_DEF-1);
        buf[MAX_DEF-1] = '\0';
        tok = strtok(buf, " ,.");
        while (tok) {
            if (strlen(tok) > 2 && wordIsPalin(tok)) {
                appendNode(&result, newNode(tok, "", "", s->name));
                break;
            }
            tok = strtok(NULL, " ,.");
        }
        s = s->next;
    }
    return sortWord(result);
}

/* ========== Merge ========== */
TList* mergeNodes(TList *s, TList *a) {
    TList *result = NULL;
    TList *n;
    TList *cur;
    while (s) {
        n = newNode(s->name, s->dob, s->dod, s->definition);
        appendNode(&result, n);
        s = s->next;
    }
    cur = result;
    while (cur && cur->next) {
        cur->next->prev = cur;
        cur = cur->next;
    }
    (void)a;
    return result;
}
TList* merge2Nodes(TList *s, TList *a) {
    TList *result;
    TList *tail;
    result = mergeNodes(s, a);
    if (!result) return NULL;
    tail = result;
    while (tail->next)
        tail = tail->next;
    tail->next = result;
    result->prev = tail;
    return result;
}

/* ========== Queue conversions ========== */
TQueue* sName(TList *s) {
    /* collect into array, sort by word count */
    int n;
    TList **arr;
    TList *cur;
    int i;
    int x;
    int y;
    int wc1;
    int wc2;
    char t1[MAX_NAME];
    char t2[MAX_NAME];
    char *tk;
    TQueue *q;
    TList *tmp;
    n = listLength(s);
    if (n == 0) return newQueue();
    arr = (TList**)malloc(n * sizeof(TList*));
    cur = s;
    i = 0;
    while (cur) {
        arr[i++] = cur;
        cur = cur->next;
    }
    for (x = 0; x < n - 1; x++)
        for (y = x + 1; y < n; y++) {
            wc1 = wc2 = 0;
            strncpy(t1, arr[x]->name, MAX_NAME-1);
            t1[MAX_NAME-1] = '\0';
            strncpy(t2, arr[y]->name, MAX_NAME-1);
            t2[MAX_NAME-1] = '\0';
            tk = strtok(t1, " ");
            while (tk) {
                wc1++;
                tk = strtok(NULL, " ");
            }
            tk = strtok(t2, " ");
            while (tk) {
                wc2++;
                tk = strtok(NULL, " ");
            }
            if (wc1 > wc2) {
                tmp = arr[x];
                arr[x] = arr[y];
                arr[y] = tmp;
            }
        }
    q = newQueue();
    for (x = 0; x < n; x++)
        enqueue(q, arr[x]);
    free(arr);
    return q;
}
TQueue* ageP(TList *a) {
    int n;
    TList **arr;
    TList *cur;
    int i;
    int x;
    int y;
    TList *tmp;
    TQueue *q;
    n = listLength(a);
    if (n == 0) return newQueue();
    arr = (TList**)malloc(n * sizeof(TList*));
    cur = a;
    i = 0;
    while (cur) {
        arr[i++] = cur;
        cur = cur->next;
    }
    for (x = 0; x < n - 1; x++)
        for (y = x + 1; y < n; y++) {
            if (personalityAge(arr[x]->dob, arr[x]->dod) > personalityAge(arr[y]->dob, arr[y]->dod)) {
                tmp = arr[x];
                arr[x] = arr[y];
                arr[y] = tmp;
            }
        }
    q = newQueue();
    for (x = 0; x < n; x++)
        enqueue(q, arr[x]);
    free(arr);
    return q;
}
TQueue* toQueue(TList *merged) {
    TQueue *q;
    TList *start;
    TList *cur;
    q = newQueue();
    if (!merged) return q;
    start = merged;
    cur = merged;
    do {
        enqueue(q, cur);
        cur = cur->next;
    } while (cur && cur != start);
    return q;
}