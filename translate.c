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
        translate_FunDec(body);
        translate_CompSt(body->next_sibling);
    }
    // ExtDef -> Specifier FunDec SEMI
    // ExtDef -> Specifier SEMI
}

/*** Declarators ***/

void translate_FunDec(syntaxNode* cur) {
    // FunDec -> ID LP RP
    // FunDec -> ID LP VarList RP
    gen_ir_1(IR_FUNC,new_func(cur->first_child->sval));
    syntaxNode *varlist = cur->first_child->next_sibling->next_sibling;
    if (astcmp(varlist, "VarList")) {
        while(1){
            syntaxNode* paramdec=varlist->first_child;
            gen_ir_1(IR_PARAM,new_var(paramdec->first_child->next_sibling->first_child->sval));
            if(paramdec->next_sibling){
                varlist=paramdec->next_sibling->next_sibling;
            }else{
                break;
            }
        }
    }
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
        translate_Exp(body,NULL);
    }
    // Stmt → CompSt
    else if(astcmp(body,"CompSt")){
        translate_CompSt(body);
    }
    // Stmt -> RETURN Exp SEMI
   else if (astcmp(body, "RETURN")) {
        Operand ret = new_temp();
        translate_Exp(body->next_sibling,ret);
        gen_ir_1(IR_RET,ret);
   }
   // Stmt → IF LP Exp RP Stmt | IF LP Exp RP Stmt ELSE Stmt
   else if (astcmp(body,"IF")) {
       syntaxNode* exp=body->next_sibling->next_sibling,
                 * stmt=exp->next_sibling->next_sibling;
       Operand l1=new_label(),l2=new_label(),l3=new_label();
       translate_Cond(exp,l1,l2);
       gen_ir_1(IR_LABEL,l1);
       translate_Stmt(stmt);
       if(stmt->next_sibling){
           gen_ir_1(IR_GOTO,l3);
       }
       gen_ir_1(IR_LABEL,l2);
       if(stmt->next_sibling){
           translate_Stmt(stmt->next_sibling->next_sibling);
           gen_ir_1(IR_LABEL,l3);
       }
   }
   // Stmt → WHILE LP Exp RP Stmt
   else if (astcmp(body,"WHILE")) {
       syntaxNode* exp=body->next_sibling->next_sibling,
                 * stmt=exp->next_sibling->next_sibling;
       Operand l1=new_label(),l2=new_label(),l3=new_label();
       gen_ir_1(IR_LABEL,l1);
        translate_Cond(exp,l2,l3);
        gen_ir_1(IR_LABEL,l2);
        translate_Stmt(stmt);
        gen_ir_1(IR_GOTO,l1);
        gen_ir_1(IR_LABEL,l3);
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
        Operand tmp = new_temp();
        translate_Exp(assignop->next_sibling,tmp);
        gen_ir_2(IR_ASSIGN, new_var(cur->first_child->first_child->sval), tmp);
    }
}

/*** Expression ***/

void translate_Exp(syntaxNode* cur, Operand place) {
    syntaxNode* e1=cur->first_child,
            * e2=e1 ? e1->next_sibling:NULL,
            * e3=e2 ? e2->next_sibling:NULL;
    // Exp → LP Exp RP
    if(astcmp(e1,"LP")){
       translate_Exp(e2,place);
    }
    // Exp → Exp LB Exp RB
    else if(e2&&astcmp(e2,"LB")) {
//        translate_Exp(e1);
    }
    // Exp → Exp DOT ID
    else if(e2&&astcmp(e2,"DOT")){
//        translate_Exp(e1);
    }
    // Exp → ID LP Args RP | ID LP RP | ID
    else if (astcmp(e1, "ID")){
        // System IO
        if(e2==NULL){
            gen_ir_2(IR_ASSIGN,place, new_var(e1->sval));// #todo opt
        }else if(astcmp(e3,"RP")){
            if(strcmp(e1->sval,"read")==0){
                gen_ir_1(IR_READ,place);
            }else{
                gen_ir_2(IR_CALL,place, new_func(e1->sval));
            }
        }else{
            if(strcmp(e1->sval,"write")==0){
                Operand t=new_temp();
                translate_Exp(e3->first_child,t);
                gen_ir_1(IR_WRITE,t);
            }else{
                if(e3->first_child){
                    translate_Args(e3);
                }
                gen_ir_2(IR_CALL,place, new_func(e1->sval));
            }
        }
    }
    // Exp → INT | FLOAT
    else if(astcmp(e1,"INT")){
        Operand op2 = new_int(e1->ival);
        gen_ir_2(IR_ASSIGN,place,op2);
    } else if(astcmp(e1,"FLOAT")){

    }
    // Exp → Exp ASSIGNOP Exp
    else if (e2&&astcmp(e2, "ASSIGNOP")){
        place= new_var(e1->first_child->sval);
        Operand tmp=new_temp();
        translate_Exp(e3,tmp);
        gen_ir_2(IR_ASSIGN,place,tmp);
    }
    // Exp → Exp AND Exp | Exp OR Exp | Exp RELOP Exp | NOT Exp | Exp PLUS Exp | Exp MINUS Exp | Exp STAR Exp | Exp DIV Exp
    else if (e2&&e3&&(astcmp(e3,"Exp")|| astcmp(e1,"NOT"))) {
        if(astcmp(e2,"AND")||astcmp(e2,"OR")||astcmp(e2,"RELOP")||astcmp(e1,"NOT")){
            Operand l1=new_label(),l2=new_label();
            gen_ir_2(IR_ASSIGN,place,new_const("0"));
            translate_Cond(cur,l1,l2);
            gen_ir_1(IR_LABEL,l1);
            gen_ir_2(IR_ASSIGN,place,new_const("1"));
            gen_ir_1(IR_LABEL,l2);
        }else{
            Operand t1=new_temp(),t2=new_temp();
            translate_Exp(e1,t1);
            translate_Exp(e3,t2);
            IR_TYPE arith_type;
            if (astcmp(e2, "PLUS"))
                arith_type = IR_ADD;
            else if (astcmp(e2, "MINUS"))
                arith_type = IR_SUB;
            else if (astcmp(e2, "STAR"))
                arith_type = IR_MUL;
            else if (astcmp(e2, "DIV"))
                arith_type = IR_DIV;
            gen_ir_3(arith_type,place,t1,t2);
        }
    }
    // Exp → MINUS Exp
    else if(e1&&astcmp(e1, "MINUS")) {
        Operand t=new_temp();
        translate_Exp(e2,t);
        gen_ir_3(IR_SUB,place, new_const("0"),t);
    }
}

void translate_Cond(syntaxNode *cur, Operand label_true, Operand label_false) {
    syntaxNode* e1=cur->first_child,
            * e2=e1 ? e1->next_sibling:NULL,
            * e3=e2 ? e2->next_sibling:NULL;
    if(e1&& astcmp(e1,"NOT")){
        translate_Cond(e1,label_false,label_true);
    }else if(e2&& astcmp(e2,"RELOP")){
        Operand t1=new_temp(),t2=new_temp();
        translate_Exp(e1,t1);
        translate_Exp(e3,t2);
        gen_ir_if(e2->sval,t1,t2,label_true);
        gen_ir_1(IR_GOTO,label_false);
    }
    else if(e2&& astcmp(e2,"AND")){
        Operand l1=new_label();
        translate_Cond(e1,l1,label_false);
        gen_ir_1(IR_LABEL,l1);
        translate_Cond(e3,label_true,label_false);
    }
    else if(e2&& astcmp(e2,"OR")){
        Operand l1=new_label();
        translate_Cond(e1,label_true,l1);
        gen_ir_1(IR_LABEL,l1);
        translate_Cond(e3,label_true,label_false);
    }
    else{
        Operand t1=new_temp();
        translate_Exp(cur,t1);
        gen_ir_if("!=",t1,new_const("0"),label_true);
        gen_ir_1(IR_GOTO,label_false);
    }
}

void translate_Args(syntaxNode *cur) {
    //    Args → Exp COMMA Args | Exp
    syntaxNode* exp=cur->first_child;
    Operand t1=new_temp();
    translate_Exp(exp,t1);
    // arg_list
    if(exp->next_sibling){
        translate_Args(exp->next_sibling->next_sibling);
    }
    gen_ir_1(IR_ARG,t1);
}
