%{
#include "lex.yy.c"
extern void logErrorf(char* type, char* str);

void yyerror(char* msg) {
    logErrorf("B",msg);
}

%}
%union {
unsigned type_unsigned;
float type_float;
char type_string[64];
}
%locations
/* declared tokens */
%token <type_unsigned>INT 
%token <type_float>FLOAT
%token <type_string>ID
%token SEMI COMMA
%token RELOP ASSIGNOP
%token PLUS MINUS STAR DIV AND OR NOT
%token DOT
%token LP RP LB RB LC RC
%token TYPE STRUCT RETURN IF ELSE WHILE

%start Program
%right ASSIGNOP
%left OR
%left AND 
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT NEG
%left DOT LB RB LP RP
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%
/* High-level Definitions */
Program: ExtDefList{

    }
    ;
ExtDefList: ExtDef ExtDefList
    | /* empty */
    ;
ExtDef: Specifier ExtDecList SEMI
    | Specifier SEMI
    | Specifier FunDec CompSt
    | error SEMI
    | error FunDec CompSt
    ;
ExtDecList: VarDec
    | VarDec COMMA ExtDecList
    ;
/* Specifiers */
Specifier: TYPE {

        }
    | StructSpecifier {

        }
    ;
StructSpecifier: STRUCT OptTag LC DefList RC {

        }
    | STRUCT Tag {

        }
    ;
OptTag: ID {

        }
    | /* empty */ {

        }
    ;
Tag: ID {

        }
    ;
/* Declarators */
VarDec: ID {

        }
    | VarDec LB INT RB {

        }
    ;
FunDec: ID LP VarList RP {

        }
    | ID LP RP {

        }
    ;
VarList: ParamDec COMMA VarList {

        }
    | ParamDec {

        }
    ;
ParamDec: Specifier VarDec {

        }
    ;

/* Statements */
CompSt: LC DefList StmtList RC {

        }
    ;
StmtList: Stmt StmtList {

        }
    | /* empty */ {

        }
    ;
Stmt: Exp SEMI {

        }
    | CompSt {

        }
    | RETURN Exp SEMI {

        }
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {

        }
    | IF LP Exp RP Stmt ELSE Stmt {

        }
    | WHILE LP Exp RP Stmt {

        }
    | error SEMI
    ;

/* Local Definitions */
DefList: Def DefList {

        }
    | /* empty */ {

        }
    ;
Def: Specifier DecList SEMI {

        }
    ;
DecList: Dec {

        }
    | Dec COMMA DecList {

        }
    ;
Dec: VarDec {

        }
    | VarDec ASSIGNOP Exp {

        }
    ;

/* Expressions */
Exp: Exp ASSIGNOP Exp {

        }
    | Exp AND Exp {

        }
    | Exp OR Exp {

        }
    | Exp RELOP Exp {

        }
    | Exp PLUS Exp {

        }
    | Exp MINUS Exp {

        }
    | Exp STAR Exp {

        }
    | Exp DIV Exp {

        }
    | LP Exp RP {

        }
    | MINUS Exp %prec NEG{

        }
    | NOT Exp {

        }
    | ID LP Args RP {

        }
    | ID LP RP {

        }
    | Exp LB Exp RB {

        }
    | Exp DOT ID {

        }
    | ID {

        }
    | INT {

        }
    | FLOAT {

        }
    ;
Args: Exp COMMA Args {

        }
    | Exp {

        }
    ;
%% 
#include<stdarg.h>

void insert(syntaxNode* parent,int num,...){
    va_list ap;
    va_start(ap,num);

}

syntaxNode *insertsyntaxNode(char *syntaxName, syntaxNode *parent, syntaxNode *pre_sibling) {
    syntaxNode *child = (syntaxNode *)malloc(sizeof(syntaxNode));
    child->syntaxName = syntaxName;

    if (parent != NULL) parent->first_child = child;
    if (pre_sibling != NULL) pre_sibling->next_sibling = child;

    child->first_child = NULL;
    child->next_sibling = NULL;

    return child;
}