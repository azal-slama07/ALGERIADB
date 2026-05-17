#include "raylib.h"
#include "types.h"
#include "file_parser.h"
#include "linked_list.h"
#include "stack.h"
#include "tree.h"
#include "recursion.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_W        1200
#define SCREEN_H        720
#define FONT_SIZE       17
#define LINE_H          22
#define PAD             12
#define SIDEBAR_W       232
#define MAX_RESULT_LINES 512
#define MAX_LINE_LEN     512

#define COL_BG        CLITERAL(Color){13,  13,  28,  255}
#define COL_PANEL     CLITERAL(Color){22,  22,  50,  255}
#define COL_SIDEBAR   CLITERAL(Color){18,  18,  42,  255}
#define COL_BTN       CLITERAL(Color){38,  76, 155,  255}
#define COL_BTN_HOV   CLITERAL(Color){58, 108, 210,  255}
#define COL_BTN_ACT   CLITERAL(Color){28,  56, 120,  255}
#define COL_ACCENT    CLITERAL(Color){80, 180, 255,  255}
#define COL_GREEN     CLITERAL(Color){55, 195,  95,  255}
#define COL_RED       CLITERAL(Color){215, 65,  65,  255}
#define COL_GOLD      CLITERAL(Color){215,175,  55,  255}
#define COL_TEXT      CLITERAL(Color){218,218,232,  255}
#define COL_DIMTEXT   CLITERAL(Color){125,125,155,  255}
#define COL_INPUT_BG  CLITERAL(Color){28,  28,  62,  255}
#define COL_INPUT_ACT CLITERAL(Color){33,  48,  88,  255}
#define COL_BORDER    CLITERAL(Color){55,  55,  95,  255}
#define COL_ROW_A     CLITERAL(Color){26,  26,  56,  255}
#define COL_ROW_B     CLITERAL(Color){20,  20,  46,  255}

/* ---- font ---- */
static Font gFont;
static bool gFontLoaded = false;

static void TF(const char *text, int x, int y, int size, Color color) {
    if (gFontLoaded)
        DrawTextEx(gFont, text, (Vector2){(float)x,(float)y}, (float)size, 1.0f, color);
    else
        DrawText(text, x, y, size, color);
}
static int TM(const char *text, int size) {
    if (gFontLoaded)
        return (int)MeasureTextEx(gFont, text, (float)size, 1.0f).x;
    return MeasureText(text, size);
}

/* ---- global data ---- */
#define DATA_FILE "data/history.txt"
FILE   *gFile          = NULL;
TList  *gPersonalities = NULL;
TList  *gDates         = NULL;
TList  *gEvents        = NULL;
TStack *gStack         = NULL;
TTree  *gTree          = NULL;

/* ---- result buffer ---- */
char resultLines[MAX_RESULT_LINES][MAX_LINE_LEN];
int  resultCount  = 0;
int  resultScroll = 0;

void resBuf(const char *fmt, ...) {
    if (resultCount >= MAX_RESULT_LINES) return;
    va_list a; va_start(a,fmt);
    vsnprintf(resultLines[resultCount++], MAX_LINE_LEN, fmt, a);
    va_end(a);
}
void resClear() { resultCount=0; resultScroll=0; }

/* ---- reload ---- */
void reloadData() {
    if (gFile) fclose(gFile);
    gFile = fopen(DATA_FILE,"r+");
    if (!gFile) gFile = fopen(DATA_FILE,"w+");
    freeList(gPersonalities); freeList(gDates); freeList(gEvents);
    freeStack(gStack); freeTree(gTree);
    gPersonalities = getPersonality(gFile);
    gDates         = getDatePersonality(gFile);
    gEvents        = getEvents(gFile);
    TList *merged  = mergeNodes(gPersonalities, gDates);
    gStack         = toStack(merged);
    gTree          = toTree(gStack);
    freeList(merged);
}

/* ---- deep copy helpers ---- */
TList* copyList(TList *h) {
    TList *r=NULL, *tail=NULL;
    while(h){
        TList *n=newNode(h->name,h->dob,h->dod,h->definition);
        n->type=h->type;
        if(!r){r=n;tail=n;} else{tail->next=n;n->prev=tail;tail=n;}
        h=h->next;
    }
    return r;
}
TStack* copyStack(TStack *s){
    int n=0; TStack *c=s;
    while(c){n++;c=c->next;}
    if(!n) return NULL;
    TStack **a=(TStack**)malloc(n*sizeof(TStack*));
    c=s; int i=n-1;
    while(c){a[i--]=c;c=c->next;}
    TStack *top=NULL;
    for(int j=0;j<n;j++){
        TStack *nd=(TStack*)calloc(1,sizeof(TStack));
        strncpy(nd->name,a[j]->name,MAX_NAME-1);
        strncpy(nd->definition,a[j]->definition,MAX_DEF-1);
        strncpy(nd->dob,a[j]->dob,MAX_DATE-1);
        strncpy(nd->dod,a[j]->dod,MAX_DATE-1);
        nd->type=a[j]->type; nd->next=top; top=nd;
    }
    free(a); return top;
}

/* ---- truncate helper ---- */
static const char* trunc90(const char *s){
    static char _t[256];
    if((int)strlen(s)<=90) return s;
    strncpy(_t,s,90); _t[90]='\0'; strcat(_t,"..."); return _t;
}

/* ---- dump helpers ---- */
void dumpList(TList *h){
    int i=1;
    if(!h){resBuf("  (empty list)");return;}
    TList *start=h;
    while(h){
        resBuf("[%d]  %s",i,h->name);
        if(h->dob[0]) resBuf("     %s : %s",(h->type==1?"Date":"Born"),h->dob);
        if(h->dod[0]) resBuf("     Died : %s",h->dod);
        if(h->definition[0]) resBuf("     Info : %s",trunc90(h->definition));
        resBuf(" ");
        h=h->next; i++;
        if(h==start) break;
    }
}
void dumpStack(TStack *s){
    int i=1;
    if(!s){resBuf("  (empty stack)");return;}
    while(s){
        resBuf("[%d]  %s",i,s->name);
        if(s->dob[0]) resBuf("     Born : %s",s->dob);
        if(s->dod[0]) resBuf("     Died : %s",s->dod);
        if(s->definition[0]) resBuf("     Info : %s",trunc90(s->definition));
        resBuf(" ");
        s=s->next; i++;
    }
}
void dumpQueue(TQueue *q){
    TQueueNode *c; int i=1;
    if(!q||!q->head){resBuf("  (empty queue)");return;}
    c=q->head;
    while(c){
        resBuf("[%d]  %s",i,c->name);
        if(c->dob[0]) resBuf("     Born : %s",c->dob);
        if(c->dod[0]) resBuf("     Died : %s",c->dod);
        resBuf(" ");
        c=c->next; i++;
    }
}
void dumpTree(TTree *t){
    if(!t) return;
    dumpTree(t->left);
    resBuf("[N]  %s",t->name);
    if(t->dob[0]) resBuf("     Born : %s",t->dob);
    if(t->dod[0]) resBuf("     Died : %s",t->dod);
    resBuf(" ");
    dumpTree(t->right);
}
void dumpTreePre(TTree *t){
    if(!t) return;
    resBuf("[N]  %s",t->name);
    if(t->dob[0]) resBuf("     Born : %s",t->dob);
    if(t->dod[0]) resBuf("     Died : %s",t->dod);
    resBuf(" ");
    dumpTreePre(t->left); dumpTreePre(t->right);
}
void dumpTreePost(TTree *t){
    if(!t) return;
    dumpTreePost(t->left); dumpTreePost(t->right);
    resBuf("[N]  %s",t->name);
    if(t->dob[0]) resBuf("     Born : %s",t->dob);
    if(t->dod[0]) resBuf("     Died : %s",t->dod);
    resBuf(" ");
}

/* ---- UI primitives ---- */
int drawButton(int x,int y,int w,int h,const char *lbl,Color base){
    Rectangle r={(float)x,(float)y,(float)w,(float)h};
    Vector2 mp=GetMousePosition();
    int hov=CheckCollisionPointRec(mp,r);
    int clk=hov&&IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
    Color col=hov?COL_BTN_HOV:base;
    if(hov&&IsMouseButtonDown(MOUSE_LEFT_BUTTON)) col=COL_BTN_ACT;
    DrawRectangleRounded(r,0.22f,6,col);
    DrawRectangleRoundedLines(r,0.22f,6,1.5f,COL_BORDER);
    int tw=TM(lbl,FONT_SIZE-2);
    TF(lbl,x+w/2-tw/2,y+h/2-(FONT_SIZE-2)/2,FONT_SIZE-2,COL_TEXT);
    return clk;
}

typedef struct{char buf[MAX_LINE_LEN];int len;int active;}InputBox;
void initInput(InputBox *b){b->buf[0]='\0';b->len=0;b->active=0;}

void drawInput(InputBox *b,int x,int y,int w,int h,const char *hint){
    Rectangle r={(float)x,(float)y,(float)w,(float)h};
    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        b->active=CheckCollisionPointRec(GetMousePosition(),r);
    DrawRectangleRounded(r,0.15f,6,b->active?COL_INPUT_ACT:COL_INPUT_BG);
    DrawRectangleRoundedLines(r,0.15f,6,1.5f,b->active?COL_ACCENT:COL_BORDER);
    if(b->active){
        int k=GetCharPressed();
        while(k>0){
            if(k>=32&&k<127&&b->len<MAX_LINE_LEN-2)
                b->buf[b->len++]=(char)k;
            b->buf[b->len]='\0';
            k=GetCharPressed();
        }
        if(IsKeyPressed(KEY_BACKSPACE)&&b->len>0) b->buf[--b->len]='\0';
    }
    const char *disp=(b->len==0&&!b->active)?hint:b->buf;
    Color tc=(b->len==0&&!b->active)?COL_DIMTEXT:COL_TEXT;
    TF(disp,x+PAD/2,y+h/2-FONT_SIZE/2,FONT_SIZE,tc);
    if(b->active&&((int)(GetTime()*2)%2==0)){
        int cx=x+PAD/2+TM(b->buf,FONT_SIZE);
        DrawLine(cx,y+4,cx,y+h-4,COL_ACCENT);
    }
}

void drawResultPanel(int x,int y,int w,int h){
    Rectangle r={(float)x,(float)y,(float)w,(float)h};
    DrawRectangleRounded(r,0.04f,4,COL_PANEL);
    DrawRectangleRoundedLines(r,0.04f,4,1.5f,COL_BORDER);

    if(CheckCollisionPointRec(GetMousePosition(),r)){
        float wh=GetMouseWheelMove();
        if(wh!=0){
            resultScroll-=(int)wh*3;
            if(resultScroll<0) resultScroll=0;
            int mx=resultCount-(h-PAD*2)/LINE_H;
            if(mx<0)mx=0;
            if(resultScroll>mx) resultScroll=mx;
        }
    }

    BeginScissorMode(x+2,y+2,w-12,h-4);
    int vy=y+PAD, visH=h-PAD*2, skip=resultScroll;
    for(int i=0;i<resultCount;i++){
        if(skip>0){skip--;continue;}
        if(vy-y>visH) break;
        const char *line=resultLines[i];
        DrawRectangle(x+2,vy-2,w-14,LINE_H,(i%2==0)?COL_ROW_A:COL_ROW_B);
        Color tc=COL_TEXT;
        if(line[0]=='[')                   tc=COL_ACCENT;
        if(strncmp(line,"     Born",9)==0) tc=COL_GREEN;
        if(strncmp(line,"     Date",9)==0) tc=COL_GREEN;
        if(strncmp(line,"     Died",9)==0) tc=COL_RED;
        if(strncmp(line,"     Info",9)==0) tc=COL_GOLD;
        TF(line,x+PAD,vy,FONT_SIZE,tc);
        vy+=LINE_H;
    }
    EndScissorMode();

    int vis=(h-PAD*2)/LINE_H;
    if(resultCount>vis){
        int sbH=h-PAD*2;
        int thH=sbH*vis/resultCount; if(thH<16)thH=16;
        int thY=y+PAD+(resultCount>vis?(sbH-thH)*resultScroll/(resultCount-vis):0);
        DrawRectangle(x+w-8,y+PAD,4,sbH,COL_BORDER);
        DrawRectangle(x+w-8,thY,4,thH,COL_ACCENT);
    }
}

void drawTitle(int x,int y,const char *t){
    TF(t,x,y,FONT_SIZE+2,COL_ACCENT);
    DrawLine(x,y+FONT_SIZE+5,x+TM(t,FONT_SIZE+2)+30,y+FONT_SIZE+5,COL_ACCENT);
}
void drawLabel(int x,int y,const char *t){TF(t,x,y,FONT_SIZE-2,COL_DIMTEXT);}

/* ================================================================
   SCREEN ENUM
   ================================================================ */
typedef enum{SCR_HOME=0,SCR_LIST,SCR_STACK,SCR_BST,SCR_RECURSION}Screen;
Screen currentScreen=SCR_HOME;

/* ================================================================
   HOME
   ================================================================ */
void drawHome(){
    const char *l1="Second semester Project";
    TF(l1,SCREEN_W/2-TM(l1,16)/2,62,16,COL_DIMTEXT);
    const char *l2="History of Algeria Database";
    TF(l2,SCREEN_W/2-TM(l2,34)/2,82,34,COL_ACCENT);
    const char *l3="using Dynamic Data Structures";
    TF(l3,SCREEN_W/2-TM(l3,22)/2,120,22,COL_GOLD);
    const char *l4="Level: 1st Year   |   Material: Algorithms and Dynamic Data Structures   |   NSCS 2025/2026";
    TF(l4,SCREEN_W/2-TM(l4,13)/2,148,13,COL_DIMTEXT);
    DrawLine(SCREEN_W/2-400,168,SCREEN_W/2+400,168,COL_BORDER);

    int pC=listLength(gPersonalities),eC=listLength(gEvents);
    int sC=stackSize(gStack),tS=treeSize(gTree);
    struct{const char*lbl;int v;Color c;}stats[]={
        {"Personalities",pC,COL_ACCENT},{"Events",eC,COL_GREEN},
        {"Stack nodes",sC,COL_GOLD},{"BST nodes",tS,COL_RED}};
    for(int i=0;i<4;i++){
        int bx=SCREEN_W/2-470+i*245, by=180;
        Rectangle r={(float)bx,(float)by,225,82};
        DrawRectangleRounded(r,0.2f,6,COL_PANEL);
        DrawRectangleRoundedLines(r,0.2f,6,1.5f,stats[i].c);
        char num[16]; sprintf(num,"%d",stats[i].v);
        TF(num,bx+112-TM(num,32)/2,by+10,32,stats[i].c);
        TF(stats[i].lbl,bx+112-TM(stats[i].lbl,FONT_SIZE-2)/2,by+54,FONT_SIZE-2,COL_DIMTEXT);
    }

    struct{const char*lbl;Screen sc;}btns[]={
        {"Linked List & Queues",SCR_LIST},{"Stack",SCR_STACK},
        {"Binary Search Tree",SCR_BST},{"Recursion",SCR_RECURSION}};
    for(int i=0;i<4;i++){
        int bx=SCREEN_W/2-470+i*245;
        if(drawButton(bx,284,225,48,btns[i].lbl,COL_BTN))
            currentScreen=btns[i].sc;
    }

    TF("Data file: data/history.txt",PAD,SCREEN_H-26,FONT_SIZE-4,COL_DIMTEXT);
    if(drawButton(SCREEN_W-170,SCREEN_H-44,150,34,"Reload Data",COL_BTN)){
        reloadData(); resClear();
        resBuf("Data reloaded: %d personalities, %d events.",
               listLength(gPersonalities),listLength(gEvents));
    }
}

/* ================================================================
   LINKED LIST
   ================================================================ */
typedef enum{
    LL_NONE=0,
    LL_GET_PERS,        /* getPersonality      */
    LL_GET_DATE_PERS,   /* getDatePersonality  */
    LL_GET_EVT,         /* getEvents           */
    LL_INFO_DOB,        /* getInfoByDates      */
    LL_INFO_DOD,        /* getInfoByDates2     */
    LL_SORT_WORD,       /* sortWord            */
    LL_SORT_WORD2,      /* sortWord2           */
    LL_SORT_PERS,       /* sortPersonality     */
    LL_ADD_PERS,        /* addPersonality      */
    LL_ADD_EVT,         /* addEvents           */
    LL_DEL_PERS,        /* deletePersonality   */
    LL_UPD_PERS,        /* updatePersonality   */
    LL_SIMILAR,         /* similarPersonality  */
    LL_COUNT,           /* countPersonality    */
    LL_PALIN,           /* palindromeName      */
    LL_MERGE,           /* mergeNodes          */
    LL_MERGE2,          /* merge2Nodes         */
    LL_TO_QUEUE,        /* toQueue             */
    LL_SNAME,           /* sName               */
    LL_AGEP             /* ageP                */
}LLAction;

LLAction llAction=LL_NONE;
InputBox llIn1,llIn2,llIn3,llIn4;
static int llScroll=0;

void drawLinkedList(){
    int sx=PAD,sy=56,sw=SIDEBAR_W,sh=27,gap=3;
    struct{const char*lbl;LLAction act;}items[]={
        {"getPersonality",     LL_GET_PERS},
        {"getDatePersonality", LL_GET_DATE_PERS},
        {"getEvents",          LL_GET_EVT},
        {"getInfoByDates",     LL_INFO_DOB},
        {"getInfoByDates2",    LL_INFO_DOD},
        {"sortWord",           LL_SORT_WORD},
        {"sortWord2",          LL_SORT_WORD2},
        {"sortPersonality",    LL_SORT_PERS},
        {"addPersonality",     LL_ADD_PERS},
        {"addEvents",          LL_ADD_EVT},
        {"deletePersonality",  LL_DEL_PERS},
        {"updatePersonality",  LL_UPD_PERS},
        {"similarPersonality", LL_SIMILAR},
        {"countPersonality",   LL_COUNT},
        {"palindromeName",     LL_PALIN},
        {"mergeNodes",         LL_MERGE},
        {"merge2Nodes",        LL_MERGE2},
        {"toQueue",            LL_TO_QUEUE},
        {"sName",              LL_SNAME},
        {"ageP",               LL_AGEP},
    };
    int n=20,slotH=sh+gap,sideH=SCREEN_H-sy-PAD,totH=n*slotH;

    DrawRectangle(sx-4,sy,sw+8,sideH,COL_SIDEBAR);

    Rectangle sr={(float)sx,(float)sy,(float)sw,(float)sideH};
    if(CheckCollisionPointRec(GetMousePosition(),sr)){
        float wh=GetMouseWheelMove();
        if(wh!=0){
            llScroll-=(int)(wh*slotH*3);
            if(llScroll<0)llScroll=0;
            int mx=totH-sideH; if(mx<0)mx=0;
            if(llScroll>mx)llScroll=mx;
        }
    }
    if(llScroll>0) TF("^ up",sx+4,sy+2,12,COL_ACCENT);
    if(totH-llScroll>sideH) TF("v down",sx+4,SCREEN_H-PAD-13,12,COL_ACCENT);

    BeginScissorMode(sx,sy+2,sw,sideH-4);
    for(int i=0;i<n;i++){
        int by=sy+i*slotH-llScroll;
        if(by+sh<sy||by>sy+sideH) continue;
        Color col=(llAction==items[i].act)?COL_BTN_ACT:COL_BTN;
        if(drawButton(sx,by,sw,sh,items[i].lbl,col)){
            llAction=items[i].act; resClear();
            initInput(&llIn1);initInput(&llIn2);
            initInput(&llIn3);initInput(&llIn4);

            /* immediate no-input actions */
            if(llAction==LL_GET_PERS){
                TList *c=copyList(gPersonalities); dumpList(c); freeList(c); llAction=LL_NONE;
            }
            if(llAction==LL_GET_DATE_PERS){
                TList *c=copyList(gDates); dumpList(c); freeList(c); llAction=LL_NONE;
            }
            if(llAction==LL_GET_EVT){
                TList *c=copyList(gEvents); dumpList(c); freeList(c); llAction=LL_NONE;
            }
            if(llAction==LL_SORT_WORD){
                TList *c=copyList(gPersonalities);
                TList *s=sortWord(c); dumpList(s); freeList(s); llAction=LL_NONE;
            }
            if(llAction==LL_SORT_WORD2){
                TList *c=copyList(gPersonalities);
                TList *s=sortWord2(c); dumpList(s); freeList(s); llAction=LL_NONE;
            }
            if(llAction==LL_SORT_PERS){
                TList *c=copyList(gPersonalities);
                TList *s=sortPersonality(c); dumpList(s); freeList(s); llAction=LL_NONE;
            }
            if(llAction==LL_PALIN){
                TList *c=copyList(gPersonalities);
                TList *s=palindromeName(c); dumpList(s); freeList(s); freeList(c); llAction=LL_NONE;
            }
            if(llAction==LL_MERGE){
                TList *m=mergeNodes(gPersonalities,gDates);
                dumpList(m); freeList(m); llAction=LL_NONE;
            }
            if(llAction==LL_MERGE2){
                TList *m=merge2Nodes(gPersonalities,gDates);
                if(m){
                    TList *cur=m,*start=m; int lim=0,idx=1;
                    do{
                        resBuf("[%d]  %s  Born:%s  Died:%s",idx++,cur->name,cur->dob,cur->dod);
                        cur=cur->next; lim++;
                    }while(cur&&cur!=start&&lim<300);
                    resBuf("(circular — %d nodes shown)",lim);
                    TList *x=m;
                    do{TList*nx=x->next;if(nx==start){x->next=NULL;break;}x=nx;}while(x&&x!=start);
                    freeList(m);
                }else resBuf("(empty)");
                llAction=LL_NONE;
            }
            if(llAction==LL_TO_QUEUE){
                TList *m=mergeNodes(gPersonalities,gDates);
                if(m){TQueue *q=toQueue(m); dumpQueue(q); freeQueue(q); freeList(m);}
                else resBuf("(no data loaded)");
                llAction=LL_NONE;
            }
            if(llAction==LL_SNAME){
                if(gPersonalities){
                    TList *c=copyList(gPersonalities);
                    TQueue *q=sName(c);
                    if(q){dumpQueue(q); freeQueue(q);}
                    freeList(c);
                }else resBuf("No personalities loaded.");
                llAction=LL_NONE;
            }
            if(llAction==LL_AGEP){
                if(gDates){
                    TList *c=copyList(gDates);
                    TQueue *q=ageP(c);
                    if(q){dumpQueue(q); freeQueue(q);}
                    freeList(c);
                }else resBuf("No date data loaded.");
                llAction=LL_NONE;
            }
        }
    }
    EndScissorMode();

    int cx=sx+sw+PAD*2, cy=60, cw=SCREEN_W-cx-PAD;
    int fb=cy;

    if(llAction==LL_INFO_DOB||llAction==LL_INFO_DOD){
        const char*ttl=(llAction==LL_INFO_DOB)?"getInfoByDates — Search by Birth Date"
                                               :"getInfoByDates2 — Search by Death Date";
        drawTitle(cx,cy,ttl);
        drawLabel(cx,cy+36,(llAction==LL_INFO_DOB)?"Birth date (e.g. 1923 or 01/01/1923)":"Death date (e.g. 04/03/1957)");
        drawInput(&llIn1,cx,cy+54,cw,32,"date...");
        if(drawButton(cx,cy+96,120,32,"Search",COL_BTN)){
            resClear();
            TList *cur=gPersonalities; int found=0;
            while(cur){
                const char*d=(llAction==LL_INFO_DOB)?cur->dob:cur->dod;
                if(strcmp(d,llIn1.buf)==0){
                    resBuf("[Found]  %s",cur->name);
                    resBuf("     Born : %s",cur->dob);
                    resBuf("     Died : %s",cur->dod);
                    resBuf("     Info : %s",trunc90(cur->definition));
                    resBuf(" "); found++;
                }
                cur=cur->next;
            }
            if(!found) resBuf("No entry found for: %s",llIn1.buf);
        }
        fb=cy+140;
    }
    if(llAction==LL_SIMILAR){
        drawTitle(cx,cy,"similarPersonality — Find by Year");
        drawLabel(cx,cy+36,"Year of birth or death:");
        drawInput(&llIn1,cx,cy+54,240,32,"e.g. 1930");
        if(drawButton(cx,cy+96,120,32,"Search",COL_BTN)){
            resClear();
            TList *s=similarPersonality(gPersonalities,llIn1.buf);
            dumpList(s); freeList(s);
        }
        fb=cy+140;
    }
    if(llAction==LL_COUNT){
        drawTitle(cx,cy,"countPersonality — Count by Date");
        drawLabel(cx,cy+36,"Year (e.g. 1954):");
        drawInput(&llIn1,cx,cy+54,240,32,"e.g. 1830");
        if(drawButton(cx,cy+96,120,32,"Count",COL_BTN)){
            resClear();
            Date d; d.day=0;d.month=0;d.year=atoi(llIn1.buf);
            TList *s=countPersonality(gPersonalities,&d);
            resBuf("Found %d personalities for year %s:",listLength(s),llIn1.buf);
            resBuf(" "); dumpList(s); freeList(s);
        }
        fb=cy+140;
    }
    if(llAction==LL_ADD_PERS){
        drawTitle(cx,cy,"addPersonality — Add Personality");
        drawLabel(cx,cy+32,"Full name:"); drawInput(&llIn1,cx,cy+48,cw,30,"e.g. Amirouche Ait Hamouda");
        drawLabel(cx,cy+84,"Date of birth:"); drawInput(&llIn2,cx,cy+100,cw,30,"e.g. 1926");
        drawLabel(cx,cy+136,"Date of death:"); drawInput(&llIn3,cx,cy+152,cw,30,"e.g. 29/03/1959");
        drawLabel(cx,cy+188,"Definition:"); drawInput(&llIn4,cx,cy+204,cw,30,"...");
        if(drawButton(cx,cy+244,130,32,"Add",COL_GREEN)){
            if(llIn1.len>0){
                gPersonalities=addPersonality(gPersonalities,gDates,
                    llIn1.buf,llIn2.buf,llIn3.buf,llIn4.buf,DATA_FILE);
                resClear(); resBuf("Added: %s",llIn1.buf);
                initInput(&llIn1);initInput(&llIn2);initInput(&llIn3);initInput(&llIn4);
            }else{resClear();resBuf("Please enter a name.");}
        }
        fb=cy+286;
    }
    if(llAction==LL_ADD_EVT){
        drawTitle(cx,cy,"addEvents — Add Event");
        drawLabel(cx,cy+32,"Event name:"); drawInput(&llIn1,cx,cy+48,cw,30,"e.g. Battle of Issafen");
        drawLabel(cx,cy+84,"Date:"); drawInput(&llIn2,cx,cy+100,cw,30,"e.g. 22/04/1958");
        drawLabel(cx,cy+136,"Description:"); drawInput(&llIn3,cx,cy+152,cw,30,"Description...");
        if(drawButton(cx,cy+192,130,32,"Add",COL_GREEN)){
            if(llIn1.len>0){
                gEvents=addEvents(gEvents,llIn1.buf,llIn2.buf,llIn3.buf,DATA_FILE);
                resClear(); resBuf("Added event: %s",llIn1.buf);
                initInput(&llIn1);initInput(&llIn2);initInput(&llIn3);
            }else{resClear();resBuf("Please enter a name.");}
        }
        fb=cy+236;
    }
    if(llAction==LL_DEL_PERS){
        drawTitle(cx,cy,"deletePersonality — Delete");
        drawLabel(cx,cy+36,"Exact name:"); drawInput(&llIn1,cx,cy+54,cw,32,"Name...");
        if(drawButton(cx,cy+96,140,32,"Delete",COL_RED)){
            if(llIn1.len>0){
                gPersonalities=deletePersonality(DATA_FILE,gPersonalities,gDates,llIn1.buf);
                resClear(); resBuf("Deleted: %s",llIn1.buf); initInput(&llIn1);
            }
        }
        fb=cy+140;
    }
    if(llAction==LL_UPD_PERS){
        drawTitle(cx,cy,"updatePersonality — Update");
        drawLabel(cx,cy+32,"Name (exact):"); drawInput(&llIn1,cx,cy+48,cw,30,"Name...");
        drawLabel(cx,cy+84,"New definition (blank=keep):"); drawInput(&llIn2,cx,cy+100,cw,30,"");
        drawLabel(cx,cy+136,"New DoB (blank=keep):"); drawInput(&llIn3,cx,cy+152,cw,30,"");
        drawLabel(cx,cy+188,"New DoD (blank=keep):"); drawInput(&llIn4,cx,cy+204,cw,30,"");
        if(drawButton(cx,cy+244,130,32,"Update",COL_GOLD)){
            if(llIn1.len>0){
                gPersonalities=updatePersonality(DATA_FILE,gPersonalities,gDates,
                    llIn1.buf,llIn2.buf,llIn3.buf,llIn4.buf);
                resClear(); resBuf("Updated: %s",llIn1.buf);
            }
        }
        fb=cy+286;
    }

    int rpY=fb+(fb==cy?0:12);
    int rpH=SCREEN_H-rpY-PAD;
    if(rpH>60) drawResultPanel(cx,rpY,cw,rpH);
}

/* ================================================================
   STACK
   ================================================================ */
typedef enum{
    ST_NONE=0,
    ST_TO_STACK,ST_GET_INFO,ST_SORT_NAME,ST_DEL,ST_UPDATE,
    ST_TO_QUEUE,ST_TO_LIST,ST_ADD,ST_DEF_SORT,ST_PRONUN,
    ST_SMALLEST,ST_OVERLAP,ST_KILLED,ST_REVERSE
}STAction;

STAction stAction=ST_NONE;
InputBox stIn1,stIn2,stIn3,stIn4;
static int stScroll=0;

void drawStack(){
    int sx=PAD,sy=56,sw=SIDEBAR_W,sh=27,gap=3;
    struct{const char*lbl;STAction act;}items[]={
        {"toStack",            ST_TO_STACK},
        {"getInfoPersonality", ST_GET_INFO},
        {"sortNameStack",      ST_SORT_NAME},
        {"deleteName",         ST_DEL},
        {"updateStack",        ST_UPDATE},
        {"stackToQueue",       ST_TO_QUEUE},
        {"stackToList",        ST_TO_LIST},
        {"addNameStack",       ST_ADD},
        {"definitionStack",    ST_DEF_SORT},
        {"pronunciationStack", ST_PRONUN},
        {"getSmallest",        ST_SMALLEST},
        {"continuousSearch",   ST_OVERLAP},
        {"isPersonalityKilled",ST_KILLED},
        {"recRevStack",        ST_REVERSE},
    };
    int n=14,slotH=sh+gap,sideH=SCREEN_H-sy-PAD,totH=n*slotH;

    DrawRectangle(sx-4,sy,sw+8,sideH,COL_SIDEBAR);
    Rectangle sr={(float)sx,(float)sy,(float)sw,(float)sideH};
    if(CheckCollisionPointRec(GetMousePosition(),sr)){
        float wh=GetMouseWheelMove();
        if(wh!=0){
            stScroll-=(int)(wh*slotH*3);
            if(stScroll<0)stScroll=0;
            int mx=totH-sideH; if(mx<0)mx=0;
            if(stScroll>mx)stScroll=mx;
        }
    }

    BeginScissorMode(sx,sy+2,sw,sideH-4);
    for(int i=0;i<n;i++){
        int by=sy+i*slotH-stScroll;
        if(by+sh<sy||by>sy+sideH) continue;
        Color col=(stAction==items[i].act)?COL_BTN_ACT:COL_BTN;
        if(drawButton(sx,by,sw,sh,items[i].lbl,col)){
            stAction=items[i].act; resClear();
            initInput(&stIn1);initInput(&stIn2);initInput(&stIn3);initInput(&stIn4);

            if(stAction==ST_TO_STACK) { dumpStack(gStack); stAction=ST_NONE; }
            if(stAction==ST_SORT_NAME){
                TStack *c=copyStack(gStack); c=sortNameStack(c);
                dumpStack(c); freeStack(c); stAction=ST_NONE;
            }
            if(stAction==ST_TO_QUEUE){
                TStack *c=copyStack(gStack);
                TQueue *q=stackToQueue(c); dumpQueue(q); freeQueue(q); freeStack(c); stAction=ST_NONE;
            }
            if(stAction==ST_TO_LIST){
                TStack *c=copyStack(gStack);
                TList *l=stackToList(c); dumpList(l); freeList(l); freeStack(c); stAction=ST_NONE;
            }
            if(stAction==ST_DEF_SORT){
                TStack *c=copyStack(gStack); c=definitionStack(c);
                dumpStack(c); freeStack(c); stAction=ST_NONE;
            }
            if(stAction==ST_PRONUN){
                TStack *c=copyStack(gStack);
                TStack *s2=pronunciationStack(c);
                resBuf("Short (<=5 words) then long:"); resBuf(" ");
                dumpStack(s2); freeStack(s2); stAction=ST_NONE;
            }
            if(stAction==ST_SMALLEST){
                if(gStack){
                    TStack *mn=gStack,*c=gStack->next;
                    while(c){if(strlen(c->definition)<strlen(mn->definition))mn=c;c=c->next;}
                    resBuf("Smallest definition:");
                    resBuf("  Name: %s",mn->name);
                    resBuf("  Def : %s",trunc90(mn->definition));
                }else resBuf("Stack is empty.");
                stAction=ST_NONE;
            }
            if(stAction==ST_OVERLAP){
                resBuf("continuousSearch — overlapping events:"); resBuf(" ");
                TStack *a=gStack;
                while(a){
                    int ya1=dateToYear(a->dob),ya2=dateToYear(a->dod);
                    if(ya1>0){
                        TStack *b=a->next;
                        while(b){
                            int yb1=dateToYear(b->dob),yb2=dateToYear(b->dod);
                            if(yb1>0){
                                int aE=ya2?ya2:ya1,bE=yb2?yb2:yb1;
                                if(ya1<=bE&&aE>=yb1)
                                    resBuf("  '%s'  overlaps  '%s'",a->name,b->name);
                            }
                            b=b->next;
                        }
                    }
                    a=a->next;
                }
                stAction=ST_NONE;
            }
            if(stAction==ST_REVERSE){
                TStack *c=copyStack(gStack); c=recRevStack(c);
                dumpStack(c); freeStack(c); stAction=ST_NONE;
            }
        }
    }
    EndScissorMode();

    int cx=sx+sw+PAD*2,cy=60,cw=SCREEN_W-cx-PAD,fb=cy;

    if(stAction==ST_GET_INFO){
        drawTitle(cx,cy,"getInfoPersonality — Search Stack");
        drawLabel(cx,cy+36,"Name:"); drawInput(&stIn1,cx,cy+54,cw,32,"Exact name...");
        if(drawButton(cx,cy+96,120,32,"Search",COL_BTN)){
            resClear(); TStack *nd=getInfoPersonality(gStack,stIn1.buf);
            if(nd){resBuf("Name: %s",nd->name);resBuf("Born: %s",nd->dob);
                   resBuf("Died: %s",nd->dod);resBuf("Info: %s",trunc90(nd->definition));}
        }
        fb=cy+140;
    }
    if(stAction==ST_DEL){
        drawTitle(cx,cy,"deleteName — Delete from Stack");
        drawInput(&stIn1,cx,cy+54,cw,32,"Exact name...");
        if(drawButton(cx,cy+96,120,32,"Delete",COL_RED)){
            resClear(); gStack=deleteName(gStack,stIn1.buf);
            resBuf("Done."); initInput(&stIn1);
        }
        fb=cy+140;
    }
    if(stAction==ST_ADD){
        drawTitle(cx,cy,"addNameStack — Add to Stack");
        drawLabel(cx,cy+32,"Name:"); drawInput(&stIn1,cx,cy+48,cw,30,"Name...");
        drawLabel(cx,cy+84,"Definition:"); drawInput(&stIn2,cx,cy+100,cw,30,"Info...");
        drawLabel(cx,cy+136,"Birth date:"); drawInput(&stIn3,cx,cy+152,cw,30,"DoB...");
        drawLabel(cx,cy+188,"Death date:"); drawInput(&stIn4,cx,cy+204,cw,30,"DoD...");
        if(drawButton(cx,cy+244,130,32,"Add",COL_GREEN)){
            resClear(); gStack=addNameStack(gStack,stIn1.buf,stIn2.buf,stIn3.buf,stIn4.buf);
            resBuf("Added: %s",stIn1.buf);
        }
        fb=cy+286;
    }
    if(stAction==ST_UPDATE){
        drawTitle(cx,cy,"updateStack — Update Entry");
        drawLabel(cx,cy+32,"Name:"); drawInput(&stIn1,cx,cy+48,cw,30,"Name...");
        drawLabel(cx,cy+84,"New Definition:"); drawInput(&stIn2,cx,cy+100,cw,30,"");
        drawLabel(cx,cy+136,"New DoB:"); drawInput(&stIn3,cx,cy+152,cw,30,"");
        drawLabel(cx,cy+188,"New DoD:"); drawInput(&stIn4,cx,cy+204,cw,30,"");
        if(drawButton(cx,cy+244,130,32,"Update",COL_GOLD)){
            resClear(); gStack=updateStack(gStack,stIn1.buf,stIn2.buf,stIn3.buf,stIn4.buf);
            resBuf("Updated.");
        }
        fb=cy+286;
    }
    if(stAction==ST_KILLED){
        drawTitle(cx,cy,"isPersonalityKilled — Check");
        drawLabel(cx,cy+36,"Paste definition text:");
        drawInput(&stIn1,cx,cy+54,cw,32,"Text...");
        if(drawButton(cx,cy+96,120,32,"Check",COL_BTN)){
            resClear();
            resBuf(isPersonalityKilled(stIn1.buf)?
                "Result: YES — killed/executed/assassinated.":
                "Result: NO — not mentioned as killed.");
        }
        fb=cy+140;
    }

    int rpY=fb+(fb==cy?0:12);
    int rpH=SCREEN_H-rpY-PAD;
    if(rpH>60) drawResultPanel(cx,rpY,cw,rpH);
}

/* ================================================================
   BST
   ================================================================ */
typedef enum{
    BST_NONE=0,BST_TO_TREE,BST_FILL,BST_GET_INFO,BST_ADD,BST_DELETE,BST_UPDATE,
    BST_INORDER,BST_PREORDER,BST_POSTORDER,BST_HEIGHT,BST_LCA,
    BST_RANGE,BST_SUCC,BST_MIRROR,BST_BALANCED,BST_MERGE
}BSTAction;

BSTAction bstAction=BST_NONE;
InputBox bstIn1,bstIn2,bstIn3,bstIn4;
static int bstScroll=0;

void drawBST(){
    int sx=PAD,sy=56,sw=SIDEBAR_W,sh=27,gap=3;
    struct{const char*lbl;BSTAction act;}items[]={
        {"toTree",               BST_TO_TREE},
        {"fillTree",             BST_FILL},
        {"getInfoNameTree",      BST_GET_INFO},
        {"addNameBST",           BST_ADD},
        {"deleteNameBST",        BST_DELETE},
        {"updateNameBST",        BST_UPDATE},
        {"traversalBSTinOrder",  BST_INORDER},
        {"traversalBSTpreOrder", BST_PREORDER},
        {"traversalBSTpostOrder",BST_POSTORDER},
        {"heightSizeBST",        BST_HEIGHT},
        {"lowestCommonAncestor", BST_LCA},
        {"countNodesRange",      BST_RANGE},
        {"inOrderSuccessor",     BST_SUCC},
        {"BSTMirror",            BST_MIRROR},
        {"isBalancedBST",        BST_BALANCED},
        {"BTSMerge",             BST_MERGE},
    };
    int n=16,slotH=sh+gap,sideH=SCREEN_H-sy-PAD,totH=n*slotH;

    DrawRectangle(sx-4,sy,sw+8,sideH,COL_SIDEBAR);
    Rectangle sr={(float)sx,(float)sy,(float)sw,(float)sideH};
    if(CheckCollisionPointRec(GetMousePosition(),sr)){
        float wh=GetMouseWheelMove();
        if(wh!=0){
            bstScroll-=(int)(wh*slotH*3);
            if(bstScroll<0)bstScroll=0;
            int mx=totH-sideH; if(mx<0)mx=0;
            if(bstScroll>mx)bstScroll=mx;
        }
    }
    if(totH-bstScroll>sideH) TF("v scroll",sx+4,SCREEN_H-PAD-13,12,COL_ACCENT);

    BeginScissorMode(sx,sy+2,sw,sideH-4);
    for(int i=0;i<n;i++){
        int by=sy+i*slotH-bstScroll;
        if(by+sh<sy||by>sy+sideH) continue;
        Color col=(bstAction==items[i].act)?COL_BTN_ACT:COL_BTN;
        if(drawButton(sx,by,sw,sh,items[i].lbl,col)){
            bstAction=items[i].act; resClear();
            initInput(&bstIn1);initInput(&bstIn2);initInput(&bstIn3);initInput(&bstIn4);

            if(bstAction==BST_TO_TREE){resBuf("toTree — current BST (in-order):"); resBuf(" "); dumpTree(gTree); bstAction=BST_NONE;}
            if(bstAction==BST_FILL){
                rewind(gFile); TTree *ft=fillTree(gFile);
                resBuf("fillTree (in-order):"); resBuf(" ");
                dumpTree(ft); freeTree(ft); bstAction=BST_NONE;
            }
            if(bstAction==BST_INORDER)  {resBuf("traversalBSTinOrder:"); resBuf(" "); dumpTree(gTree); bstAction=BST_NONE;}
            if(bstAction==BST_PREORDER) {resBuf("traversalBSTpreOrder:"); resBuf(" "); dumpTreePre(gTree); bstAction=BST_NONE;}
            if(bstAction==BST_POSTORDER){resBuf("traversalBSTpostOrder:"); resBuf(" "); dumpTreePost(gTree); bstAction=BST_NONE;}
            if(bstAction==BST_HEIGHT){
                resBuf("heightSizeBST:");
                resBuf("  Height : %d",treeHeight(gTree));
                resBuf("  Size   : %d",treeSize(gTree));
                bstAction=BST_NONE;
            }
            if(bstAction==BST_MIRROR){
                gTree=BSTMirror(gTree);
                resBuf("BSTMirror — mirrored. New in-order:"); resBuf(" "); dumpTree(gTree);
                bstAction=BST_NONE;
            }
            if(bstAction==BST_BALANCED){
                resBuf(isBalancedBST(gTree)?"isBalancedBST: YES":"isBalancedBST: NO");
                bstAction=BST_NONE;
            }
            if(bstAction==BST_MERGE){
                rewind(gFile); TTree *ft2=fillTree(gFile);
                TTree *mg=BTSMerge(gTree,ft2);
                resBuf("BTSMerge — balanced (in-order):"); resBuf(" ");
                dumpTree(mg); freeTree(mg); bstAction=BST_NONE;
            }
        }
    }
    EndScissorMode();

    int cx=sx+sw+PAD*2,cy=60,cw=SCREEN_W-cx-PAD,fb=cy;

    if(bstAction==BST_GET_INFO){
        drawTitle(cx,cy,"getInfoNameTree — Search BST");
        drawInput(&bstIn1,cx,cy+54,cw,32,"Name...");
        if(drawButton(cx,cy+96,120,32,"Search",COL_BTN)){
            resClear(); TTree *nd=getInfoNameTree(gTree,bstIn1.buf);
            if(nd){resBuf("Name: %s",nd->name);resBuf("Born: %s",nd->dob);
                   resBuf("Died: %s",nd->dod);resBuf("Info: %s",trunc90(nd->definition));}
        }
        fb=cy+140;
    }
    if(bstAction==BST_ADD){
        drawTitle(cx,cy,"addNameBST — Add to BST");
        drawLabel(cx,cy+32,"Name:"); drawInput(&bstIn1,cx,cy+48,cw,30,"Name...");
        drawLabel(cx,cy+84,"Definition:"); drawInput(&bstIn2,cx,cy+100,cw,30,"Info...");
        drawLabel(cx,cy+136,"DoB:"); drawInput(&bstIn3,cx,cy+152,cw,30,"DoB...");
        drawLabel(cx,cy+188,"DoD:"); drawInput(&bstIn4,cx,cy+204,cw,30,"DoD...");
        if(drawButton(cx,cy+244,130,32,"Add",COL_GREEN)){
            resClear(); gTree=addNameBST(gTree,bstIn1.buf,bstIn2.buf,bstIn3.buf,bstIn4.buf);
            resBuf("Added: %s",bstIn1.buf);
        }
        fb=cy+286;
    }
    if(bstAction==BST_DELETE){
        drawTitle(cx,cy,"deleteNameBST — Delete from BST");
        drawInput(&bstIn1,cx,cy+54,cw,32,"Name...");
        if(drawButton(cx,cy+96,120,32,"Delete",COL_RED)){
            resClear(); gTree=deleteNameBST(gTree,bstIn1.buf);
            resBuf("Deleted: %s",bstIn1.buf);
        }
        fb=cy+140;
    }
    if(bstAction==BST_UPDATE){
        drawTitle(cx,cy,"updateNameBST — Update BST");
        drawLabel(cx,cy+32,"Name:"); drawInput(&bstIn1,cx,cy+48,cw,30,"Name...");
        drawLabel(cx,cy+84,"New def:"); drawInput(&bstIn2,cx,cy+100,cw,30,"");
        drawLabel(cx,cy+136,"New DoB:"); drawInput(&bstIn3,cx,cy+152,cw,30,"");
        drawLabel(cx,cy+188,"New DoD:"); drawInput(&bstIn4,cx,cy+204,cw,30,"");
        if(drawButton(cx,cy+244,130,32,"Update",COL_GOLD)){
            resClear(); gTree=updateNameBST(gTree,bstIn1.buf,bstIn2.buf,bstIn3.buf,bstIn4.buf);
            resBuf("Updated.");
        }
        fb=cy+286;
    }
    if(bstAction==BST_LCA){
        drawTitle(cx,cy,"lowestCommonAncestor");
        drawLabel(cx,cy+36,"Name 1:"); drawInput(&bstIn1,cx,cy+54,cw,32,"Name...");
        drawLabel(cx,cy+96,"Name 2:"); drawInput(&bstIn2,cx,cy+114,cw,32,"Name...");
        if(drawButton(cx,cy+156,120,32,"Find",COL_BTN)){
            resClear(); TTree *lca=lowestCommonAncestor(gTree,bstIn1.buf,bstIn2.buf);
            if(lca) resBuf("LCA: %s",lca->name); else resBuf("Not found.");
        }
        fb=cy+200;
    }
    if(bstAction==BST_RANGE){
        drawTitle(cx,cy,"countNodesRange — Count in Year Range");
        drawLabel(cx,cy+36,"From year:"); drawInput(&bstIn1,cx,cy+54,220,32,"e.g. 1900");
        drawLabel(cx,cy+96,"To year:"); drawInput(&bstIn2,cx,cy+114,220,32,"e.g. 1962");
        if(drawButton(cx,cy+156,140,32,"Count",COL_BTN)){
            resClear(); int c=countNodesRange(gTree,atoi(bstIn1.buf),atoi(bstIn2.buf));
            resBuf("Nodes with DoB in [%s, %s]: %d",bstIn1.buf,bstIn2.buf,c);
        }
        fb=cy+200;
    }
    if(bstAction==BST_SUCC){
        drawTitle(cx,cy,"inOrderSuccessor");
        drawInput(&bstIn1,cx,cy+54,cw,32,"Name...");
        if(drawButton(cx,cy+96,120,32,"Find",COL_BTN)){
            resClear(); TTree *s=inOrderSuccessor(gTree,bstIn1.buf);
            if(s) resBuf("Successor of '%s': %s",bstIn1.buf,s->name);
            else  resBuf("No successor for '%s'.",bstIn1.buf);
        }
        fb=cy+140;
    }

    int rpY=fb+(fb==cy?0:12);
    int rpH=SCREEN_H-rpY-PAD;
    if(rpH>60) drawResultPanel(cx,rpY,cw,rpH);
}

/* ================================================================
   RECURSION
   ================================================================ */
typedef enum{
    RC_NONE=0,RC_COUNT,RC_REMOVE,RC_REPLACE,RC_PERM,
    RC_SUBSEQ,RC_OVERLAP,RC_DISTINCT,RC_PALINDROME
}RCAction;

RCAction rcAction=RC_NONE;
InputBox rcIn1,rcIn2,rcIn3;

void permBuf(char *s,int l,int r){
    char tmp;
    if(l==r){resBuf("  %s",s);return;}
    for(int i=l;i<=r;i++){
        tmp=s[l];s[l]=s[i];s[i]=tmp;
        permBuf(s,l+1,r);
        tmp=s[l];s[l]=s[i];s[i]=tmp;
    }
}
void subseqBuf(char *w,char *b,int wi,int bi){
    b[bi]='\0';
    if(bi>0) resBuf("  %s",b);
    for(int i=wi;w[i];i++){b[bi]=w[i];subseqBuf(w,b,i+1,bi+1);}
}

void drawRecursion(){
    int sx=PAD,sy=56,sw=SIDEBAR_W,sh=27,gap=3;
    struct{const char*lbl;RCAction act;}items[]={
        {"countOccurence",    RC_COUNT},
        {"removeOccurence",   RC_REMOVE},
        {"replaceOccurence",  RC_REPLACE},
        {"namePermutation",   RC_PERM},
        {"subseqName",        RC_SUBSEQ},
        {"longestSubyear",    RC_OVERLAP},
        {"distinctSubseqWord",RC_DISTINCT},
        {"isPalindromeWord",  RC_PALINDROME},
    };
    int n=8,sideH=SCREEN_H-sy-PAD;

    DrawRectangle(sx-4,sy,sw+8,sideH,COL_SIDEBAR);
    for(int i=0;i<n;i++){
        Color col=(rcAction==items[i].act)?COL_BTN_ACT:COL_BTN;
        if(drawButton(sx,sy+i*(sh+gap),sw,sh,items[i].lbl,col)){
            rcAction=items[i].act; resClear();
            initInput(&rcIn1);initInput(&rcIn2);initInput(&rcIn3);
        }
    }

    int cx=sx+sw+PAD*2,cy=60,cw=SCREEN_W-cx-PAD,fb=cy;

    if(rcAction==RC_COUNT){
        drawTitle(cx,cy,"countOccurence — Count in File");
        drawLabel(cx,cy+36,"Name or word:");
        drawInput(&rcIn1,cx,cy+54,cw,32,"e.g. FLN");
        if(drawButton(cx,cy+96,120,32,"Count",COL_BTN)){
            resClear(); rewind(gFile);
            int c=countOccurrence(gFile,rcIn1.buf);
            resBuf("'%s' appears %d time(s) in file.",rcIn1.buf,c);
        }
        fb=cy+140;
    }
    if(rcAction==RC_REMOVE){
        drawTitle(cx,cy,"removeOccurence — Remove Lines");
        drawLabel(cx,cy+36,"WARNING: permanently removes lines from history.txt");
        drawInput(&rcIn1,cx,cy+68,cw,32,"Word to remove...");
        if(drawButton(cx,cy+110,150,32,"Remove",COL_RED)){
            resClear(); removeOccurrence(DATA_FILE,rcIn1.buf);
            reloadData(); resBuf("Removed lines containing '%s'.",rcIn1.buf);
        }
        fb=cy+155;
    }
    if(rcAction==RC_REPLACE){
        drawTitle(cx,cy,"replaceOccurence — Replace Dates");
        drawLabel(cx,cy+32,"Name:"); drawInput(&rcIn1,cx,cy+48,cw,30,"Name...");
        drawLabel(cx,cy+84,"New DoB:"); drawInput(&rcIn2,cx,cy+100,cw,30,"DoB...");
        drawLabel(cx,cy+136,"New DoD:"); drawInput(&rcIn3,cx,cy+152,cw,30,"DoD...");
        if(drawButton(cx,cy+192,130,32,"Replace",COL_GOLD)){
            resClear(); replaceOccurrence(DATA_FILE,rcIn1.buf,rcIn2.buf,rcIn3.buf);
            reloadData(); resBuf("Dates replaced for '%s'.",rcIn1.buf);
        }
        fb=cy+236;
    }
    if(rcAction==RC_PERM){
        drawTitle(cx,cy,"namePermutation — All Permutations");
        drawLabel(cx,cy+36,"Short name (max 7 chars recommended):");
        drawInput(&rcIn1,cx,cy+54,cw,32,"e.g. Ali");
        if(drawButton(cx,cy+96,130,32,"Generate",COL_BTN)){
            resClear();
            char tmp[MAX_LINE_LEN]; strncpy(tmp,rcIn1.buf,MAX_LINE_LEN-1);
            permBuf(tmp,0,(int)strlen(tmp)-1);
            resBuf(" "); resBuf("Total: %d",resultCount-1);
        }
        fb=cy+140;
    }
    if(rcAction==RC_SUBSEQ){
        drawTitle(cx,cy,"subseqName — All Subsequences");
        drawLabel(cx,cy+36,"Short word (max 8 chars):");
        drawInput(&rcIn1,cx,cy+54,cw,32,"e.g. abc");
        if(drawButton(cx,cy+96,130,32,"Generate",COL_BTN)){
            resClear();
            char tmp[MAX_LINE_LEN]; strncpy(tmp,rcIn1.buf,MAX_LINE_LEN-1);
            char buf2[MAX_LINE_LEN]={0};
            subseqBuf(tmp,buf2,0,0);
        }
        fb=cy+140;
    }
    if(rcAction==RC_OVERLAP){
        drawTitle(cx,cy,"longestSubyear — Events in Date Range");
        drawLabel(cx,cy+36,"Start year:"); drawInput(&rcIn1,cx,cy+54,220,32,"e.g. 1954");
        drawLabel(cx,cy+96,"End year:"); drawInput(&rcIn2,cx,cy+114,220,32,"e.g. 1962");
        if(drawButton(cx,cy+156,120,32,"Search",COL_BTN)){
            resClear();
            int y1=atoi(rcIn1.buf),y2=atoi(rcIn2.buf);
            int lo=(y1<y2)?y1:y2,hi=(y1>y2)?y1:y2;
            resBuf("Events between %d and %d:",lo,hi); resBuf(" ");
            TList *e=gEvents; int found=0;
            while(e){int ey=dateToYear(e->dob);if(ey>=lo&&ey<=hi){resBuf("  %s  (%s)",e->name,e->dob);found++;}e=e->next;}
            if(!found) resBuf("No events found in that range.");
        }
        fb=cy+200;
    }
    if(rcAction==RC_DISTINCT){
        drawTitle(cx,cy,"distinctSubseqWord — Distinct Subsequences");
        drawInput(&rcIn1,cx,cy+54,cw,32,"Word...");
        if(drawButton(cx,cy+96,120,32,"Count",COL_BTN)){
            resClear(); int d=distinctSubseqWord(rcIn1.buf);
            resBuf("Distinct subsequences of '%s': %d",rcIn1.buf,d);
        }
        fb=cy+140;
    }
    if(rcAction==RC_PALINDROME){
        drawTitle(cx,cy,"isPalindromeWord — Palindrome Check");
        drawInput(&rcIn1,cx,cy+54,cw,32,"Word...");
        if(drawButton(cx,cy+96,120,32,"Check",COL_BTN)){
            resClear();
            if(isPalindromeWord(rcIn1.buf)) resBuf("'%s' IS a palindrome.",rcIn1.buf);
            else resBuf("'%s' is NOT a palindrome.",rcIn1.buf);
        }
        fb=cy+140;
    }

    int rpY=fb+(fb==cy?0:12);
    int rpH=SCREEN_H-rpY-PAD;
    if(rpH>60) drawResultPanel(cx,rpY,cw,rpH);
}

/* ================================================================
   TOP BAR
   ================================================================ */
void drawTopBar(){
    DrawRectangle(0,0,SCREEN_W,52,COL_SIDEBAR);
    DrawLine(0,52,SCREEN_W,52,COL_BORDER);
    struct{const char*lbl;Screen sc;}tabs[]={
        {"Home",SCR_HOME},{"Linked List",SCR_LIST},
        {"Stack",SCR_STACK},{"BST",SCR_BST},{"Recursion",SCR_RECURSION}};
    int tx=PAD;
    for(int i=0;i<5;i++){
        int tw=TM(tabs[i].lbl,FONT_SIZE)+PAD*2;
        Rectangle r={(float)tx,8,(float)tw,36};
        bool active=(currentScreen==tabs[i].sc);
        DrawRectangleRounded(r,0.25f,4,active?COL_BTN:COL_SIDEBAR);
        if(!active) DrawRectangleRoundedLines(r,0.25f,4,1.5f,COL_BORDER);
        TF(tabs[i].lbl,tx+PAD,18,FONT_SIZE,active?WHITE:COL_DIMTEXT);
        if(CheckCollisionPointRec(GetMousePosition(),r)&&IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
            currentScreen=tabs[i].sc;
        tx+=tw+6;
    }
    char fps[32]; sprintf(fps,"FPS:%d",GetFPS());
    TF(fps,SCREEN_W-60,20,FONT_SIZE-4,COL_DIMTEXT);
}

/* ================================================================
   MAIN
   ================================================================ */
int main(){
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_W,SCREEN_H,"Algeria History Database — NSCS 2025/2026");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    {
        const char*paths[]={"C:\\Windows\\Fonts\\calibri.ttf",
            "C:\\Windows\\Fonts\\segoeui.ttf","C:\\Windows\\Fonts\\arial.ttf",NULL};
        for(int i=0;paths[i];i++){
            if(FileExists(paths[i])){
                gFont=LoadFontEx(paths[i],20,0,512);
                if(gFont.texture.id!=0){
                    SetTextureFilter(gFont.texture,TEXTURE_FILTER_BILINEAR);
                    gFontLoaded=true;
                }
                break;
            }
        }
    }

    reloadData();
    initInput(&llIn1);initInput(&llIn2);initInput(&llIn3);initInput(&llIn4);
    initInput(&stIn1);initInput(&stIn2);initInput(&stIn3);initInput(&stIn4);
    initInput(&bstIn1);initInput(&bstIn2);initInput(&bstIn3);initInput(&bstIn4);
    initInput(&rcIn1);initInput(&rcIn2);initInput(&rcIn3);

    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(COL_BG);
        drawTopBar();
        if     (currentScreen==SCR_HOME)      drawHome();
        else if(currentScreen==SCR_LIST)      drawLinkedList();
        else if(currentScreen==SCR_STACK)     drawStack();
        else if(currentScreen==SCR_BST)       drawBST();
        else if(currentScreen==SCR_RECURSION) drawRecursion();
        EndDrawing();
    }

    freeList(gPersonalities);freeList(gDates);freeList(gEvents);
    freeStack(gStack);freeTree(gTree);
    if(gFile) fclose(gFile);
    CloseWindow();
    return 0;
}