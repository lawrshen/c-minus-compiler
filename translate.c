/*
 * File: translate.c
 * Project: lab3
 * File Created: 2021/11/21
 * Author: Tianjie Shen (lawrshen@smail.nju.edu.cn)
 * -----
 * Copyright 2021 NJU, Tianjie Shen
 */

#include "translate.h"

int calculate_StructSize(Type_ptr t);
int calculate_ArraySize(Type_ptr t) {
    if(t->kind==ARRAY){
        return t->u.array.size* calculate_ArraySize(t->u.array.elem);
    }else if(t->kind==STRUCTURE){
        return calculate_StructSize(t);
    }else{
        return 4;
    }
}
int calculate_StructOffset(Type_ptr s, char* id) {
    if(s->kind==STRUCTURE){
        int size = 0;
        for (Symbol_ptr p = s->u.structure; p; p = p->cross_nxt) {
            if (strcmp(p->name, id) == 0) break;
            switch (p->type->kind) {
                case BASIC:
                    size += 4;
                    break;
                case ARRAY:
                    size += calculate_ArraySize(p->type);
                    break;
                case STRUCTURE:
                    size += calculate_StructSize(p->type);
                    break;
                default:
                    break;
            }
        }
        return size;
    }else{
        printf("\033[36m[Debug INFO]:None Offset,mismatch for structure Type!!\n\033[0m");
        return 0;
    }
}
int calculate_StructSize(Type_ptr t){
    return calculate_StructOffset(t,"");
}
Symbol_ptr find_struct_info(syntaxNode* cur){
    while(astcmp(cur,"ID")==false){
        if(astcmp(cur,"Exp")){
            cur=cur->first_child;
        }else{
            cur=cur->first_child->next_sibling;
        }
    }
    return hash_find_nocompst(cur->sval);
}

int array_size_cache[256],array_size_len_cache=0;
bool CACHE_SIZE_AVAILABLE=false;
int array_base_size=4;
int calculate_SubArraySize(syntaxNode* cur,int array_depth){
    if(CACHE_SIZE_AVAILABLE){
        if(array_depth==0){
            return array_base_size;
        }else{
            return array_size_cache[array_size_len_cache-array_depth]*array_base_size;
        }
    }else{
        while(astcmp(cur,"Exp")){
            cur=cur->first_child;
        }
        Symbol_ptr s=hash_find_nocompst(cur->sval);
        int array_size[256];//256d-array...suppose it enough,anyway not good hard-coding
        int i=0;
        Type_ptr t=s->type;
        while(t->kind==ARRAY){
            array_size[i]=t->u.array.size;
            i++;
            t=t->u.array.elem;
        }
        array_base_size=t->kind==STRUCTURE? calculate_StructSize(t):4;
        // store cache
        array_size_len_cache=i;
        for(int j=0;j<array_size_len_cache;j++){
            array_size_cache[j]=array_size[j];
        }
        CACHE_SIZE_AVAILABLE=true;
        if(array_depth==0){
            return array_base_size;
        }else{
            return array_size[i-array_depth]*array_base_size;
        }
    }
}

bool SPECIFIAL_STRUCT_ARRAY =false;

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
            char* var_name;
            if(astcmp(paramdec->first_child->next_sibling->first_child,"ID")){
                var_name=paramdec->first_child->next_sibling->first_child->sval;
                Symbol_ptr s=hash_find_nocompst(var_name);
                if(s&&s->type->kind==STRUCTURE){
                    s->is_array_param=true;
                }
            }else{
                var_name=paramdec->first_child->next_sibling->first_child->first_child->sval;
                Symbol_ptr s=hash_find_nocompst(var_name);
                if(s){//for safe ,although always found
                    s->is_array_param=true;
                }
            }
            gen_ir_1(IR_PARAM,new_var(var_name));
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
    if(astcmp(cur->first_child,"VarDec")){
        // since only int[]...
        if(astcmp(cur->first_child->first_child,"ID")){
            Symbol_ptr s = hash_find_nocompst(cur->first_child->first_child->sval);
            gen_ir_2(IR_DEC, new_var(cur->first_child->first_child->sval), new_size(calculate_ArraySize(s->type)));
        }else{
            translate_VarDec(cur->first_child);
        }
    }else{
        Symbol_ptr s = hash_find_nocompst(cur->first_child->sval);
        if(s&&s->type->kind==STRUCTURE){
            gen_ir_2(IR_DEC, new_var(cur->first_child->sval), new_size(calculate_StructSize(s->type)));
        }
    }
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
        if(tmp->is_addr){
            Operand t=new_temp();
            gen_ir_2(IR_LOAD,t,tmp);
            gen_ir_2(IR_ASSIGN, new_var(cur->first_child->first_child->sval), t);
        }else{
            gen_ir_2(IR_ASSIGN, new_var(cur->first_child->first_child->sval), tmp);
        }
    }
}

/*** Expression ***/
Operand offset_last=NULL,offset_now=NULL;
int ARRAY_DEPTH=0;

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
        Operand t=new_temp();
        translate_Exp(e3,t);
        Operand offset=new_temp();
        gen_ir_3(IR_MUL,offset,t,new_int(calculate_SubArraySize(cur,ARRAY_DEPTH)));
        offset_now=new_temp();
        if(offset_last){
            gen_ir_3(IR_ADD,offset_now,offset,offset_last);
        }
        if(astcmp(e1->first_child,"ID")){
            Operand addr= new_addr(e1->first_child->sval);
            Symbol_ptr s=hash_find_nocompst(e1->first_child->sval);
            if(s->is_array_param){
                addr=new_var(e1->first_child->sval);
            }
            if(place==NULL){
                place=new_temp();
            }
            place->is_addr=true;
            if(offset_last){
                gen_ir_3(IR_ADD,place,addr,offset_now);
            }else{
                gen_ir_3(IR_ADD,place,addr,offset);
            }
            // init array settings
            offset_last=offset_now=NULL;
            CACHE_SIZE_AVAILABLE=false;
            ARRAY_DEPTH=0,array_base_size=4;
        }else{
            offset_last=offset;
            ARRAY_DEPTH++;
            translate_Exp(e1,place);
        }
    }
    // Exp → Exp DOT ID
    else if(e2&&astcmp(e2,"DOT")){
        // Get Address
        Operand t1 = new_temp();
        translate_Exp(e1, t1);
        // Get Offset
        char* id_name = e3->sval;
        Symbol_ptr s=find_struct_info(cur);
        if(s==NULL){//for safe
            printf("\033[36m[Debug INFO]:mismatch for structure dot!!\n\033[0m");
            return ;
        }
        Type_ptr t=s->type;
        while(t->kind==ARRAY){
            t=t->u.array.elem;
        }
        int size = calculate_StructOffset(t, id_name);
        place->is_addr=true;
        gen_ir_3(IR_ADD, place, t1, new_int(size));
    }
    // Exp → ID LP Args RP | ID LP RP | ID
    else if (astcmp(e1, "ID")){
        // System IO
        if(place==NULL){
            place=new_temp();
        }
        if(e2==NULL){
            gen_ir_2(IR_ASSIGN,place, new_var(e1->sval));
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
                if(t->is_addr){
                    Operand t2=new_temp();
                    gen_ir_2(IR_LOAD,t2,t);
                    gen_ir_1(IR_WRITE,t2);
                }else{
                    gen_ir_1(IR_WRITE,t);
                }
            }else{
                if(e3->first_child){
                    translate_Args(e3);
                }
                if(place==NULL){
                    place=new_temp();
                }
                gen_ir_2(IR_CALL,place, new_func(e1->sval));
            }
        }
    }
    // Exp → INT | FLOAT
    else if(astcmp(e1,"INT")){
        Operand op2 = new_int(e1->ival);
        if(place==NULL){
            place=new_temp();
        }
        gen_ir_2(IR_ASSIGN,place,op2);
    } else if(astcmp(e1,"FLOAT")){

    }
    // Exp → Exp ASSIGNOP Exp
    else if (e2&&astcmp(e2, "ASSIGNOP")){
        Operand tmp=new_temp();
        translate_Exp(e3,tmp);
        if(astcmp(e1->first_child,"ID")){
            place= new_var(e1->first_child->sval);
            if(tmp->is_addr){//...=array[]
                Operand t2=new_temp();
                gen_ir_2(IR_LOAD,t2,tmp);
                gen_ir_2(IR_ASSIGN,place,t2);
            }else{
                gen_ir_2(IR_ASSIGN,place,tmp);
            }
        }else{// array[]=...
            place=new_temp();
            translate_Exp(e1,place);
            if(tmp->is_addr){// array[]=array[]
                Operand t2=new_temp();
                gen_ir_2(IR_LOAD,t2,tmp);
                gen_ir_2(IR_SAVE,place,t2);
            }else{
                gen_ir_2(IR_SAVE,place,tmp);
            }
        }
    }
    // Exp → Exp AND Exp | Exp OR Exp | Exp RELOP Exp | NOT Exp | Exp PLUS Exp | Exp MINUS Exp | Exp STAR Exp | Exp DIV Exp
    else if (e2&&e3&&(astcmp(e3,"Exp")|| astcmp(e1,"NOT"))) {
        if(astcmp(e2,"AND")||astcmp(e2,"OR")||astcmp(e2,"RELOP")||astcmp(e1,"NOT")){
            Operand l1=new_label(),l2=new_label();
            if(place==NULL){
                place=new_temp();
            }
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
            Operand ta1=new_temp(),ta2=new_temp();
            if(place==NULL){
                place=new_temp();
            }
            if(t1->is_addr){
                gen_ir_2(IR_LOAD,ta1,t1);
            }
            if(t2->is_addr){
                gen_ir_2(IR_LOAD,ta2,t2);
            }
            if(t1->is_addr&&t2->is_addr){
                gen_ir_3(arith_type,place,ta1,ta2);
            }else if(t1->is_addr){
                gen_ir_3(arith_type,place,ta1,t2);
            }else if(t2->is_addr){
                gen_ir_3(arith_type,place,t1,ta2);
            }else{
                gen_ir_3(arith_type,place,t1,t2);
            }
        }
    }
    // Exp → MINUS Exp
    else if(e1&&astcmp(e1, "MINUS")) {
        if(place==NULL){
            place = new_temp();
        }
        Operand t=new_temp();
        translate_Exp(e2,t);
        if(t->is_addr){
            Operand tmp=new_temp();
            gen_ir_2(IR_LOAD,tmp,t);
            gen_ir_3(IR_SUB,place, new_const("0"),tmp);
        }else{
            gen_ir_3(IR_SUB,place, new_const("0"),t);
        }
    }
}

void translate_Cond(syntaxNode *cur, Operand label_true, Operand label_false) {
    syntaxNode* e1=cur->first_child,
            * e2=e1 ? e1->next_sibling:NULL,
            * e3=e2 ? e2->next_sibling:NULL;
    if(e1&& astcmp(e1,"NOT")){
        translate_Cond(e2,label_false,label_true);
    }else if(e2&& astcmp(e2,"RELOP")){
        Operand t1=new_temp(),t2=new_temp();
        translate_Exp(e1,t1);
        translate_Exp(e3,t2);
        Operand ta1=new_temp(),ta2=new_temp();
        if(t1->is_addr){
            gen_ir_2(IR_LOAD,ta1,t1);
        }
        if(t2->is_addr){
            gen_ir_2(IR_LOAD,ta2,t2);
        }
        if(t1->is_addr&&t2->is_addr){
            gen_ir_if(e2->sval,ta1,ta2,label_true);
        }else if(t1->is_addr){
            gen_ir_if(e2->sval,ta1,t2,label_true);
        }else if(t2->is_addr){
            gen_ir_if(e2->sval,t1,ta2,label_true);
        }else{
            gen_ir_if(e2->sval,t1,t2,label_true);
        }
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
        if(t1->is_addr){
            Operand t2=new_temp();
            gen_ir_2(IR_LOAD,t2,t1);
            gen_ir_if("!=",t2,new_const("0"),label_true);
        }else{
            gen_ir_if("!=",t1,new_const("0"),label_true);
        }
        gen_ir_1(IR_GOTO,label_false);
    }
}

void translate_Args(syntaxNode *cur) {
    //    Args → Exp COMMA Args | Exp
    syntaxNode* exp=cur->first_child;
    Operand t1=new_temp();
    // for array[] arg
    syntaxNode* next_id=exp->first_child,*last_exp=next_id;
    int depth=0;
    while(astcmp(next_id,"Exp")){
        last_exp=next_id;
        next_id=next_id->first_child;
        depth++;
    }
    Symbol_ptr s=hash_find_nocompst(next_id->sval);
    if(s){
        Type_ptr t_elem=s->type;
        for(int idx=depth;idx>0&&t_elem->kind!=BASIC;idx--){
            t_elem=t_elem->u.array.elem;
        }
        if(t_elem->kind==ARRAY){
            if(astcmp(last_exp,"ID")){
                Symbol_ptr s=hash_find_nocompst(last_exp->sval);
                if(s->is_array_param){
                    t1=new_var(last_exp->sval);
                }else{
                    t1=new_addr(last_exp->sval);
                }
            }else{
                syntaxNode* subexp=last_exp->next_sibling->next_sibling;
                Operand temp1=new_temp(),temp2=new_temp();
                translate_Exp(subexp,temp1);
                int offset=calculate_SubArraySize(next_id,depth);
                gen_ir_3(IR_MUL,temp2,temp1,new_int(offset));
                gen_ir_3(IR_ADD,t1,new_addr(next_id->sval),temp2);
            }
        }else if(s->type->kind==STRUCTURE){
            if(astcmp(last_exp,"ID")){
                Symbol_ptr s=hash_find_nocompst(last_exp->sval);
                if(s->is_array_param){
                    t1=new_var(last_exp->sval);
                }else{
                    t1=new_addr(last_exp->sval);
                }
            }else{
                printf("\033[36m[Debug INFO]:Undefined performence about struct!!\n\033[0m");
                return ;
            }
        }else if(s->type->kind==ARRAY&&t_elem->kind==STRUCTURE){
            SPECIFIAL_STRUCT_ARRAY=true;// like a mutex lock
            translate_Exp(exp,t1);
        }else{
            translate_Exp(exp,t1);
        }
    }else{
        translate_Exp(exp,t1);
    }
    // arg_list
    if(exp->next_sibling){
        translate_Args(exp->next_sibling->next_sibling);
    }
    if(t1->is_addr&&SPECIFIAL_STRUCT_ARRAY==false){
        Operand t2=new_temp();
        gen_ir_2(IR_LOAD,t2,t1);
        gen_ir_1(IR_ARG,t2);
    }else{
        gen_ir_1(IR_ARG,t1);
        SPECIFIAL_STRUCT_ARRAY=false;
    }
}
