#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include "../include/types.h"
#include "../include/file_parser.h"
#include "../include/linked_list.h"
#include "../include/stack.h"
#include "../include/tree.h"
#include "../include/recursion.h"

int passed=0, failed=0;
#define TEST(name,expr) do{if(expr){printf("  [PASS] %s\n",name);passed++;}else{printf("  [FAIL] %s\n",name);failed++;}}while(0)

int main(void){
    printf("=== Algeria DB Test Suite ===\n\n-- Parser --\n");
    char name[256],dob[20],dod[20],def[1024];
    TEST("pers line",  isPersonalityLine("Larbi {1923-04/03/1957}= desc"));
    TEST("event line", isEventLine("Battle {20/10/1827}: desc"));
    bool ok=parsePersonalityLine("Larbi {1923-04/03/1957}= algerian figure",name,dob,dod,def);
    TEST("parse ok", ok);
    TEST("name", strcmp(name,"Larbi")==0);
    TEST("dob",  strcmp(dob,"1923")==0);
    TEST("dod",  strcmp(dod,"04/03/1957")==0);

    printf("\n-- Linked List --\n");
    TList *head=NULL;
    appendNode(&head,newNode("Zara","1900","1950","desc z"));
    appendNode(&head,newNode("Ahmed","1920","1980","desc a"));
    appendNode(&head,newNode("Malik","1910","1960","desc m"));
    TEST("length 3", listLength(head)==3);
    TList *s=sortWord(head);
    TEST("alpha sort", strcmp(s->name,"Ahmed")==0);
    TList *fresh=NULL; appendNode(&fresh,newNode("Ahmed","1920","1980","d")); TList *sim=similarPersonality(fresh,"1920");
    TEST("similar year", sim!=NULL);
    freeList(sim); freeList(head);

    printf("\n-- Stack --\n");
    TList *n1=newNode("Zara","1900","1950","d"); TList *n2=newNode("Ahmed","1920","1980","d");
    TStack *stk=NULL; stk=pushStack(stk,n1); stk=pushStack(stk,n2);
    TEST("stack size 2", stackSize(stk)==2);
    TEST("killed yes",  isPersonalityKilled("was killed"));
    TEST("killed no",   !isPersonalityKilled("died peacefully"));
    TStack *rev=recRevStack(stk);
    TEST("reverse top Zara", strcmp(rev->name,"Zara")==0);
    freeList(n1); freeList(n2); freeStack(rev);

    printf("\n-- BST --\n");
    TTree *tr=NULL;
    tr=addNameBST(tr,"Mourad","d","1920","1960");
    tr=addNameBST(tr,"Ahmed","d","1900","1950");
    tr=addNameBST(tr,"Zara","d","1930","1970");
    TEST("size 3", treeSize(tr)==3);
    TEST("balanced", isBalancedBST(tr));
    TEST("range count", countNodesRange(tr,1900,1925)==2);
    tr=deleteNameBST(tr,"Ahmed");
    TEST("after delete size 2", treeSize(tr)==2);
    freeTree(tr);

    printf("\n-- Recursion --\n");
    TEST("palindrome radar", isPalindromeWord("radar"));
    TEST("not palindrome abc", !isPalindromeWord("abc"));

    printf("\n=== %d passed, %d failed ===\n",passed,failed);
    return failed>0?1:0;
}
