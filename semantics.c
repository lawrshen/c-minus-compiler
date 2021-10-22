#include "semantics.h"
#include "syntax.tab.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define astcmp(node, str) \
    (strcmp(node->name, str) == 0)

extern syntaxNode *stroot;
int region_depth = 0;
const STError SETable[] = {
  {  0, "Pseudo error", "" },
  {  1, "Use of undefined variable ", "" },
  {  2, "Use of undefined function ", "" },
  {  3, "Duplicated name ", " of variables" },
  {  4, "Redefinition of function ", "" },
  {  5, "Type mismatched for assignment", "" },
  {  6, "Cannot assign to rvalue", "" },
  {  7, "Type mismatched for operands", "" },
  {  8, "Type mismatched for return" },
  {  9, "Arguments mismatched for function ", "" },
  { 10, "Array access on non-array variable", "" },
  { 11, "Function call on non-function variable ", "" },
  { 12, "Non integer offset of an array", "" },
  { 13, "Field access on non-struct variable", "" },
  { 14, "Access to undefined field ", " in struct" },
  { 15, "Redefinition or initialization of field ", " in struct" },
  { 16, "Duplicated name ", " of struct" },
  { 17, "Use of undefined struct ", "" },
  { 18, "Function ", " declared but not defined" },
  { 19, "Inconsistent declaration of function ", "" },
};

// Throw an semantic error.
void logSTErrorf(enum SemanticErrors id, int line, const char *name) {
  fprintf(stderr, "Error type %d at Line %d: ", id, line);
  if (name != NULL) {
    fprintf(stderr, "%s\"%s\"%s", SETable[id].message1, name, SETable[id].message2);
  } else {
    fprintf(stderr, "%s", SETable[id].message1);
  }
  fprintf(stderr, ".\n");
}

// main entry of semantic scan
void semanticScan()
{
    checkSemantics(stroot, stroot);
}

void checkSemantics(syntaxNode *node, syntaxNode *parent)
{
    if (node->empty)
        return;
    if (node->token == ID)
    {
        printf("%s %s:%s\n", parent->name, node->name, node->sval);
    }
    for (syntaxNode *child = node->first_child; child != NULL; child = child->next_sibling)
    {
        checkSemantics(child, node);
    }
}

/*** High-Level Definitions ***/
void ParseProgram(syntaxNode *cur)
{
    hash_create();
    //    Program → ExtDefList
    if (astcmp(cur->first_child, "ExtDefList"))
    {
        ParseExtDefList(cur->first_child);
    }
}

void ParseExtDefList(syntaxNode *cur)
{
    //    ExtDefList → ExtDef ExtDefList | \epsilon
    if (cur->first_child == NULL)
    {
        //     PSEUDO
    }
    else if (astcmp(cur->first_child, "ExtDef"))
    {
        ParseExtDef(cur->first_child);
        ParseExtDefList(cur->first_child->next_sibling);
    }
}

void ParseExtDef(syntaxNode *cur)
{
    //    ExtDef → Specifier ExtDecList SEMI
    //    | Specifier SEMI
    //    | Specifier FunDec CompSt
    Type_ptr specifier = ParseSpecifier(cur->first_child);
    syntaxNode *body = cur->first_child->next_sibling;
    if (astcmp(body, "SEMI"))
    {
        return;
    }
    else if (astcmp(body, "ExtDecList"))
    {
        printf("\n ExtDEC! \n");
    }
    else if (astcmp(body, "FunDec"))
    {
        Symbol_ptr sym = ParseFunDec(body, specifier);
        if (astcmp(body->next_sibling, "CompSt"))
        {
            ParseCompSt(body->next_sibling);
        }
    }
}

void ParseExtDecList(syntaxNode *cur, Type_ptr specifier_type)
{
    //    ExtDecList → VarDec | VarDec COMMA ExtDecList
}

Type_ptr ParseSpecifier(syntaxNode *cur)
{
    //    Specifier → TYPE | StructSpecifier
    syntaxNode *body = cur->first_child;
    Type_ptr type = NULL;
    if (astcmp(body, "TYPE"))
    {
        type = (Type_ptr)malloc(sizeof(Type));
        type->kind = BASIC;
        if (strcmp(body->sval, "int") == 0)
        {
            type->u.basic = INT;
        }
        else
        {
            type->u.basic = FLOAT;
        }
    }
    else if (astcmp(body, "StructSpecifier"))
    {
        type = ParseStructSpecifier(body);
    }
    return type;
}

Type_ptr ParseStructSpecifier(syntaxNode *cur)
{
    //    StructSpecifier → STRUCT OptTag LC DefList RC | STRUCT Tag
    return NULL;
}

Symbol_ptr ParseFunDec(syntaxNode *cur, Type_ptr specifier_type)
{
    //    FunDec → ID LP VarList RP | ID LP RP
    Symbol_ptr sym = new_symbol(region_depth);
    sym->name = cur->first_child->sval;
    sym->type = (Type_ptr)malloc(sizeof(Type));
    sym->type->kind = FUNCTION;
    sym->type->u.function.ret = specifier_type;
    syntaxNode *body = cur->first_child->next_sibling->next_sibling;
    if (astcmp(body, "RP"))
    {
        sym->type->u.function.params_num = 0;
    }
    else
    {
        // return val? #todo
        ParseVarList(body, sym);
    }
    // error #todo
    hash_insert(sym);
    return sym;
}

Symbol_ptr ParseVarList(syntaxNode *cur, Symbol_ptr func)
{
    //    VarList → ParamDec COMMA VarList | ParamDec
    return NULL;
}

Symbol_ptr ParseParamDec(syntaxNode *cur)
{
    //    ParamDec → Specifier VarDec
    return NULL;
}

Symbol_ptr ParseVarDec(syntaxNode *cur, Type_ptr specifier_type)
{
    //    VarDec → ID | VarDec LB INT RB
    return NULL;
}

Symbol_ptr ParseDefList(syntaxNode *cur)
{
    //    DecList → Dec | Dec COMMA DecList
    return NULL;
}

Symbol_ptr ParseDef(syntaxNode *cur)
{
    //    Def → Specifier DecList SEMI
    return NULL;
}

Symbol_ptr ParseDecList(syntaxNode *cur, Type_ptr specifier_type)
{
    //    DecList → Dec | Dec COMMA DecList
    return NULL;
}

Symbol_ptr ParseDec(syntaxNode *cur, Type_ptr specifier_type)
{
    //    Dec → VarDec | VarDec ASSIGNOP Exp
    return NULL;
}

void ParseCompSt(syntaxNode *cur)
{
    //    CompSt → LC DefList StmtList RC
    region_depth++;
    syntaxNode *deflist = cur->first_child->next_sibling, *stmtlist = deflist->next_sibling;
    assert(deflist);
    ParseExtDefList(deflist);
    assert(stmtlist);
    ParseStmtList(stmtlist);
    compst_destroy(region_depth);
    region_depth--;
}

void ParseStmtList(syntaxNode *cur)
{
    //    StmtList → Stmt StmtList | \epsilon
    if (cur->first_child == NULL)
    {
        //     PSEUDO
    }
    else if (astcmp(cur->first_child, "Stmt"))
    {
        ParseStmt(cur->first_child);
        ParseStmtList(cur->first_child->next_sibling);
    }
}

void ParseStmt(syntaxNode *cur)
{
    //    Stmt → Exp SEMI | CompSt | RETURN Exp SEMI | IF LP Exp RP Stmt
    //          | IF LP Exp RP Stmt ELSE Stmt | WHILE LP Exp RP Stmt
    syntaxNode *body = cur->first_child;
    if (astcmp(body, "Exp"))
    {
        ParseExp(body);
    }
}

Type_ptr ParseExp(syntaxNode *cur)
{
    //    Exp → Exp ASSIGNOP Exp
    //        | Exp AND Exp
    //        | Exp OR Exp
    //        | Exp RELOP Exp
    //        | Exp PLUS Exp
    //        | Exp MINUS Exp
    //        | Exp STAR Exp
    //        | Exp DIV Exp
    //        | LP Exp RP
    //        | MINUS Exp
    //        | NOT Exp
    //        | ID LP Args RP
    //        | ID LP RP
    //        | Exp LB Exp RB
    //        | Exp DOT ID
    //        | ID
    //        | INT
    //        | FLOAT

    //bug fix
    if (astcmp(cur->first_child, "ID"))
    {
        Symbol_ptr target = hash_find(cur->first_child->sval);
        if (target == NULL)
        {
            logSTErrorf(1,cur->first_child->line,NULL);
            return &UNKNOWN_TYPE;
        }
        return target->type;
    }
    syntaxNode *body = cur->first_child->next_sibling;
    if (body&&astcmp(body, "ASSIGNOP"))
    {
        Type_ptr type_1 = ParseExp(cur->first_child), type_2 = ParseExp(body->next_sibling);
        //error #todo
        return type_1;
    }
    return &UNKNOWN_TYPE;
}
