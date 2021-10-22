#include <stdio.h>
#include "tree.h"
#include "syntax.tab.h"
#include <string.h>
#include <stdlib.h>

syntaxNode *newNode(char *_name, int num, ...)
{
  syntaxNode *n = (syntaxNode *)malloc(sizeof(syntaxNode));
  strcpy(n->name, _name);
  n->token = -1;
  n->first_child = n->next_sibling = NULL;
  if (num == 0)
  {
    n->empty = true;
  }
  else
  {
    va_list list;
    va_start(list, num);
    n->line = va_arg(list, int);
    n->first_child = va_arg(list, syntaxNode *);
    syntaxNode *head = n->first_child;
    for (int i = 0; i < num - 1; i++){
        head->next_sibling = va_arg(list, syntaxNode *);
        head = head->next_sibling;
    }
    va_end(list);
  }

  return n;
}

void printSyntaxTree()
{
  printSyntaxTreeAux(stroot, 0);
}

void printSyntaxTreeAux(syntaxNode *node, int indent)
{
  if (node->empty)
    return;
  for (int i = 0; i < indent; ++i)
    printf("  ");

  printf("%s", node->name);
  if (node->token == -1){
    printf(" (%d)", node->line);
  }
  else
  {
    switch (node->token){
    case INT:
      printf(": %u", node->ival);
      break;
    case FLOAT:
      printf(": %f", node->fval);
      break;
    case ID:
    case TYPE:
      printf(": %s", node->sval);
      break;
    default:
      break;
    }
  }
  printf("\n");

  for (syntaxNode *child = node->first_child; child != NULL; child = child->next_sibling)
  {
    printSyntaxTreeAux(child, indent + 1);
  }
}
