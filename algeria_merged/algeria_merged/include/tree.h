#ifndef TREE_H
#define TREE_H

#include "types.h"
#include "stack.h"

/* ---- Construction ---- */
TTree* toTree(TStack *stk);
TTree* fillTree(FILE *f);

/* ---- Search ---- */
TTree* getInfoNameTree(TTree *tr, const char *name);

/* ---- CRUD ---- */
TTree* addNameBST(TTree *tr, const char *name,
                  const char *def, const char *DoB, const char *DoD);
TTree* deleteNameBST(TTree *tr, const char *name);
TTree* updateNameBST(TTree *tr, const char *name,
                     const char *s, const char *DoB, const char *DoD);

/* ---- Traversals ---- */
void traversalBSTinOrder(TTree *tr);
void traversalBSTpreOrder(TTree *tr);
void traversalBSTpostOrder(TTree *tr);

/* ---- Statistics ---- */
void heightSizeBST(TTree *tr);
int  treeHeight(TTree *tr);
int  treeSize(TTree *tr);

/* ---- Advanced ---- */
TTree* lowestCommonAncestor(TTree *tr, const char *word1, const char *word2);
int    countNodesRange(TTree *tr, int l, int h);
TTree* inOrderSuccessor(TTree *tr, const char *word);
TTree* BSTMirror(TTree *tr);
bool   isBalancedBST(TTree *tr);
TTree* BTSMerge(TTree *tr1, TTree *tr2);

/* ---- Helpers ---- */
TTree* newTreeNode(const char *name, const char *def,
                   const char *dob, const char *dod);
void   freeTree(TTree *tr);

#endif
