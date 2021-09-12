%{
#include "syntax.tab.h"
void logDebugf(char *str){
    char* debugf="\033[36m[Debug INFO]:\033[0m Line %d :input %s : %s\n";
    printf(debugf, yylineno, str, yytext);
}
void logErrorf(char* type, char* str){
    char* errorf="\033[31mError type %s\033[0m at Line %d: \033[31m%s\033[0m \'%s\'\n";
    printf(errorf, type, yylineno, str, yytext);
}
typedef struct node{
    char* syntaxName;
    union {
    unsigned int    ival;
    float           fval;
    char            sval[64];
    };
    struct node *first_child,*next_sibling;
}tree;

#if DEBUGLEVEL
#define TOKENTREE(t) logDebugf("t")
#else
#define TOKENTREE(t) \
 do{ \
    tree *node = (tree *)malloc(sizeof(tree));\
 } while (0)
#endif
int yycolumn = 1;
#define YY_USER_ACTION                                \
  yylloc.first_line   = yylloc.last_line = yylineno;  \
  yylloc.first_column = yycolumn;                     \
  yylloc.last_column  = yycolumn + yyleng - 1;        \
  yycolumn += yyleng;

%}
%option yylineno
digit [0-9]
digits [0-9]+
letter_ [a-zA-Z_]
DEC 0|([1-9][0-9]*)
OCT 0[0-7]+
OCTERROR 0[0-7]*[8-9A-WY-Za-wy-z]+[0-9A-Za-z]*
HEX 0[xX][0-9A-Fa-f]+
HEXERROR 0[xX][0-9A-Fa-f]*[g-zG-Z]+[0-9A-Za-z]*
FLOAT ({digits}\.{digits})|({digit}*\.{digit}*[Ee][+-]?{digits})
FLOATERROR ({digits}\.)|(\.{digits})|({digit}*\.{digit}*[Ee][+-]?)
ID {letter_}({letter_}|{digit}){0,31}
RELOP >|<|>=|<=|==|!=
TYPE int|float
LP \(
RP \)
LB \[
RB \]
LC \{
RC \}
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while
NEWLINE \n
WHITESPACE [ \t\r\n]+
COMMENTL "//"
COMMENTB "/*"
%%
{FLOAT}      {TOKENTREE(FLOAT);return FLOAT;}
{FLOATERROR} {logErrorf("A","Illegal floating point number");return FLOAT;}
{DEC}        {TOKENTREE(INT);return INT;}
{OCT}        {TOKENTREE(INT);return INT;}
{OCTERROR}   {logErrorf("A","Illegal octal number");return INT;}
{HEX}        {TOKENTREE(INT);return INT;}
{HEXERROR}   {logErrorf("A","Illegal hexadecimal number");return INT;}
";"      {TOKENTREE(SEMI);return SEMI;}
","      {TOKENTREE(COMMA);return COMMA;}
"="      {TOKENTREE(ASSIGNOP);return ASSIGNOP;}
{RELOP}  {TOKENTREE(RELOP);return RELOP;}
"+"      {TOKENTREE(PLUS); return PLUS;}
"-"      {TOKENTREE(MINUS);return MINUS; }
"*"      {TOKENTREE(STAR); return STAR;}
"/"      {TOKENTREE(DIV); return DIV;}
"&&"     {TOKENTREE(AND); return AND;}
"||"     {TOKENTREE(OR);return OR;}
"."      {TOKENTREE(DOT);return DOT;}
"!"      {TOKENTREE(NOT);return NOT;}
{TYPE}   {TOKENTREE(TYPE);return TYPE;}
{LP}     {TOKENTREE(LP);return LP;}
{RP}     {TOKENTREE(RP);return RP;}
{LB}     {TOKENTREE(LB);return LB;}
{RB}     {TOKENTREE(RB);return RB;}
{LC}     {TOKENTREE(LC);return LC;}
{RC}     {TOKENTREE(RC);return RC;}
{STRUCT} {TOKENTREE(STRUCT);return STRUCT;}
{RETURN} {TOKENTREE(RETURN);return RETURN;}
{IF}     {TOKENTREE(IF);return IF;}
{ELSE}   {TOKENTREE(ELSE);return ELSE;}
{WHILE}  {TOKENTREE(WHILE);return WHILE;}
{ID}     {TOKENTREE(ID);return ID;}
{NEWLINE} {yycolumn=1;}
{WHITESPACE} {;}
{COMMENTL} {
    char c = input();
    while (c != '\n') c = input();
    logDebugf("COMMENTL");
}
{COMMENTB} {
    logDebugf("COMMENTB");
}
. {logErrorf("A","Mysterious characters");}
%%
