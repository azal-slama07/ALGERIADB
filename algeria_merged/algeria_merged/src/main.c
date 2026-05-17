/*  Algeria History Database - Main Menu
    Course: Algorithms and Dynamic Data Structures
    Institution: NSCS (National School of Cybersecurity)
    Academic Year: 2025/2026
*/
#include "types.h"
#include "file_parser.h"
#include "linked_list.h"
#include "stack.h"
#include "tree.h"
#include "recursion.h"

#define DATA_FILE "data/history.txt"

//Global state
FILE *gFile = NULL;
TList *gPersonalities = NULL;
TList *gDates = NULL;
TList *gEvents = NULL;
TStack *gStack = NULL;
TTree *gTree = NULL;

//Utility 
void clearInputBuffer() {
    int c;
    while ((c=getchar()) != '\n' && c != EOF);
}
void readString(const char *prompt, char *buf, int maxLen) {
    int l;
    printf("%s", prompt);
    if (!fgets(buf, maxLen, stdin)) { buf[0]='\0'; return; }
    l=(int)strlen(buf);
    if(l>0 && buf[l-1]=='\n') buf[l-1]='\0';
}

void reloadData() {
    if (gFile) fclose(gFile);
    gFile = fopen(DATA_FILE, "r+");
    if (!gFile) { gFile = fopen(DATA_FILE, "w+"); }
    if (!gFile) { perror("Cannot open data file"); exit(1); }
    freeList(gPersonalities); freeList(gDates); freeList(gEvents);
    freeStack(gStack); freeTree(gTree);
    gPersonalities = getPersonality(gFile);
    rewind(gFile);
    gDates = getDatePersonality(gFile);
    rewind(gFile);
    gEvents = getEvents(gFile);
    TList *merged  = mergeNodes(gPersonalities, gDates);
    gStack = toStack(merged);
    gTree = toTree(gStack);
    freeList(merged);
}

//MENU SECTIONS

void menuLinkedList() {
    char buf[MAX_NAME], buf2[MAX_DEF], buf3[MAX_DATE], buf4[MAX_DATE];
    int choice;
    printf("\n===== LINKED LIST & QUEUE MENU =====\n");
    printf("  1.  Show all personalities\n");
    printf("  2.  Show all events\n");
    printf("  3.  Search by date of birth\n");
    printf("  4.  Search by date of death\n");
    printf("  5.  Sort alphabetically\n");
    printf("  6.  Sort by name length\n");
    printf("  7.  Sort by age\n");
    printf("  8.  Add personality\n");
    printf("  9.  Add event\n");
    printf("  10. Delete personality\n");
    printf("  11. Update personality\n");
    printf("  12. Find similar by year\n");
    printf("  13. Count personalities matching a date\n");
    printf("  14. Show palindrome names\n");
    printf("  15. Merge into bidirectional list\n");
    printf("  16. Queue sorted by name word count\n");
    printf("  17. Queue sorted by age\n");
    printf("  0.  Back\n");
    printf("Choice: "); scanf("%d",&choice); clearInputBuffer();

    switch(choice) {
        case 1: printList(gPersonalities); break;
        case 2: printList(gEvents); break;
        case 3:
            readString("Enter DoB (DD/MM/YYYY or YYYY): ", buf, MAX_DATE);
            getInfoByDates(gPersonalities, buf); break;
        case 4:
            readString("Enter DoD (DD/MM/YYYY or YYYY): ", buf, MAX_DATE);
            getInfoByDates2(gPersonalities, buf); break;
        case 5: { TList *s=sortWord(gPersonalities); printList(s); break; }
        case 6: { TList *s=sortWord2(gPersonalities); printList(s); break; }
        case 7: { TList *s=sortPersonality(gPersonalities); printList(s); break; }
        case 8:
            readString("Name: ", buf, MAX_NAME);
            readString("DoB : ", buf3, MAX_DATE);
            readString("DoD : ", buf4, MAX_DATE);
            readString("Def : ", buf2, MAX_DEF);
            gPersonalities = addPersonality(gPersonalities, gDates, buf, buf3, buf4, buf2, DATA_FILE);
            break;
        case 9:
            readString("Event name : ", buf, MAX_NAME);
            readString("Date       : ", buf3, MAX_DATE);
            readString("Description: ", buf2, MAX_DEF);
            gEvents = addEvents(gEvents, buf, buf3, buf2, DATA_FILE);
            break;
        case 10:
            readString("Name to delete: ", buf, MAX_NAME);
            gPersonalities = deletePersonality(DATA_FILE, gPersonalities, gDates, buf);
            break;
        case 11:
            readString("Name   : ", buf, MAX_NAME);
            readString("New Def (Enter to skip): ", buf2, MAX_DEF);
            readString("New DoB (Enter to skip): ", buf3, MAX_DATE);
            readString("New DoD (Enter to skip): ", buf4, MAX_DATE);
            gPersonalities = updatePersonality(DATA_FILE, gPersonalities, gDates, buf, buf2, buf3, buf4);
            break;
        case 12:
            readString("Year: ", buf, MAX_DATE);
            { TList *s=similarPersonality(gPersonalities, buf); printList(s); freeList(s); }
            break;
        case 13: {
            Date d={0,0,0};
            readString("Year (e.g. 1957): ", buf, MAX_DATE); d.year=atoi(buf);
            TList *s=countPersonality(gPersonalities,&d); printList(s); freeList(s); break;
        }
        case 14: { TList *s=palindromeName(gPersonalities); printList(s); freeList(s); break; }
        case 15: { TList *m=mergeNodes(gPersonalities,gDates); printList(m); freeList(m); break; }
        case 16: { TQueue *q=sName(gPersonalities); printQueue(q); freeQueue(q); break; }
        case 17: { TQueue *q=ageP(gPersonalities); printQueue(q); freeQueue(q); break; }
        case 0: return;
        default: printf("  Invalid choice.\n");
    }
}

void menuStack() {
    char buf[MAX_NAME], buf2[MAX_DEF], buf3[MAX_DATE], buf4[MAX_DATE];
    int choice;
    printf("\n===== STACK MENU =====\n");
    printf("  1.  Show stack\n");
    printf("  2.  Search personality in stack\n");
    printf("  3.  Sort stack alphabetically\n");
    printf("  4.  Delete from stack\n");
    printf("  5.  Update in stack\n");
    printf("  6.  Add to stack\n");
    printf("  7.  Convert stack -> sorted queue\n");
    printf("  8.  Convert stack -> bidirectional list\n");
    printf("  9.  Sort by definition word count\n");
    printf("  10. Split by description length\n");
    printf("  11. Get smallest definition\n");
    printf("  12. Find overlapping events\n");
    printf("  13. Check if personality was killed\n");
    printf("  14. Reverse stack (recursive)\n");
    printf("  0.  Back\n");
    printf("Choice: "); scanf("%d",&choice); clearInputBuffer();

    switch(choice) {
        case 1: printStack(gStack); break;
        case 2:
            readString("Name: ", buf, MAX_NAME);
            getInfoPersonality(gStack, buf); break;
        case 3: gStack=sortNameStack(gStack); printStack(gStack); break;
        case 4:
            readString("Name to delete: ", buf, MAX_NAME);
            gStack=deleteName(gStack,buf); break;
        case 5:
            readString("Name  : ", buf, MAX_NAME);
            readString("Def   : ", buf2, MAX_DEF);
            readString("DoB   : ", buf3, MAX_DATE);
            readString("DoD   : ", buf4, MAX_DATE);
            gStack=updateStack(gStack,buf,buf2,buf3,buf4); break;
        case 6:
            readString("Name: ", buf, MAX_NAME);
            readString("Def : ", buf2, MAX_DEF);
            readString("DoB : ", buf3, MAX_DATE);
            readString("DoD : ", buf4, MAX_DATE);
            gStack=addNameStack(gStack,buf,buf2,buf3,buf4); break;
        case 7: { TQueue *q=stackToQueue(gStack); printQueue(q); freeQueue(q); break; }
        case 8: { TList *l=stackToList(gStack); printList(l); freeList(l); break; }
        case 9: gStack=definitionStack(gStack); printStack(gStack); break;
        case 10: { TStack *s2=pronunciationStack(gStack); printStack(s2); break; }
        case 11: getSmallest(gStack); break;
        case 12: continuousSearch(gStack); break;
        case 13:
            readString("Name/definition to check: ", buf2, MAX_DEF);
            printf("  Was killed: %s\n", isPersonalityKilled(buf2)?"YES":"NO"); break;
        case 14: gStack=recRevStack(gStack); printStack(gStack); break;
        case 0: return;
        default: printf("  Invalid choice.\n");
    }
}

void menuBST() {
    char buf[MAX_NAME], buf2[MAX_DEF], buf3[MAX_DATE], buf4[MAX_DATE];
    int choice;
    printf("\n===== BST MENU =====\n");
    printf("  1.  Search by name\n");
    printf("  2.  Add personality\n");
    printf("  3.  Delete personality\n");
    printf("  4.  Update personality\n");
    printf("  5.  In-order traversal\n");
    printf("  6.  Pre-order traversal\n");
    printf("  7.  Post-order traversal\n");
    printf("  8.  Height & size\n");
    printf("  9.  Lowest common ancestor\n");
    printf("  10. Count nodes in year range\n");
    printf("  11. In-order successor\n");
    printf("  12. Mirror tree\n");
    printf("  13. Is tree balanced?\n");
    printf("  0.  Back\n");
    printf("Choice: "); scanf("%d",&choice); clearInputBuffer();

    switch(choice) {
        case 1: readString("Name: ",buf,MAX_NAME); getInfoNameTree(gTree,buf); break;
        case 2:
            readString("Name: ",buf,MAX_NAME); readString("Def: ",buf2,MAX_DEF);
            readString("DoB: ",buf3,MAX_DATE); readString("DoD: ",buf4,MAX_DATE);
            gTree=addNameBST(gTree,buf,buf2,buf3,buf4); break;
        case 3: readString("Name: ",buf,MAX_NAME); gTree=deleteNameBST(gTree,buf); break;
        case 4:
            readString("Name: ",buf,MAX_NAME); readString("New def: ",buf2,MAX_DEF);
            readString("New DoB: ",buf3,MAX_DATE); readString("New DoD: ",buf4,MAX_DATE);
            gTree=updateNameBST(gTree,buf,buf2,buf3,buf4); break;
        case 5: traversalBSTinOrder(gTree); break;
        case 6: traversalBSTpreOrder(gTree); break;
        case 7: traversalBSTpostOrder(gTree); break;
        case 8: heightSizeBST(gTree); break;
        case 9:
            readString("Name 1: ",buf,MAX_NAME); readString("Name 2: ",buf2,MAX_NAME);
            lowestCommonAncestor(gTree,buf,buf2); break;
        case 10: {
            int l,h; printf("  From year: "); scanf("%d",&l); clearInputBuffer();
            printf("  To year  : "); scanf("%d",&h); clearInputBuffer();
            printf("  Nodes in [%d,%d]: %d\n",l,h,countNodesRange(gTree,l,h)); break;
        }
        case 11: readString("Name: ",buf,MAX_NAME); inOrderSuccessor(gTree,buf); break;
        case 12: gTree=BSTMirror(gTree); printf("  Tree mirrored.\n"); break;
        case 13: printf("  Balanced: %s\n", isBalancedBST(gTree)?"YES":"NO"); break;
        case 0: return;
        default: printf("  Invalid choice.\n");
    }
}

void menuRecursion() {
    char buf[MAX_NAME], buf2[MAX_DATE], buf3[MAX_DATE];
    int choice;
    printf("\n===== RECURSION MENU =====\n");
    printf("  1.  Count occurrences of a name\n");
    printf("  2.  Remove all lines with a word\n");
    printf("  3.  Replace dates for a name\n");
    printf("  4.  Print all permutations of a name\n");
    printf("  5.  Print all subsequences of a word\n");
    printf("  6.  Events overlapping date range\n");
    printf("  7.  Count distinct subsequences\n");
    printf("  8.  Check if word is palindrome\n");
    printf("  0.  Back\n");
    printf("Choice: "); scanf("%d",&choice); clearInputBuffer();

    switch(choice) {
        case 1: readString("Name: ",buf,MAX_NAME); countOccurrence(gFile,buf); break;
        case 2: readString("Word: ",buf,MAX_NAME); removeOccurrence(DATA_FILE,buf); reloadData(); break;
        case 3:
            readString("Name: ",buf,MAX_NAME);
            readString("New DoB: ",buf2,MAX_DATE);
            readString("New DoD: ",buf3,MAX_DATE);
            replaceOccurrence(DATA_FILE,buf,buf2,buf3); reloadData(); break;
        case 4: readString("Name: ",buf,MAX_NAME); namePermutation(buf); break;
        case 5: readString("Word: ",buf,MAX_NAME); subseqName(buf); break;
        case 6:
            readString("Start year: ",buf2,MAX_DATE);
            readString("End year  : ",buf3,MAX_DATE);
            longestSubyear(gEvents,buf2,buf3); break;
        case 7: readString("Event: ",buf,MAX_NAME); distinctSubseqWord(buf); break;
        case 8:
            readString("Word: ",buf,MAX_NAME);
            printf("  '%s' is%s a palindrome.\n",buf,isPalindromeWord(buf)?"":" NOT"); break;
        case 0: return;
        default: printf("  Invalid choice.\n");
    }
}

/* ===================== MAIN ===================== */
int main() {
    printf("========================================\n");
    printf("  Algeria History Database\n");
    printf("  NSCS - Algorithms & Dynamic Data Structures\n");
    printf("========================================\n");

    reloadData();
    printf("  Loaded %d personalities and %d events.\n\n",
           listLength(gPersonalities), listLength(gEvents));

    int choice;
    do {
        printf("\n============= MAIN MENU =============\n");
        printf("  1. Linked Lists & Queues\n");
        printf("  2. Stacks\n");
        printf("  3. Binary Search Tree (BST)\n");
        printf("  4. Recursive Functions\n");
        printf("  5. Reload data from file\n");
        printf("  0. Exit\n");
        printf("Choice: ");
        if (scanf("%d",&choice) != 1) break;
        clearInputBuffer();

        switch(choice) {
            case 1: menuLinkedList(); break;
            case 2: menuStack();      break;
            case 3: menuBST();        break;
            case 4: menuRecursion();  break;
            case 5: reloadData(); printf("  Data reloaded.\n"); break;
            case 0: printf("  Goodbye!\n"); break;
            default: printf("  Invalid choice.\n");
        }
    } while (choice != 0);

    freeList(gPersonalities); freeList(gDates); freeList(gEvents);
    freeStack(gStack); freeTree(gTree);
    if (gFile) fclose(gFile);
    return 0;
}
