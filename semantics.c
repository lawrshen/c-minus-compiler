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
Symbol_ptr region_func=NULL;
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

#if COLORFUL
const char* STerrorf="\033[31mError type %d\033[0m at Line %d: ";
#else
const char* STerrorf="Error type %d at Line %d: ";
#endif

// Log an semantic error.
void logSTErrorf(enum SemanticErrors id, int line, const char *name) {
  fprintf(stderr, STerrorf, id, line);
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
    if (astcmp(body, "SEMI")){
        return;
    }
    else if (astcmp(body, "ExtDecList")){
        ParseExtDecList(body,specifier);
    }
    else if (astcmp(body, "FunDec"))
    {
        Symbol_ptr sym = ParseFunDec(body, specifier);
        // error 4
        if(hash_insert(sym)==false){
            logSTErrorf(4,cur->first_child->line,sym->name);
        }
        region_func = sym;
        ParseCompSt(body->next_sibling);
        region_func = NULL;
    }
}

void ParseExtDecList(syntaxNode *cur, Type_ptr specifier_type){
    //    ExtDecList → VarDec | VarDec COMMA ExtDecList
    Symbol_ptr sym = ParseVarDec(cur->first_child,specifier_type);
    if(hash_insert(sym)==false){
        logSTErrorf(3,cur->first_child->line,sym->name);
    }
    syntaxNode *comma=cur->first_child->next_sibling;
    if(comma){
        ParseExtDecList(comma->next_sibling,specifier_type);
    }
}

void ParseCompSt(syntaxNode *cur)
{
    //    CompSt → LC DefList StmtList RC
    region_depth++;
    syntaxNode *deflist = cur->first_child->next_sibling, *stmtlist = deflist->next_sibling;
    assert(deflist);
    ParseDefList(deflist);
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
//todo if/while
void ParseStmt(syntaxNode *cur)
{
    //    Stmt → IF LP Exp RP Stmt | IF LP Exp RP Stmt ELSE Stmt | WHILE LP Exp RP Stmt
    syntaxNode *body = cur->first_child;
    // Stmt → Exp SEMI
    if (astcmp(body, "Exp"))
    {
        ParseExp(body);
    }
    // Stmt → CompSt
    else if(astcmp(body,"CompSt")){
        ParseCompSt(body);
    }
    // Stmt → RETURN Exp SEMI
    else if(astcmp(body,"RETURN")){
        Type_ptr t = ParseExp(body->next_sibling);
        if(equal_type(t,region_func->type->u.function.ret)==false){
            logSTErrorf(8,body->line,NULL);
        }
    }
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
            type->u.basic = B_INT;
        }
        else
        {
            type->u.basic = B_FLOAT;
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
        sym->type->u.function.params = ParseVarList(body, sym);
    }
    return sym;
}

Symbol_ptr ParseVarList(syntaxNode *cur, Symbol_ptr func)
{
    //    VarList → ParamDec COMMA VarList | ParamDec
    func->type->u.function.params_num++;
    Symbol_ptr paramdec = ParseParamDec(cur->first_child);
    syntaxNode *comma=cur->first_child->next_sibling;
    if(comma){
        paramdec->cross_nxt=ParseVarList(comma->next_sibling,func);
    }else{// ParamDec
        paramdec->cross_nxt=NULL;
    }
    return paramdec;
}

Symbol_ptr ParseParamDec(syntaxNode *cur)
{
    //    ParamDec → Specifier VarDec
    Type_ptr specifier = ParseSpecifier(cur->first_child);
    Symbol_ptr vardec = ParseVarDec(cur->first_child->next_sibling,specifier);
    return vardec;
}

Symbol_ptr ParseVarDec(syntaxNode *cur, Type_ptr specifier_type)
{
    //    VarDec → ID | VarDec LB INT RB
    Symbol_ptr sym = new_symbol(region_depth);
    if(astcmp(cur->first_child,"ID")){
        sym->name = cur->first_child->sval;
        sym->type = specifier_type;
    }else{
        Type_ptr array_type = (Type_ptr)malloc(sizeof(Type));
        array_type->kind=ARRAY;
        array_type->u.array.size=cur->first_child->next_sibling->next_sibling->ival;
        array_type->u.array.elem=specifier_type;
        sym = ParseVarDec(cur->first_child,array_type);
    }
    return sym;
}

void ParseDefList(syntaxNode *cur)
{
    //    DefList → Def DefList | \epsilon
    if (cur->first_child == NULL)
    {
        //     PASS
    }
    else if (astcmp(cur->first_child, "Def"))
    {
        ParseDef(cur->first_child);
        ParseDefList(cur->first_child->next_sibling);
    }
}

void ParseDef(syntaxNode *cur)
{
    //    Def → Specifier DecList SEMI
    Type_ptr specifier = ParseSpecifier(cur->first_child);
    ParseDecList(cur->first_child->next_sibling,specifier);
}

void ParseDecList(syntaxNode *cur, Type_ptr specifier_type)
{
    //    DecList → Dec | Dec COMMA DecList
    Symbol_ptr sym = ParseDec(cur->first_child,specifier_type);
    if(hash_insert(sym)==false){
        logSTErrorf(3,cur->first_child->line,sym->name);
    }
    syntaxNode *comma=cur->first_child->next_sibling;
    if(comma){
        ParseDecList(comma->next_sibling,specifier_type);
    }
}

Symbol_ptr ParseDec(syntaxNode *cur, Type_ptr specifier_type)
{
    //    Dec → VarDec | VarDec ASSIGNOP Exp
    Symbol_ptr sym = ParseVarDec(cur->first_child,specifier_type);
    syntaxNode *assignop=cur->first_child->next_sibling;
    if(assignop&&equal_type(sym->type,ParseExp(assignop->next_sibling))==false){
        logSTErrorf(5,cur->first_child->line,cur->first_child->sval);
    }
    return sym;
}

Type_ptr ParseExp(syntaxNode *cur)
{
    syntaxNode* e1=cur->first_child,
              * e2=e1 ? e1->next_sibling:NULL,
              * e3=e2 ? e2->next_sibling:NULL;
    // Exp → LP Exp RP
    if(astcmp(e1,"LP")){
        ParseExp(e2);
    }
    // Exp → Exp LB Exp RB
    else if(e2&&astcmp(e2,"LB")){
        Type_ptr t1 = ParseExp(e1), t2 = ParseExp(e3);
        if(t1->kind!=ARRAY){
            logSTErrorf(10,e1->line,e1->sval);
        }
        if(equal_type(t2,&INT_TYPE)==false){
            logSTErrorf(12,e1->line,e3->sval);
        }
        return t1->u.array.elem;
    }
    // Exp → Exp DOT ID
    else if(e2&&astcmp(e2,"DOT")){
        Type_ptr t = ParseExp(e1);
        if(t->kind!=STRUCTURE){
            logSTErrorf(13,e1->line,e1->sval);
        }
        // error14 #todo
        return ParseExp(e3);
    }
    // Exp → ID LP Args RP | ID LP RP | ID
    else if (astcmp(e1, "ID")){
        Symbol_ptr id = hash_find(e1->sval);
        if (id == NULL){
            if(e2==NULL){
                logSTErrorf(1,e1->line,e1->sval);
            }else{
                logSTErrorf(2,e1->line,e1->sval);
            }
            return &UNKNOWN_TYPE;
        }else if(e3&&astcmp(e3,"RP")){
            if(id->type->u.function.params_num!=0){
                logSTErrorf(9,e1->line,id->name);
            }
        }else if(e3&& astcmp(e3,"Args")){
            int num=0;
            Symbol_ptr arg = ParseArgs(e3,&num);
            if(id->type->u.function.params_num!=num){
                logSTErrorf(9,e1->line,id->name);
            }else{
                for(Symbol_ptr param=id->type->u.function.params;param;param=param->cross_nxt,arg=arg->cross_nxt){
                    if(equal_type(param->type,arg->type)==false){
                        logSTErrorf(9,e1->line,id->name);
                        break;
                    }
                }
            }
        }
        return id->type;
    // Exp → INT | FLOAT
    } else if(astcmp(e1,"INT")){
        return &INT_TYPE;
    } else if(astcmp(e1,"FLOAT")){
        return &FLOAT_TYPE;
    }
    //    Exp → Exp ASSIGNOP Exp
    else if (e2&&astcmp(e2, "ASSIGNOP"))
    {
        if(!astcmp(e1,"ID")){
            logSTErrorf(6,e1->line,NULL);
            return &UNKNOWN_TYPE;
        }
        Type_ptr t1 = ParseExp(e1), t2 = ParseExp(e3);
        if(equal_type(t1,t2)==false){
            logSTErrorf(5,e1->line,NULL);
        }
        return t1;
    }
    // Exp → Exp AND Exp | Exp OR Exp | Exp RELOP Exp | Exp PLUS Exp | Exp MINUS Exp | Exp STAR Exp | Exp DIV Exp
    else if(e2&&e3&&astcmp(e3,"Exp")){
        if(astcmp(e2,"AND")||astcmp(e2,"OR")){
            Type_ptr t1 = ParseExp(e1), t2 = ParseExp(e3);
            if(!(equal_type(t1,t2)&& equal_type(t1,&INT_TYPE))){
                logSTErrorf(7,e1->line,NULL);
            }
        }else{
            Type_ptr t1 = ParseExp(e1), t2 = ParseExp(e3);
            if( !(equal_type(t1,t2)&&(equal_type(t1,&INT_TYPE)|| equal_type(t1,&FLOAT_TYPE))) ){
                logSTErrorf(7,e1->line,NULL);
            }
        }
    }
    // Exp → MINUS Exp | NOT Exp
    else if(e2&&astcmp(e2, "Exp")){
        ParseExp(e2);
    }
    return &UNKNOWN_TYPE;
}

Symbol_ptr ParseArgs(syntaxNode *cur,int* num) {
//    Args → Exp COMMA Args | Exp
    (*num)++;
    Symbol_ptr sym = (Symbol_ptr)malloc(sizeof(Symbol));
    sym->type= ParseExp(cur->first_child);
    syntaxNode *comma=cur->first_child->next_sibling;
    if(comma){
        sym->cross_nxt = ParseArgs(comma->next_sibling,num);
    }
    return sym;
}
