%{
#include "tree.h"

typedef struct {
  syntaxNode* st_node;
  union {
    unsigned        ival;
    float           fval;
    char            sval[64];
  };
} node_type;
#define YYSTYPE node_type

#include "lex.yy.c"
extern void logErrorf(char* type, char* str);

void yyerror(char* msg) {
    logErrorf("B",msg);
}

%}

%locations
/* declared tokens */
%token INT 
%token FLOAT
%token ID
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
Program: ExtDefList             {stroot=$$.st_node=newNode("Program",1,@1.first_line,$1.st_node);}
    ;
ExtDefList: ExtDef ExtDefList   {$$.st_node = newNode("ExtDefList",2,@1.first_line,$1.st_node,$2.st_node);}
    | /* empty */               {$$.st_node = newNode("ExtDefList",0);}
    ;
ExtDef: Specifier ExtDecList SEMI    {$$.st_node = newNode("ExtDef",3,@1.first_line,$1.st_node,$2.st_node,$3.st_node);}
    | Specifier SEMI                 {$$.st_node = newNode("ExtDef",2,@1.first_line,$1.st_node,$2.st_node);}
    | Specifier FunDec CompSt        {$$.st_node = newNode("ExtDef",3,@1.first_line,$1.st_node,$2.st_node,$3.st_node);}
    | error SEMI                     
    | error FunDec CompSt            
    ;
ExtDecList: VarDec                  {$$.st_node = newNode("ExtDecList",1,@1.first_line,$1.st_node);}
    | VarDec COMMA ExtDecList       {$$.st_node = newNode("ExtDecList",3,@1.first_line,$1.st_node,$2.st_node,$3.st_node);}
    ;
/* Specifiers */
Specifier: TYPE         {$$.st_node = newNode("Specifier",1,@1.first_line,$1.st_node);}
    | StructSpecifier   {$$.st_node = newNode("Specifier",1,@1.first_line,$1.st_node);}
    ;
StructSpecifier: STRUCT OptTag LC DefList RC   {$$.st_node = newNode("StructSpecifier", 5, @1.first_line, $1.st_node, $2.st_node, $3.st_node, $4.st_node, $5.st_node);}
    | STRUCT OptTag LC error RC
    | STRUCT Tag                               {$$.st_node = newNode("StructSpecifier", 2, @1.first_line, $1.st_node, $2.st_node);}
    ;
OptTag:     {$$.st_node = newNode("OptTag",0);}
    | ID    {$$.st_node = newNode("OptTag", 1, @1.first_line, $1.st_node);}
    ;
Tag: ID     {$$.st_node = newNode("Tag", 1, @1.first_line, $1.st_node);}
    ;

/* Declarators */
VarDec: ID                  {$$.st_node = newNode("VarDec", 1, @1.first_line, $1.st_node);} 
    | VarDec LB INT RB      {$$.st_node = newNode("VarDec", 4, @1.first_line, $1.st_node, $2.st_node, $3.st_node, $4.st_node);}
    ;
FunDec: ID LP VarList RP    {$$.st_node = newNode("FunDec",4,@1.first_line,$1.st_node,$2.st_node,$3.st_node,$4.st_node);}
    | ID LP RP              {$$.st_node = newNode("FunDec",3,@1.first_line,$1.st_node,$2.st_node,$3.st_node);}
    | error RP
    ;
VarList: ParamDec COMMA VarList     {$$.st_node = newNode("VarList", 3, @1.first_line, $1.st_node, $2.st_node, $3.st_node);}
    | ParamDec                      {$$.st_node = newNode("VarList", 1, @1.first_line, $1.st_node);}
    ;
ParamDec: Specifier VarDec          {$$.st_node = newNode("ParamDec", 2, @1.first_line, $1.st_node, $2.st_node);}
    ;

/* Statements */
CompSt: LC DefList StmtList RC      {$$.st_node = newNode("CompSt", 4,@1.first_line, $1.st_node,$2.st_node,$3.st_node,$4.st_node); }
    | error RC
    ;
StmtList: Stmt StmtList  {$$.st_node = newNode("StmtList", 2, @1.first_line, $1.st_node, $2.st_node);}
    | /* empty */        {$$.st_node = newNode("StmtList", 0);}
    ;
Stmt: Exp SEMI                                  {$$.st_node = newNode("Stmt", 2, @1.first_line, $1.st_node, $2.st_node);} 
    | CompSt                                    {$$.st_node = newNode("Stmt", 1, @1.first_line, $1.st_node);}
    | RETURN Exp SEMI                           {$$.st_node = newNode("Stmt", 3, @1.first_line, $1.st_node, $2.st_node, $3.st_node);}
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE   {$$.st_node = newNode("Stmt", 5, @1.first_line, $1.st_node, $2.st_node, $3.st_node, $4.st_node, $5.st_node);}
    | IF LP Exp RP Stmt ELSE Stmt               {$$.st_node = newNode("Stmt", 7, @1.first_line, $1.st_node, $2.st_node, $3.st_node, $4.st_node, $5.st_node, $6.st_node, $7.st_node);}
    | WHILE LP Exp RP Stmt                      {$$.st_node = newNode("Stmt", 5, @1.first_line, $1.st_node, $2.st_node, $3.st_node, $4.st_node, $5.st_node);}
    | error SEMI
    | IF LP error RP Stmt %prec LOWER_THAN_ELSE
    | IF LP error RP Stmt ELSE Stmt
    | IF LP Exp RP error ELSE Stmt
    | WHILE LP error RP Stmt
    | WHILE LP Exp error Stmt
    | error Stmt
    ;

/* Local Definitions */
DefList: Def DefList        {$$.st_node = newNode("DefList", 2, @1.first_line, $1.st_node, $2.st_node);}
    |                       {$$.st_node = newNode("DefList", 0);}
    ;
Def: Specifier DecList SEMI {$$.st_node = newNode("Def", 3, @1.first_line, $1.st_node, $2.st_node, $3.st_node); }
    | Specifier error SEMI
    ;
DecList: Dec                {$$.st_node = newNode("DecList", 1, @1.first_line, $1.st_node);}
    | Dec COMMA DecList     {$$.st_node = newNode("DecList", 3, @1.first_line, $1.st_node, $2.st_node, $3.st_node); }
    ;
Dec: VarDec                 {$$.st_node = newNode("Dec", 1, @1.first_line, $1.st_node);}
    | VarDec ASSIGNOP Exp   {$$.st_node = newNode("Dec", 3, @1.first_line, $1.st_node, $2.st_node, $3.st_node); }
    | error ASSIGNOP Exp
    ;

/* Expressions */
Exp: Exp ASSIGNOP Exp       {$$.st_node = newNode("Exp", 3, @1.first_line, $1.st_node, $2.st_node, $3.st_node);}
    | Exp AND Exp           {$$.st_node = newNode("Exp", 3, @1.first_line, $1.st_node, $2.st_node, $3.st_node);}
    | Exp OR Exp            {$$.st_node = newNode("Exp", 3, @1.first_line, $1.st_node, $2.st_node, $3.st_node);}
    | Exp RELOP Exp         {$$.st_node = newNode("Exp", 3, @1.first_line, $1.st_node, $2.st_node, $3.st_node);}
    | Exp PLUS Exp          {$$.st_node = newNode("Exp", 3, @1.first_line, $1.st_node, $2.st_node, $3.st_node);}
    | Exp MINUS Exp         {$$.st_node = newNode("Exp", 3, @1.first_line, $1.st_node, $2.st_node, $3.st_node);}
    | Exp STAR Exp          {$$.st_node = newNode("Exp", 3, @1.first_line, $1.st_node, $2.st_node, $3.st_node);}
    | Exp DIV Exp           {$$.st_node = newNode("Exp", 3, @1.first_line, $1.st_node, $2.st_node, $3.st_node);}
    | LP Exp RP             {$$.st_node = newNode("Exp", 3, @1.first_line, $1.st_node, $2.st_node, $3.st_node);}
    | LP error RP
    | LP error SEMI
    | LP error RC
    | MINUS Exp %prec NEG   {$$.st_node = newNode("Exp", 2, @1.first_line, $1.st_node, $2.st_node);}
    | NOT Exp               {$$.st_node = newNode("Exp", 2, @1.first_line, $1.st_node, $2.st_node);}
    | ID LP Args RP         {$$.st_node = newNode("Exp", 4, @1.first_line, $1.st_node, $2.st_node, $3.st_node, $4.st_node);}
    | ID LP RP              {$$.st_node = newNode("Exp", 3, @1.first_line, $1.st_node, $2.st_node, $3.st_node);}
    | ID LP error RP
    | ID LP error SEMI
    | ID LP error RC
    | Exp LB Exp RB         {$$.st_node = newNode("Exp", 4, @1.first_line, $1.st_node, $2.st_node, $3.st_node, $4.st_node);}
    | Exp LB error RB
    | Exp LB error SEMI
    | Exp LB error RC
    | Exp DOT ID            {$$.st_node = newNode("Exp", 3, @1.first_line, $1.st_node, $2.st_node, $3.st_node);}
    | ID                    {$$.st_node = newNode("Exp", 1, @1.first_line, $1.st_node);}
    | INT                   {$$.st_node = newNode("Exp", 1, @1.first_line, $1.st_node);}
    | FLOAT                 {$$.st_node = newNode("Exp", 1, @1.first_line, $1.st_node);}
    ;
Args:Exp COMMA Args     {$$.st_node = newNode("Args", 3, @1.first_line, $1.st_node, $2.st_node, $3.st_node);}
    | Exp               {$$.st_node = newNode("Args", 1, @1.first_line, $1.st_node);}
    ;
%% 
