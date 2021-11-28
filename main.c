#include <stdio.h>
#include "tree.h"
#include "semantics.h"
#include "ir.h"
#include "translate.h"
#include "opt.h"

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

    if(!AError&&!BError){
#if ASTREE
        printSyntaxTree();
#endif
    // Lab 2: conduct a full semantic scan.
        ParseProgram(stroot);
        declare_check();
    // Lab 3: generate IR
        translate_Program(stroot);
        LinearOptIC();
        if(argc==3){
            FILE *ir = fopen(argv[2], "w");
            outputInterCodes(ir);
        }else{
            outputInterCodes(stdout);
        }
    }

    return 0;
}
#endif