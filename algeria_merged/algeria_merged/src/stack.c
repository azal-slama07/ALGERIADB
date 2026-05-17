#include "stack.h"
#include "linked_list.h"

/* ========== Helpers ========== */
TStack* pushStackRaw(TStack *top, const char *name, const char *def,
                     const char *dob, const char *dod) {
    TStack *n = (TStack*)calloc(1, sizeof(TStack));
    strncpy(n->name,       name ? name : "", MAX_NAME-1);
    strncpy(n->definition, def  ? def  : "", MAX_DEF-1);
    strncpy(n->dob,        dob  ? dob  : "", MAX_DATE-1);
    strncpy(n->dod,        dod  ? dod  : "", MAX_DATE-1);
    n->next = top;
    return n;
}
TStack* pushStack(TStack *top, TList *node) {
    TStack *n = pushStackRaw(top, node->name, node->definition, node->dob, node->dod);
    n->type = node->type;
    return n;
}
TStack* popStack(TStack **top) {
    if (!*top) return NULL;
    TStack *n = *top; *top = (*top)->next; n->next = NULL; return n;
}
void printStack(TStack *stk) {
    if (!stk) { printf("  (empty stack)\n"); return; }
    int i=1;
    while(stk){
        printf("  [%d] %s  DoB:%s  DoD:%s\n", i++, stk->name, stk->dob, stk->dod);
        stk=stk->next;
    }
}
void freeStack(TStack *stk) {
    while(stk){ TStack *t=stk->next; free(stk); stk=t; }
}
int stackSize(TStack *stk) {
    int c=0; while(stk){c++;stk=stk->next;} return c;
}

/* ========== Construction ========== */
TStack* toStack(TList *merged) {
    TStack *top = NULL;
    TList *cur = merged, *start = merged;
    if (!cur) return NULL;
    do {
        top = pushStack(top, cur);
        cur = cur->next;
    } while (cur && cur != start);
    return top;
}

/* ========== Search ========== */
TStack* getInfoPersonality(TStack *stk, const char *name) {
    while (stk) {
        if (strcasecmp(stk->name, name) == 0) {
            printf("  Name: %s\n  DoB : %s\n  DoD : %s\n  Def : %s\n",
                   stk->name, stk->dob, stk->dod, stk->definition);
            return stk;
        }
        stk = stk->next;
    }
    printf("  '%s' not found in stack.\n", name);
    return NULL;
}

/* ========== Sort (alphabetical) ========== */
TStack* sortNameStack(TStack *s) {
    /* collect to array, insertion sort, rebuild */
    int n = stackSize(s);
    if (n <= 1) return s;
    TStack **arr = (TStack**)malloc(n * sizeof(TStack*));
    TStack *cur = s; int i=0;
    while(cur){ arr[i++]=cur; TStack *t=cur->next; cur->next=NULL; cur=t; }
    for(int x=0;x<n-1;x++) for(int y=x+1;y<n;y++)
        if(strcasecmp(arr[x]->name,arr[y]->name)>0){TStack *tmp=arr[x];arr[x]=arr[y];arr[y]=tmp;}
    for(int x=0;x<n-1;x++) arr[x]->next=arr[x+1];
    arr[n-1]->next=NULL;
    TStack *top = arr[0]; free(arr); return top;
}

/* ========== CRUD ========== */
TStack* deleteName(TStack *stk, const char *name) {
    TStack *cur=stk, *prev2=NULL;
    while(cur){
        if(strcasecmp(cur->name,name)==0){
            if(prev2) prev2->next=cur->next; else stk=cur->next;
            free(cur); printf("  Deleted '%s' from stack.\n",name); return stk;
        }
        prev2=cur; cur=cur->next;
    }
    printf("  '%s' not found.\n",name);
    return stk;
}
TStack* updateStack(TStack *stk, const char *name, const char *def,
                    const char *DoB, const char *DoD) {
    TStack *cur=stk;
    while(cur){
        if(strcasecmp(cur->name,name)==0){
            if(def&&def[0])  strncpy(cur->definition,def,MAX_DEF-1);
            if(DoB&&DoB[0])  strncpy(cur->dob,DoB,MAX_DATE-1);
            if(DoD&&DoD[0])  strncpy(cur->dod,DoD,MAX_DATE-1);
            printf("  Updated '%s' in stack.\n",name); return stk;
        }
        cur=cur->next;
    }
    printf("  '%s' not found.\n",name);
    return stk;
}
TStack* addNameStack(TStack *stk, const char *name, const char *definition,
                     const char *DoB, const char *DoD) {
    /* insert in alphabetical order */
    TStack *n = pushStackRaw(NULL, name, definition, DoB, DoD);
    if (!stk || strcasecmp(name, stk->name) <= 0) {
        n->next = stk; return n;
    }
    TStack *cur = stk;
    while(cur->next && strcasecmp(name, cur->next->name) > 0) cur=cur->next;
    n->next = cur->next; cur->next = n;
    return stk;
}

/* ========== Conversions ========== */
TQueue* stackToQueue(TStack *stk) {
    TStack *sorted = sortNameStack(stk);
    TQueue *q = newQueue();
    /* build a temporary TList node to reuse enqueue */
    while(sorted){
        TList tmp; memset(&tmp,0,sizeof(tmp));
        strncpy(tmp.name,sorted->name,MAX_NAME-1);
        strncpy(tmp.dob,sorted->dob,MAX_DATE-1);
        strncpy(tmp.dod,sorted->dod,MAX_DATE-1);
        strncpy(tmp.definition,sorted->definition,MAX_DEF-1);
        enqueue(q,&tmp);
        sorted=sorted->next;
    }
    return q;
}
TList* stackToList(TStack *stk) {
    TStack *sorted = sortNameStack(stk);
    TList *head = NULL;
    while(sorted){
        appendNode(&head, newNode(sorted->name,sorted->dob,sorted->dod,sorted->definition));
        sorted=sorted->next;
    }
    /* fix prev pointers */
    TList *cur=head;
    while(cur&&cur->next){cur->next->prev=cur;cur=cur->next;}
    return head;
}

/* ========== Special ========== */
TStack* definitionStack(TStack *stk) {
    int n=stackSize(stk); if(n<=1) return stk;
    TStack **arr=(TStack**)malloc(n*sizeof(TStack*));
    TStack *cur=stk; int i=0;
    while(cur){arr[i++]=cur;TStack *t=cur->next;cur->next=NULL;cur=t;}
    for(int x=0;x<n-1;x++) for(int y=x+1;y<n;y++){
        int w1=0,w2=0; char t1[MAX_DEF],t2[MAX_DEF];
        strncpy(t1,arr[x]->definition,MAX_DEF-1); strncpy(t2,arr[y]->definition,MAX_DEF-1);
        char *tk=strtok(t1," ");while(tk){w1++;tk=strtok(NULL," ");}
        tk=strtok(t2," ");while(tk){w2++;tk=strtok(NULL," ");}
        if(w1>w2){TStack *tmp=arr[x];arr[x]=arr[y];arr[y]=tmp;}
    }
    for(int x=0;x<n-1;x++) arr[x]->next=arr[x+1];
    arr[n-1]->next=NULL; TStack *top=arr[0]; free(arr); return top;
}
TStack* pronunciationStack(TStack *stk) {
    /* split into short (<= 5 words) and long stacks */
    TStack *shortS=NULL, *longS=NULL;
    while(stk){
        TStack *next=stk->next; stk->next=NULL;
        char buf[MAX_DEF]; strncpy(buf,stk->definition,MAX_DEF-1);
        int wc=0; char *tk=strtok(buf," "); while(tk){wc++;tk=strtok(NULL," ");}
        if(wc<=5){stk->next=shortS;shortS=stk;}
        else{stk->next=longS;longS=stk;}
        stk=next;
    }
    /* merge short then long */
    TStack *top=shortS;
    if(!top){return longS;}
    TStack *tail=top; while(tail->next) tail=tail->next;
    tail->next=longS;
    return top;
}
char* getSmallest(TStack *stk) {
    if(!stk) return NULL;
    TStack *min=stk; stk=stk->next;
    while(stk){
        if(strlen(stk->definition)<strlen(min->definition)) min=stk;
        stk=stk->next;
    }
    printf("  Smallest definition: [%s] -> %s\n", min->name, min->definition);
    return min->definition;
}
void continuousSearch(TStack *stk) {
    printf("  Overlapping events:\n");
    TStack *a=stk;
    while(a){
        int ya1=dateToYear(a->dob), ya2=dateToYear(a->dod);
        if(ya1==0) {a=a->next; continue;}
        TStack *b=a->next;
        while(b){
            int yb1=dateToYear(b->dob), yb2=dateToYear(b->dod);
            if(yb1==0){b=b->next;continue;}
            int aEnd=ya2?ya2:ya1, bEnd=yb2?yb2:yb1;
            if(ya1<=bEnd && aEnd>=yb1)
                printf("    '%s' overlaps with '%s'\n",a->name,b->name);
            b=b->next;
        }
        a=a->next;
    }
}
bool isPersonalityKilled(const char *word) {
    const char *keywords[] = {"killed","executed","assassinated","martyred","murdered",NULL};
    char low[MAX_DEF]; strncpy(low,word,MAX_DEF-1); low[MAX_DEF-1]='\0';
    for(int i=0;low[i];i++) low[i]=(char)tolower((unsigned char)low[i]);
    for(int i=0;keywords[i];i++)
        if(strstr(low,keywords[i])) return true;
    return false;
}
TStack* recRevHelper(TStack *stk, TStack *prev) {
    if(!stk) return prev;
    TStack *next=stk->next; stk->next=prev;
    return recRevHelper(next, stk);
}
TStack* recRevStack(TStack *stk) { return recRevHelper(stk, NULL); }
