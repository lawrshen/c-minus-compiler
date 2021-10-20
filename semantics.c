#include "semantics.h"
#include "syntax.tab.h"
#include <stdio.h>

extern bool hasErrorS;
extern syntaxNode *stroot;

// main entry of semantic scan
void semanticScan() {
  checkSemantics(stroot, stroot);
}

void checkSemantics(syntaxNode *node, syntaxNode *parent) {
  if (node->empty) return;
  if (node->token == ID) {
    printf("%s %s:%s\n", parent->name, node->name, node->sval);
  }
  for (syntaxNode *child = node->first_child; child != NULL; child = child->next_sibling) {
    checkSemantics(child, node);
  }
}

/*** High-Level Definitions ***/
