/*
 * File: translate.c
 * Project: lab3
 * File Created: 2021/11/21
 * Author: Tianjie Shen (lawrshen@smail.nju.edu.cn)
 * -----
 * Copyright 2021 NJU, Tianjie Shen
 */

#include "translate.h"

/*** High-Level Definitions ***/

void translate_Program(syntaxNode* cur) {
    // Program -> ExtDefList
    if (astcmp(cur->first_child, "ExtDefList"))
    {
        translate_ExtDefList(cur->first_child);
    }
}

void translate_ExtDefList(syntaxNode* cur) {
    //    ExtDefList → ExtDef ExtDefList | \epsilon
    if (cur->first_child == NULL)
    {
        //     PSEUDO
    }
    else if (astcmp(cur->first_child, "ExtDef"))
    {
        translate_ExtDef(cur->first_child);
        translate_ExtDefList(cur->first_child->next_sibling);
    }
}

void translate_ExtDef(syntaxNode* cur) {
    // ExtDef -> Specifier ExtDecList SEMI
    // ExtDef -> Specifier FunDec CompSt
    syntaxNode *body = cur->first_child->next_sibling;
    if (astcmp(body, "FunDec") && astcmp(body->next_sibling, "CompSt")) {
        translate_CompSt(body->next_sibling);
    }
    // ExtDef -> Specifier FunDec SEMI
    // ExtDef -> Specifier SEMI
}

/*** Declarators ***/

void translate_FunDec(syntaxNode* cur) {
    // FunDec -> ID LP RP
    // FunDec -> ID LP VarList RP
    syntaxNode *body = cur->first_child->next_sibling->next_sibling;
    if (astcmp(body, "VarList")) {
        translate_VarList(body);
    }
}

void translate_VarList(syntaxNode* cur) {
    translate_ParamDec(cur->first_child);
    // VarList -> ParamDec COMMA VarList
    syntaxNode *comma=cur->first_child->next_sibling;
    if(comma){
        translate_VarList(comma->next_sibling);
    }
    // VarList -> ParamDec
}

void translate_ParamDec(syntaxNode* cur) {
    //    ParamDec → Specifier VarDec
    translate_VarDec(cur->first_child->next_sibling);
}

void translate_VarDec(syntaxNode *cur) {
    //    VarDec → ID | VarDec LB INT RB
}
/*** Statments ***/

void translate_CompSt(syntaxNode* cur) {
    // CompSt -> LC DefList StmtList RC
    syntaxNode *deflist = cur->first_child->next_sibling, *stmtlist = deflist->next_sibling;
    translate_DefList(deflist);
    translate_StmtList(stmtlist);
}

void translate_StmtList(syntaxNode* cur) {
    //    StmtList → Stmt StmtList | \epsilon
    if (cur->first_child == NULL)
    {
        //     PSEUDO
    }
    else if (astcmp(cur->first_child, "Stmt"))
    {
        translate_Stmt(cur->first_child);
        translate_StmtList(cur->first_child->next_sibling);
    }
}

void translate_Stmt(syntaxNode* cur) {
    syntaxNode *body = cur->first_child;
    // Stmt → Exp SEMI
    if (astcmp(body, "Exp")) {
        translate_Exp(body);
    }
    // Stmt → CompSt
    else if(astcmp(body,"CompSt")){
        translate_CompSt(body);
    }
    // Stmt -> RETURN Exp SEMI
    else if (astcmp(body, "RETURN")) {
        translate_Exp(body->next_sibling);
    }
    // Stmt → IF LP Exp RP Stmt | IF LP Exp RP Stmt ELSE Stmt
    else if (astcmp(body,"IF")) {
        syntaxNode* exp=body->next_sibling->next_sibling,
                  * stmt=exp->next_sibling->next_sibling;
        translate_Exp(exp);
        translate_Stmt(stmt);
        if(stmt->next_sibling){
            translate_Stmt(stmt->next_sibling->next_sibling);
        }
    }
    // Stmt → WHILE LP Exp RP Stmt
    else if (astcmp(body,"WHILE")) {
        syntaxNode* exp=body->next_sibling->next_sibling,
                  * stmt=exp->next_sibling->next_sibling;
        translate_Exp(exp);
        translate_Stmt(stmt);
    }
}

/*** Local Definitions ***/
void translate_DefList(syntaxNode* cur){
    //    DefList → Def DefList | \epsilon
    if (cur->first_child == NULL){}
    else if (astcmp(cur->first_child, "Def")){
        translate_Def(cur->first_child);
        translate_DefList(cur->first_child->next_sibling);
    }
}

void translate_Def(syntaxNode* cur){
    //    Def → Specifier DecList SEMI
    translate_DecList(cur->first_child->next_sibling);
}

void translate_DecList(syntaxNode* cur){
    //    DecList → Dec | Dec COMMA DecList
    translate_Dec(cur->first_child);
    syntaxNode *comma=cur->first_child->next_sibling;
    if(comma){
        translate_DecList(comma->next_sibling);
    }
}

void translate_Dec(syntaxNode* cur){
    //    Dec → VarDec | VarDec ASSIGNOP Exp
    translate_VarDec(cur->first_child);
    syntaxNode *assignop=cur->first_child->next_sibling;
    if(assignop){
        translate_Exp(assignop->next_sibling);
    }
}

/*** Expression ***/

void translate_Exp(syntaxNode* cur) {
    syntaxNode* e1=cur->first_child,
            * e2=e1 ? e1->next_sibling:NULL,
            * e3=e2 ? e2->next_sibling:NULL;
    // Exp → LP Exp RP
    if(astcmp(e1,"LP")){
        translate_Exp(e2);
    }
    // Exp → Exp LB Exp RB
    else if(e2&&astcmp(e2,"LB")) {
        translate_Exp(e1);
    }
    // Exp → Exp DOT ID
    else if(e2&&astcmp(e2,"DOT")){
        translate_Exp(e1);
    }
    // Exp → ID LP Args RP | ID LP RP | ID
    else if (astcmp(e1, "ID")){
        if(e2==NULL){//ID

        }
    }
    // Exp → INT | FLOAT
    else if(astcmp(e1,"INT")){

    } else if(astcmp(e1,"FLOAT")){

    }
    // Exp → Exp ASSIGNOP Exp
    else if (e2&&astcmp(e2, "ASSIGNOP")){
        translate_Exp(e1);
        translate_Exp(e3);
    }
    // Exp → Exp AND Exp | Exp OR Exp | Exp RELOP Exp | Exp PLUS Exp | Exp MINUS Exp | Exp STAR Exp | Exp DIV Exp
    else if (e2&&e3&&astcmp(e3,"Exp")) {
        translate_Exp(e1);
        translate_Exp(e3);
    }
    // Exp → MINUS Exp | NOT Exp
    else if(e2&&astcmp(e2, "Exp")) {
        translate_Exp(e2);
    }
}
