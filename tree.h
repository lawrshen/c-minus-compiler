#ifndef TREE_H
#define TREE_H

#include<stdarg.h>
#include<stdbool.h>
#include <string.h>

#define astcmp(node, str) \
    (strcmp(node->name, str) == 0)

typedef struct node{
    char name[32];
    int line,token;
    bool empty;
    union {
    unsigned int    ival;
    float           fval;
    char            sval[64];
    };
    struct node *first_child,*next_sibling;
}syntaxNode;

extern syntaxNode *stroot;

syntaxNode * newNode(char* _name,int num,...);
void printSyntaxTree();
void printSyntaxTreeAux(syntaxNode *, int);

#endif

