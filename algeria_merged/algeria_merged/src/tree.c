#include "tree.h"
#include "linked_list.h"
#include "file_parser.h"

TTree* newTreeNode(const char *name, const char *def,
                   const char *dob, const char *dod) {
    TTree *n = (TTree*)calloc(1, sizeof(TTree));
    strncpy(n->name,       name?name:"", MAX_NAME-1);
    strncpy(n->definition, def ?def :"", MAX_DEF-1);
    strncpy(n->dob,        dob ?dob :"", MAX_DATE-1);
    strncpy(n->dod,        dod ?dod :"", MAX_DATE-1);
    return n;
}

TTree* addNameBST(TTree *tr, const char *name, const char *def,
                  const char *DoB, const char *DoD) {
    if (!tr) return newTreeNode(name, def, DoB, DoD);
    int cmp = strcasecmp(name, tr->name);
    if (cmp < 0) tr->left  = addNameBST(tr->left,  name, def, DoB, DoD);
    else if(cmp > 0) tr->right = addNameBST(tr->right, name, def, DoB, DoD);
    else { /* update */
        if(def&&def[0])  strncpy(tr->definition,def,MAX_DEF-1);
        if(DoB&&DoB[0])  strncpy(tr->dob,DoB,MAX_DATE-1);
        if(DoD&&DoD[0])  strncpy(tr->dod,DoD,MAX_DATE-1);
    }
    return tr;
}

TTree* toTree(TStack *stk) {
    TTree *tr = NULL;
    while(stk){
        tr = addNameBST(tr, stk->name, stk->definition, stk->dob, stk->dod);
        stk=stk->next;
    }
    return tr;
}

TTree* fillTree(FILE *f) {
    TList *pers = getPersonality(f);
    TTree *tr = NULL;
    TList *cur = pers;
    while(cur){
        tr = addNameBST(tr, cur->name, cur->definition, cur->dob, cur->dod);
        cur=cur->next;
    }
    freeList(pers);
    return tr;
}

TTree* getInfoNameTree(TTree *tr, const char *name) {
    if (!tr) { printf("  '%s' not found.\n",name); return NULL; }
    int cmp = strcasecmp(name, tr->name);
    if (cmp == 0) {
        printf("  Name: %s\n  DoB : %s\n  DoD : %s\n  Def : %s\n",
               tr->name,tr->dob,tr->dod,tr->definition);
        return tr;
    }
    if (cmp < 0) return getInfoNameTree(tr->left,  name);
    return              getInfoNameTree(tr->right, name);
}

TTree* findMin(TTree *tr) {
    while(tr->left) tr=tr->left; return tr;
}

TTree* deleteNameBST(TTree *tr, const char *name) {
    if (!tr) return NULL;
    int cmp = strcasecmp(name, tr->name);
    if (cmp < 0) tr->left  = deleteNameBST(tr->left,  name);
    else if(cmp > 0) tr->right = deleteNameBST(tr->right, name);
    else {
        if (!tr->left) { TTree *r=tr->right; free(tr); return r; }
        if (!tr->right){ TTree *l=tr->left;  free(tr); return l; }
        TTree *succ = findMin(tr->right);
        strncpy(tr->name,       succ->name,       MAX_NAME-1);
        strncpy(tr->definition, succ->definition, MAX_DEF-1);
        strncpy(tr->dob,        succ->dob,        MAX_DATE-1);
        strncpy(tr->dod,        succ->dod,        MAX_DATE-1);
        tr->right = deleteNameBST(tr->right, succ->name);
    }
    return tr;
}

TTree* updateNameBST(TTree *tr, const char *name, const char *s,
                     const char *DoB, const char *DoD) {
    TTree *node = getInfoNameTree(tr, name);
    if (node) {
        if(s&&s[0])   strncpy(node->definition,s,MAX_DEF-1);
        if(DoB&&DoB[0]) strncpy(node->dob,DoB,MAX_DATE-1);
        if(DoD&&DoD[0]) strncpy(node->dod,DoD,MAX_DATE-1);
        printf("  Updated '%s' in BST.\n",name);
    }
    return tr;
}

void traversalBSTinOrder(TTree *tr) {
    if (!tr) return;
    traversalBSTinOrder(tr->left);
    printf("  %s (DoB:%s DoD:%s)\n", tr->name, tr->dob, tr->dod);
    traversalBSTinOrder(tr->right);
}
void traversalBSTpreOrder(TTree *tr) {
    if (!tr) return;
    printf("  %s (DoB:%s DoD:%s)\n", tr->name, tr->dob, tr->dod);
    traversalBSTpreOrder(tr->left);
    traversalBSTpreOrder(tr->right);
}
void traversalBSTpostOrder(TTree *tr) {
    if (!tr) return;
    traversalBSTpostOrder(tr->left);
    traversalBSTpostOrder(tr->right);
    printf("  %s (DoB:%s DoD:%s)\n", tr->name, tr->dob, tr->dod);
}

int treeHeight(TTree *tr) {
    if (!tr) return 0;
    int l=treeHeight(tr->left), r=treeHeight(tr->right);
    return 1 + (l>r?l:r);
}
int treeSize(TTree *tr) {
    if (!tr) return 0;
    return 1 + treeSize(tr->left) + treeSize(tr->right);
}
void heightSizeBST(TTree *tr) {
    printf("  Height: %d | Size: %d\n", treeHeight(tr), treeSize(tr));
}

TTree* lowestCommonAncestor(TTree *tr, const char *w1, const char *w2) {
    if (!tr) return NULL;
    if (strcasecmp(w1,tr->name)<0 && strcasecmp(w2,tr->name)<0)
        return lowestCommonAncestor(tr->left,  w1, w2);
    if (strcasecmp(w1,tr->name)>0 && strcasecmp(w2,tr->name)>0)
        return lowestCommonAncestor(tr->right, w1, w2);
    printf("  LCA of '%s' and '%s' is: %s\n", w1, w2, tr->name);
    return tr;
}

int countNodesRange(TTree *tr, int l, int h) {
    if (!tr) return 0;
    int yr = dateToYear(tr->dob);
    int cnt = (yr>=l && yr<=h) ? 1 : 0;
    return cnt + countNodesRange(tr->left,l,h) + countNodesRange(tr->right,l,h);
}

TTree* inOrderSuccessor(TTree *tr, const char *word) {
    TTree *succ=NULL, *cur=tr;
    while(cur){
        int cmp=strcasecmp(word,cur->name);
        if(cmp<0){succ=cur;cur=cur->left;}
        else cur=cur->right;
    }
    if(succ) printf("  In-order successor of '%s': %s\n",word,succ->name);
    else printf("  No in-order successor for '%s'.\n",word);
    return succ;
}

TTree* BSTMirror(TTree *tr) {
    if (!tr) return NULL;
    TTree *tmp = tr->left;
    tr->left  = BSTMirror(tr->right);
    tr->right = BSTMirror(tmp);
    return tr;
}

int absVal(int x){ return x<0?-x:x; }
bool isBalancedBST(TTree *tr) {
    if (!tr) return true;
    int lh=treeHeight(tr->left), rh=treeHeight(tr->right);
    return absVal(lh-rh)<=1 && isBalancedBST(tr->left) && isBalancedBST(tr->right);
}

/* Flatten tree to sorted array, then build balanced BST */
void inOrderCollect(TTree *tr, TTree **arr, int *idx) {
    if(!tr) return;
    inOrderCollect(tr->left,arr,idx);
    arr[(*idx)++]=tr;
    inOrderCollect(tr->right,arr,idx);
}
TTree* buildBalanced(TTree **arr, int start, int end) {
    if(start>end) return NULL;
    int mid=(start+end)/2;
    TTree *root=arr[mid];
    root->left  = buildBalanced(arr,start,mid-1);
    root->right = buildBalanced(arr,mid+1,end);
    return root;
}
TTree* BTSMerge(TTree *tr1, TTree *tr2) {
    int n1=treeSize(tr1), n2=treeSize(tr2), n=n1+n2;
    if(n==0) return NULL;
    TTree **arr=(TTree**)malloc(n*sizeof(TTree*));
    int idx=0;
    inOrderCollect(tr1,arr,&idx);
    inOrderCollect(tr2,arr,&idx);
    /* sort merged array by name */
    for(int x=0;x<n-1;x++) for(int y=x+1;y<n;y++)
        if(strcasecmp(arr[x]->name,arr[y]->name)>0){TTree *tmp=arr[x];arr[x]=arr[y];arr[y]=tmp;}
    TTree *result=buildBalanced(arr,0,n-1);
    free(arr);
    return result;
}

void freeTree(TTree *tr) {
    if(!tr) return;
    freeTree(tr->left); freeTree(tr->right); free(tr);
}
