#include <stdio.h>
#include "tree.h"
#include "semantics.h"

extern FILE *yyin;
extern int yylex();
extern int yyparse();
extern void yyrestart(FILE *);
syntaxNode *stroot = NULL;
bool AError=false, BError=false;

#if YYDEBUG
extern int yydebug;
#endif

#if DEBUGFLEX
int main(int argc, char** argv) {
    if (argc > 1) {
        if (!(yyin = fopen(argv[1], "r"))) {
            perror(argv[1]);
            return 1;
        }
    }
    while (yylex() != 0);
    return 0;
}
#else
int main(int argc, char **argv)
{
    if (argc <= 1)
        return 1;
    FILE *f = fopen(argv[1], "r");
    if (!f)
    {
        perror(argv[1]);
        return 1;
    }
    yyrestart(f);

#if YYDEBUG
    yydebug = 1;
#endif

    // Lab 1: call yyparse to get syntax tree.
    yyparse();

    // if(!AError&&!BError)
        // printSyntaxTree();
    
    // Lab 2: conduct a full semantic scan.
    semanticScan();
    ParseProgram(stroot);
    _hash_print_all_symbols();

    return 0;
}
#endif