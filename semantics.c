#include "semantics.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define astcmp(node, str) \
    (strcmp(node->name, str) == 0)

int anonymous = 0; // anonymous structure counter
bool region_in_structure = false;
char* structure_name = NULL;
int region_depth = 0;
Symbol_ptr region_func=NULL;
// declare list
Symbol_ptr declare_list[DECLARE_SIZE];
int total_declare=0;
const STError SETable[] = {
  {  0, "Pseudo error", "" },
  {  1, "Undefined variable ", "" },
  {  2, "Undefined function ", "" },
  {  3, "Redefined variable ", "" },
  {  4, "Redefined function ", "" },
  {  5, "Type mismatched for assignment", "" },
  {  6, "The left-hand side of an assignment must be a varia-ble", "" },
  {  7, "Type mismatched for operands", "" },
  {  8, "Type mismatched for return" },
  {  9, "Arguments mismatched for function ", "" },
  { 10, "Array access on non-array variable ", "" },
  { 11, "Function call on non-function variable ", "" },
  { 12, "Non integer offset of an array ", "" },
  { 13, "Field access on non-struct variable ", "" },
  { 14, "Access to undefined field ", " in struct" },
  { 15, "Redefinition or initialization of field ", " in struct" },
  { 16, "Duplicated name ", " of struct" },
  { 17, "Use of undefined struct ", "" },
  { 18, "Function ", " declared but not defined" },
  { 19, "Inconsistent declaration of function ", "" },
};

#if COLORFUL
const char* STerrorf="\033[31mError type %d\033[0m at Line \033[31m%d\033[0m: ";
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

void declare_insert(Symbol_ptr func_node) {
    declare_list[total_declare++] = func_node;
}
void declare_check() {
    for (int i = 0; i < total_declare; i++) {
        if (declare_list[i]->type->u.function.is_declare) {
            logSTErrorf(18,declare_list[i]->type->u.function.declare_lineno,declare_list[i]->name);
        }
    }
}

/*** High-Level Definitions ***/
void ParseProgram(syntaxNode *cur)
{
    hash_create();
    InitIO();
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
    //    | Specifier FunDec CompSt | Specifier FunDec SEMI
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
        // error 4 Redefinition of function
        if(astcmp(body->next_sibling,"CompSt")){
            Symbol_ptr dec_sym = hash_find(body->first_child->sval),
                           sym = ParseFunDec(body, specifier,false);
            if((dec_sym==NULL&&hash_insert(sym)==false)||dec_sym&&dec_sym->type->u.function.is_defed){ //undec but def double || dec and def double
                logSTErrorf(4,cur->first_child->line,sym->name);
            }else if(dec_sym&&equal_type(sym->type,dec_sym->type)==false){
                logSTErrorf(19,cur->first_child->line,body->first_child->sval);
                return;
            }else if(dec_sym){
                dec_sym->type->u.function.is_declare=false;
                dec_sym->type->u.function.is_defed=true;
            }
            region_func = sym; //judge RETURN
            ParseCompSt(body->next_sibling);
            region_func = NULL;
        }else{
            Symbol_ptr sym = ParseFunDec(body, specifier,true);
            if(hash_insert(sym)==false){
                Symbol_ptr dec_sym = hash_find(body->first_child->sval);
                if(equal_type(sym->type,dec_sym->type)==false){
                    logSTErrorf(19,cur->first_child->line,body->first_child->sval);
                }
            }
        }
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

void ParseStmt(syntaxNode *cur)
{
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
        if(!(t->kind==BASIC&&t->u.basic==B_UNKNOWN)&&equal_type(t,region_func->type->u.function.ret)==false){
            logSTErrorf(8,body->line,NULL);
        }
    }
    // Stmt → IF LP Exp RP Stmt | IF LP Exp RP Stmt ELSE Stmt
    else if(astcmp(body,"IF")){
        syntaxNode* exp=body->next_sibling->next_sibling,
                  * stmt=exp->next_sibling->next_sibling;
        Type_ptr t = ParseExp(exp);
        if(t->kind!=BASIC){
            logSTErrorf(7,body->line,NULL);
        }
        ParseStmt(stmt);
        if(stmt->next_sibling){
            ParseStmt(stmt->next_sibling->next_sibling);
        }
    }
    // Stmt → WHILE LP Exp RP Stmt
    else if(astcmp(body,"WHILE")){
        syntaxNode* exp=body->next_sibling->next_sibling,
                * stmt=exp->next_sibling->next_sibling;
        Type_ptr t = ParseExp(exp);
        if(t->kind!=BASIC){
            logSTErrorf(7,body->line,NULL);
        }
        ParseStmt(stmt);
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
    syntaxNode *tag=cur->first_child->next_sibling;
    if(astcmp(tag,"Tag")){
        Symbol_ptr sym = hash_find(tag->first_child->sval);
        if(sym == NULL){
            logSTErrorf(17,tag->line,tag->first_child->sval);
            Type_ptr t=(Type_ptr)malloc(sizeof(Type));
            t->kind=STRUCTURE;
            t->u.structure=NULL;
            return t;
        }else{
            return sym->type;
        }
    }else{
        char* name = tag->empty? NULL:tag->first_child->sval;
        if(tag->empty){
            // ID never begins with a space so it's safe!
            name = (char*)malloc(sizeof(char)*32);
            sprintf(name, " ANONYMOUS_STRUCT_%08x", anonymous++);
        }
        Symbol_ptr sym = new_symbol(region_depth);
        sym->name=name;
        sym->type = (Type_ptr)malloc(sizeof(Type));
        sym->type->kind=STRUCTURE;
        sym->is_global=true;
        region_depth++;
        structure_name = name;
        region_in_structure = true;
        sym->type->u.structure = ParseDefList(tag->next_sibling->next_sibling);
        region_in_structure = false;
        structure_name = NULL;
        compst_destroy(region_depth);
        region_depth--;
        if(hash_insert(sym)==false){
            logSTErrorf(16,tag->line,sym->name);
        }
        return sym->type;
    }
}

Symbol_ptr ParseFunDec(syntaxNode *cur, Type_ptr specifier_type, bool is_declare)
{
    //    FunDec → ID LP VarList RP | ID LP RP
    Symbol_ptr sym = new_symbol(region_depth);
    sym->name = cur->first_child->sval;
    sym->type = (Type_ptr)malloc(sizeof(Type));
    sym->type->kind = FUNCTION;
    sym->type->u.function.ret = specifier_type;
    sym->type->u.function.is_declare=is_declare;
    sym->type->u.function.declare_lineno=cur->first_child->line;
    syntaxNode *body = cur->first_child->next_sibling->next_sibling;
    if (astcmp(body, "RP"))
    {
        sym->type->u.function.params_num = 0;
        sym->type->u.function.params = NULL;
    }
    else
    {
        region_depth++;
        sym->type->u.function.params = ParseVarList(body, sym);
        region_depth--;
    }
    if(hash_find(sym->name)==NULL){ //first declare or def
        for(Symbol_ptr p=sym->type->u.function.params;p;p=p->cross_nxt){
            if(hash_insert(p)==false){
                logSTErrorf(3,cur->first_child->line,p->name);
            }
        }
        if(is_declare){
            sym->type->u.function.is_defed=false;
            declare_insert(sym);
        }else{
            sym->type->u.function.is_defed=true;
        }
    }else{
        for(Symbol_ptr p=sym->type->u.function.params;p;p=p->cross_nxt){
            if(hash_find(p->name)==NULL){
                hash_insert(p);
            }
        }
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

Symbol_ptr DecList_last_ptr = NULL;
Symbol_ptr ParseDefList(syntaxNode *cur)
{
    //    DefList → Def DefList | \epsilon
    if (cur->first_child == NULL)
    {
        return NULL;
    }
    else if (astcmp(cur->first_child, "Def"))
    {
        Symbol_ptr sym = ParseDef(cur->first_child);
        Symbol_ptr cur_last_ptr = DecList_last_ptr;
        DecList_last_ptr = NULL;
        cur_last_ptr->cross_nxt=ParseDefList(cur->first_child->next_sibling);
        return sym;
    }
}

Symbol_ptr ParseDef(syntaxNode *cur)
{
    //    Def → Specifier DecList SEMI
    Type_ptr specifier = ParseSpecifier(cur->first_child);
    return ParseDecList(cur->first_child->next_sibling,specifier);
}

Symbol_ptr ParseDecList(syntaxNode *cur, Type_ptr specifier_type)
{
    //    DecList → Dec | Dec COMMA DecList
    Symbol_ptr sym = ParseDec(cur->first_child,specifier_type);
    if(hash_insert(sym)==false){
        if(region_in_structure){
            logSTErrorf(15,cur->first_child->line,sym->name);
        }else{
            logSTErrorf(3,cur->first_child->line,sym->name);
        }
    }
    syntaxNode *comma=cur->first_child->next_sibling;
    if(comma){
        sym->cross_nxt = ParseDecList(comma->next_sibling,specifier_type);
    }
    if (DecList_last_ptr == NULL) {
        DecList_last_ptr = sym;
    }
    return sym;
}

Symbol_ptr ParseDec(syntaxNode *cur, Type_ptr specifier_type)
{
    //    Dec → VarDec | VarDec ASSIGNOP Exp
    Symbol_ptr sym = ParseVarDec(cur->first_child,specifier_type);
    syntaxNode *assignop=cur->first_child->next_sibling;
    if(assignop&&region_in_structure){
        logSTErrorf(15,cur->first_child->line,cur->first_child->first_child->sval);
    }else if(assignop&&equal_type(sym->type,ParseExp(assignop->next_sibling))==false){
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
        return ParseExp(e2);
    }
    // Exp → Exp LB Exp RB
    else if(e2&&astcmp(e2,"LB")){
        Type_ptr t1 = ParseExp(e1), t2 = ParseExp(e3);
        if(t1->kind!=ARRAY){
            logSTErrorf(10,e1->line,e1->first_child->sval);
            return &UNKNOWN_TYPE;
        }
        if(equal_type(t2,&INT_TYPE)==false){
            logSTErrorf(12,e1->line,e1->first_child->sval);// not int offset but array
        }
        return t1->u.array.elem;
    }
    // Exp → Exp DOT ID
    else if(e2&&astcmp(e2,"DOT")){
        Type_ptr t = ParseExp(e1);
        if(t->kind!=STRUCTURE){
            logSTErrorf(13,e1->line,e1->first_child->sval);
            return &UNKNOWN_TYPE;
        }
        Type_ptr ret = NULL;
        for (Symbol_ptr p = t->u.structure; p; p = p->cross_nxt) {
            if (strcmp(p->name, e3->sval) == 0) {
                ret = p->type;
                break;
            }
        }
        if(ret==NULL){
            logSTErrorf(14,e2->line,e3->sval);
            return &UNKNOWN_TYPE;
        }
        return ret;
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
        }else if(e2&&id->type->kind!=FUNCTION){
            logSTErrorf(11,e1->line,e1->sval);
            return &UNKNOWN_TYPE;
        }else if(e3&&astcmp(e3,"RP")&&id->type->u.function.params_num!=0){
            logSTErrorf(9,e1->line,id->name);
            return &UNKNOWN_TYPE;
        }else if(e3&& astcmp(e3,"Args")){
            int num=0;
            Symbol_ptr arg = ParseArgs(e3,&num);
            if(id->type->u.function.params_num!=num){
                logSTErrorf(9,e1->line,id->name);
                return &UNKNOWN_TYPE;
            }else{
                for(Symbol_ptr param=id->type->u.function.params;param;param=param->cross_nxt,arg=arg->cross_nxt){
                    if(equal_type(param->type,arg->type)==false){
                        logSTErrorf(9,e1->line,id->name);
                        return &UNKNOWN_TYPE;
                    }
                }
            }
        }
        return id->type->kind==FUNCTION? id->type->u.function.ret:id->type;
    // Exp → INT | FLOAT
    } else if(astcmp(e1,"INT")){
        return &INT_TYPE;
    } else if(astcmp(e1,"FLOAT")){
        return &FLOAT_TYPE;
    }
    //    Exp → Exp ASSIGNOP Exp
    else if (e2&&astcmp(e2, "ASSIGNOP"))
    {
        if(!((e1->first_child->next_sibling==NULL&&astcmp(e1->first_child,"ID"))
        || (e1->first_child->next_sibling&&astcmp(e1->first_child->next_sibling,"LB"))
        || (e1->first_child->next_sibling&&astcmp(e1->first_child->next_sibling,"DOT")))){
            logSTErrorf(6,e1->line,NULL);
            return &UNKNOWN_TYPE;
        }
        Type_ptr t1 = ParseExp(e1), t2 = ParseExp(e3);
        if(equal_type(t1,t2)==false){
            logSTErrorf(5,e1->line,NULL);
            return &UNKNOWN_TYPE;
        }
        return t1;
    }
    // Exp → Exp AND Exp | Exp OR Exp | Exp RELOP Exp | Exp PLUS Exp | Exp MINUS Exp | Exp STAR Exp | Exp DIV Exp
    else if(e2&&e3&&astcmp(e3,"Exp")){
        Type_ptr t1 = ParseExp(e1), t2 = ParseExp(e3);
        if(astcmp(e2,"AND")||astcmp(e2,"OR")){
            if(!(equal_type(t1,t2)&& equal_type(t1,&INT_TYPE))){
                logSTErrorf(7,e1->line,NULL);
                return &UNKNOWN_TYPE;
            }
        }else{
            if( !(equal_type(t1,t2)&&(equal_type(t1,&INT_TYPE)|| equal_type(t1,&FLOAT_TYPE))) ){
                logSTErrorf(7,e1->line,NULL);
                return &UNKNOWN_TYPE;
            }else if(astcmp(e2,"RELOP")){
                return &INT_TYPE;
            }
        }
        return t1;
    }
    // Exp → MINUS Exp | NOT Exp
    else if(e2&&astcmp(e2, "Exp")){
        return ParseExp(e2);
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

void InitIO() {
    // int read();
    Symbol_ptr read_op = new_symbol(0);
    read_op->name = "read";
    read_op->type = (Type_ptr)malloc(sizeof(Type));
    read_op->type->kind = FUNCTION;
    read_op->type->u.function.ret = &INT_TYPE;
    read_op->type->u.function.is_declare = false;
    read_op->type->u.function.params_num = 0;
    read_op->type->u.function.params = NULL;
    hash_insert(read_op);
    // int write(int);
    Symbol_ptr write_op = new_symbol(0);
    write_op->name = "write";
    write_op->type = (Type_ptr)malloc(sizeof(Type));
    write_op->type->kind = FUNCTION;
    write_op->type->u.function.ret = &INT_TYPE;
    write_op->type->u.function.is_declare = false;
    write_op->type->u.function.params_num = 1;
    Symbol_ptr write_param = new_symbol(0);
    write_param->name = "#output_int";
    write_param->type = &INT_TYPE;
    write_op->type->u.function.params = write_param;
    hash_insert(write_op);
}

