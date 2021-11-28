/*
 * File: opt.c
 * Project: lab3
 * File Created: 2021/11/27
 * Author: Tianjie Shen (lawrshen@smail.nju.edu.cn)
 * -----
 * Copyright 2021 NJU, Tianjie Shen
 */

#include "opt.h"
#include "ir.h"
#include<string.h>
extern InterCodes ir_head,ir_tail;
bool tmpactualconst[512];
int tmp2const[512];
bool tmpactualvar[512];
char tmp2var[512][64];

#define TEMP_VAR_TO_CONST(OP_TYPE)                                                    \
    do{                                                                               \
        if (ic->u.OP_TYPE.var->kind == OP_TEMP){                                      \
            if (tmpactualconst[ic->u.OP_TYPE.var->u.temp_no]){                        \
                ic->u.OP_TYPE.var->kind = OP_CONST;                                   \
                ic->u.OP_TYPE.var->u.value = tmp2const[ic->u.OP_TYPE.var->u.temp_no]; \
            }                                                                         \
        }                                                                             \
        return false;                                                                 \
    } while (0)                                                                       \

#define TEMP_VAR_TO_VAR(OP_TYPE)                                                            \
    do{                                                                                     \
        if (ic->u.OP_TYPE.var->kind == OP_TEMP){                                            \
            if (tmpactualvar[ic->u.OP_TYPE.var->u.temp_no]){                                \
                ic->u.OP_TYPE.var->kind = OP_VAR;                                           \
                strcpy(ic->u.OP_TYPE.var->u.var_name,tmp2var[ic->u.OP_TYPE.var->u.temp_no]);\
            }                                                                               \
        }                                                                                   \
        return false;                                                                       \
    } while (0)                                                                             \

// return true if it can evaluate
bool simple_eval(InterCode ic){
    switch (ic->kind){
        case IR_ASSIGN: {
            if(ic->u.assign.left->kind==OP_TEMP&&ic->u.assign.right->kind==OP_CONST){
                tmpactualconst[ic->u.assign.left->u.temp_no]=true;
                tmp2const[ic->u.assign.left->u.temp_no]=ic->u.assign.right->u.value;
                return true;
            }else if(ic->u.assign.right->kind==OP_TEMP){
                if(tmpactualconst[ic->u.assign.right->u.temp_no]){
                    ic->u.assign.right->kind=OP_CONST;
                    ic->u.assign.right->u.value=tmp2const[ic->u.assign.right->u.temp_no];
                }
            }
            return false;
        }
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:{
            Operand op1=ic->u.binop.op1,op2=ic->u.binop.op2;
            //if t1 + t2 actual is 3+4
            if(op1->kind==OP_TEMP&&tmpactualconst[op1->u.temp_no]){
                op1->kind=OP_CONST;
                op1->u.value=tmp2const[op1->u.temp_no];
            }
            if(op2->kind==OP_TEMP&&tmpactualconst[op2->u.temp_no]){
                op2->kind=OP_CONST;
                op2->u.value=tmp2const[op2->u.temp_no];
            }
            if(op1->kind==OP_CONST&&op2->kind==OP_CONST){// #3 + #4
                Operand res=ic->u.binop.result;
                if(res->kind==OP_TEMP){
                    tmpactualconst[res->u.temp_no]=true;
                    switch (ic->kind) {
                        case IR_ADD:
                            tmp2const[res->u.temp_no]=op1->u.value+op2->u.value;break;
                        case IR_SUB:
                            tmp2const[res->u.temp_no]=op1->u.value-op2->u.value;break;
                        case IR_MUL:
                            tmp2const[res->u.temp_no]=op1->u.value*op2->u.value;break;
                        case IR_DIV:
                            tmp2const[res->u.temp_no]=op1->u.value/op2->u.value;break;
                        default:break;
                    }
                    return true;
                }
            }
            return false;
        }
        case IR_JUMP_COND:{
            Operand op1=ic->u.jump_cond.op1,op2=ic->u.jump_cond.op2;
            //if t1 + t2 actual is 3+4
            if(op1->kind==OP_TEMP&&tmpactualconst[op1->u.temp_no]){
                op1->kind=OP_CONST;
                op1->u.value=tmp2const[op1->u.temp_no];
            }
            if(op2->kind==OP_TEMP&&tmpactualconst[op2->u.temp_no]){
                op2->kind=OP_CONST;
                op2->u.value=tmp2const[op2->u.temp_no];
            }
            return false;
        }
        case IR_RET:
            TEMP_VAR_TO_CONST(ret);
        case IR_WRITE:
            TEMP_VAR_TO_CONST(write);
        case IR_ARG:
            TEMP_VAR_TO_CONST(arg);
        default:
            return false;
    }
}
// return true if tmp is actual variable
bool simple_temp_val(InterCode ic){
    switch (ic->kind){
        case IR_ASSIGN: {
            if(ic->u.assign.left->kind==OP_TEMP&&ic->u.assign.right->kind==OP_VAR){
                tmpactualvar[ic->u.assign.left->u.temp_no]=true;
                strcpy(tmp2var[ic->u.assign.left->u.temp_no],ic->u.assign.right->u.var_name);
                return true;
            }else if(ic->u.assign.right->kind==OP_TEMP){
                if(tmpactualvar[ic->u.assign.right->u.temp_no]){
                    ic->u.assign.right->kind=OP_VAR;
                    strcpy(ic->u.assign.right->u.var_name,tmp2var[ic->u.assign.right->u.temp_no]);
                }
            }
            return false;
        }
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:{
            Operand op1=ic->u.binop.op1,op2=ic->u.binop.op2;
            if(op1->kind==OP_TEMP&&tmpactualvar[op1->u.temp_no]){
                op1->kind=OP_VAR;
                strcpy(op1->u.var_name,tmp2var[op1->u.temp_no]);
            }
            if(op2->kind==OP_TEMP&&tmpactualvar[op2->u.temp_no]){
                op2->kind=OP_VAR;
                strcpy(op2->u.var_name,tmp2var[op2->u.temp_no]);
            }
            return false;
        }
        case IR_JUMP_COND:{
            Operand op1=ic->u.jump_cond.op1,op2=ic->u.jump_cond.op2;
            if(op1->kind==OP_TEMP&&tmpactualvar[op1->u.temp_no]){
                op1->kind=OP_VAR;
                strcpy(op1->u.var_name,tmp2var[op1->u.temp_no]);
            }
            if(op2->kind==OP_TEMP&&tmpactualvar[op2->u.temp_no]){
                op2->kind=OP_VAR;
                strcpy(op2->u.var_name,tmp2var[op2->u.temp_no]);
            }
            return false;
        }
        case IR_RET:
            TEMP_VAR_TO_VAR(ret);
        case IR_WRITE:
            TEMP_VAR_TO_VAR(write);
        case IR_ARG:
            TEMP_VAR_TO_VAR(arg);
        default:
            return false;
    }
}

// linear scanning IR to optimize actual const/variable temp
void LinearOptIC() {
    memset(tmpactualconst,0,sizeof(tmpactualconst));
    memset(tmp2const,0,sizeof(tmp2const));
    memset(tmpactualvar,0,sizeof(tmpactualvar));
    memset(tmp2var,0,sizeof(tmp2var));
    for(InterCodes p=ir_head;p!=ir_tail;p=p->next){
        InterCode ic=p->code;
        if(simple_eval(ic)|| simple_temp_val(ic)){
            p->dead=true;
        }
    }
}
