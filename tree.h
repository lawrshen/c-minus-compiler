#ifndef TREE_H
#define TREE_H

#include<stdarg.h>

typedef struct node{
    char name[32];
    int line;
    union {
    unsigned int    ival;
    float           fval;
    char            sval[64];
    };
    struct node *first_child,*next_sibling;
}syntaxNode;

#endif

