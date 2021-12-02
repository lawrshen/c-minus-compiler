/*
 * File: ir.c
 * Project: lab3
 * File Created: 2021/11/21
 * Author: Tianjie Shen (lawrshen@smail.nju.edu.cn)
 * -----
 * Copyright 2021 NJU, Tianjie Shen
 */

#include "ir.h"
#include <stdlib.h>
#include <string.h>

InterCodes ir_head = NULL,ir_tail = NULL;

void insertInterCode(InterCode ic){
    InterCodes ir_tmp = (InterCodes)malloc(sizeof(struct InterCodes_));
    ir_tmp->code=ic;
    ir_tmp->dead=false;
    if(ir_head){
        ir_tmp->prev=ir_tail;
        ir_tmp->next=NULL;
        ir_tail->next=ir_tmp;
        ir_tail=ir_tmp;
    }else{
        ir_head=ir_tail=ir_tmp;
    }
}

size_t output_operandf(char *s, Operand op){
    switch (op->kind)   {
        case OP_TEMP:
            return sprintf(s, "t%u", op->u.temp_no);
        case OP_LABEL:
            return sprintf(s, "label%u", op->u.label_no);
        case OP_VAR:
            return sprintf(s, "%s", op->u.var_name);
        case OP_CONST:
            return sprintf(s, "#%d", op->u.value);
        case OP_RELOP:
            return sprintf(s, "%s", op->u.relop);
        case OP_FUNC:
            return sprintf(s, "%s", op->u.func_name);
        case OP_SIZE:
            return sprintf(s, "%d", op->u.value);
        case OP_ADDR:
            return sprintf(s, "&%s", op->u.addr_name);
        default:
            return sprintf(s, "(NULL)");
    }
}

// Parse and output a line of IR code to file.
static char ir_buffer[4096] = {};
void output_intercode(InterCode ic,FILE* fp){
    char* s=ir_buffer;
    switch(ic->kind){
        case IR_LABEL:
            s += sprintf(s, "LABEL ");
            s += output_operandf(s, ic->u.label.label);
            s += sprintf(s, " :");
            break;
        case IR_FUNC:
            s += sprintf(s, "FUNCTION ");
            s += output_operandf(s, ic->u.function.function);
            s += sprintf(s, " :");
            break;
        case IR_ASSIGN:
            s += output_operandf(s, ic->u.assign.left);
            s += sprintf(s, " := ");
            s += output_operandf(s, ic->u.assign.right);
            break;
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:{
            char op = ' ';
            switch (ic->kind) {
                case IR_ADD:
                    op = '+';
                    break;
                case IR_SUB:
                    op = '-';
                    break;
                case IR_MUL:
                    op = '*';
                    break;
                case IR_DIV:
                    op = '/';
                    break;
                default:
                    break;
            }
            s += output_operandf(s, ic->u.binop.result);
            s += sprintf(s, " := ");
            s += output_operandf(s, ic->u.binop.op1);
            s += sprintf(s, " %c ", op);
            s += output_operandf(s, ic->u.binop.op2);
            break;
        }
        case IR_RET:
            s += sprintf(s, "RETURN ");
            s += output_operandf(s,ic->u.ret.var);
            break;
        case IR_GOTO:
            s += sprintf(s, "GOTO ");
            s += output_operandf(s, ic->u.jump.dest);
            break;
        case IR_JUMP_COND: 
            s += sprintf(s, "IF ");
            s += output_operandf(s, ic->u.jump_cond.op1);
            s += sprintf(s, " ");
            s += output_operandf(s, ic->u.jump_cond.relop);
            s += sprintf(s, " ");
            s += output_operandf(s, ic->u.jump_cond.op2);
            s += sprintf(s, " GOTO ");
            s += output_operandf(s, ic->u.jump_cond.dest);
            break;
        case IR_READ:
            s += sprintf(s, "READ ");
            s += output_operandf(s,ic->u.read.var);
            break;
        case IR_WRITE:
            s += sprintf(s, "WRITE ");
            s += output_operandf(s,ic->u.write.var);
            break;
        case IR_PARAM:
            s += sprintf(s, "PARAM ");
            s += output_operandf(s,ic->u.param.var);
            break;
        case IR_ARG:
            s += sprintf(s, "ARG ");
            s += output_operandf(s,ic->u.arg.var);
            break;
        case IR_CALL:
            s += output_operandf(s,ic->u.call.left);
            s += sprintf(s, " := CALL ");
            s += output_operandf(s,ic->u.call.right);
            break;
        case IR_DEC:
            s += sprintf(s, "DEC ");
            s += output_operandf(s,ic->u.dec.left);
            s += sprintf(s, " ");
            s += output_operandf(s,ic->u.dec.right);
            break;
        case IR_LOAD:
            s += output_operandf(s, ic->u.load.left);
            s += sprintf(s, " := *");
            s += output_operandf(s, ic->u.load.right);
            break;
        case IR_SAVE: 
            s += sprintf(s, "*");
            s += output_operandf(s, ic->u.save.left);
            s += sprintf(s, " := ");
            s += output_operandf(s, ic->u.save.right);
            break;
        default:
            s += sprintf(s, "TODO! ");
            break;
    }
    fprintf(fp, "%s\n", ir_buffer);
}

void outputInterCodes(FILE *fp) {
    InterCodes p = ir_head;
    while (p) {
        if(p->dead==false){
            output_intercode(p->code,fp);
        }
        p = p->next;
    }
}

/*** Intercode Creation ***/

void gen_ir_1(IR_TYPE type, Operand op1) {
    InterCode ic = (InterCode)malloc(sizeof(struct InterCode_));
    ic->kind=type;
    switch(type){
        case IR_LABEL:
            ic->u.label.label=op1;break;
        case IR_FUNC:
            ic->u.function.function=op1;break;
        case IR_GOTO:
            ic->u.jump.dest=op1;break;
        case IR_RET:
            ic->u.ret.var=op1;break;
        case IR_ARG:
            ic->u.arg.var=op1;break;
        case IR_PARAM:
            ic->u.param.var=op1;break;
        case IR_READ:
            ic->u.read.var=op1;break;
        case IR_WRITE:
            ic->u.write.var=op1;break;
        default:break;
    }
    insertInterCode(ic);
}

void gen_ir_2(IR_TYPE type, Operand op1, Operand op2) {
    InterCode ic = (InterCode)malloc(sizeof(struct InterCode_));
    ic->kind=type;
    switch(type){
        case IR_ASSIGN:
            ic->u.assign.left=op1,ic->u.assign.right=op2;break;
        case IR_CALL:
            ic->u.call.left=op1,ic->u.call.right=op2;break;
        case IR_DEC:
            ic->u.dec.left=op1,ic->u.dec.right=op2;break;
        case IR_LOAD:
            ic->u.load.left=op1,ic->u.load.right=op2;break;
        case IR_SAVE:
            ic->u.save.left=op1,ic->u.save.right=op2;break;
        default:break;
    }
    insertInterCode(ic);
}

void gen_ir_3(IR_TYPE type, Operand op1, Operand op2, Operand op3) {
    InterCode ic = (InterCode)malloc(sizeof(struct InterCode_));
    ic->kind=type;
    switch(type){
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:
            ic->u.binop.result=op1,ic->u.binop.op1=op2,ic->u.binop.op2=op3;break;
        default:break;
    }
    insertInterCode(ic);
}

void gen_ir_if(char *relop, Operand op1, Operand op2, Operand op3) {
    InterCode ic = (InterCode)malloc(sizeof(struct InterCode_));
    ic->kind= IR_JUMP_COND;
    ic->u.jump_cond.op1=op1;
    ic->u.jump_cond.op2=op2;
    ic->u.jump_cond.dest=op3;
    ic->u.jump_cond.relop=new_relop(relop);
    insertInterCode(ic);
}

/*** Operand Creation ***/

int label_num = 1;
Operand new_label() {
    Operand label = (Operand)malloc(sizeof(struct Operand_));
    label->kind = OP_LABEL;
    label->u.label_no = label_num++;
    return label;
}

int temp_num = 1;
Operand new_temp() {
    Operand tmp = (Operand)malloc(sizeof(struct Operand_));
    tmp->kind = OP_TEMP;
    tmp->u.temp_no = temp_num++;
    return tmp;
}

Operand new_var(char* sval){
    Operand tmp = (Operand)malloc(sizeof(struct Operand_));
    tmp->kind = OP_VAR;
    strcpy(tmp->u.var_name, sval);
    return tmp;
}

Operand new_const(char* val) {
    Operand tmp = (Operand)malloc(sizeof(struct Operand_));
    tmp->kind = OP_CONST;
    tmp->u.value = atoi(val);
    return tmp;
}

Operand new_int(int val) {
    Operand tmp = (Operand)malloc(sizeof(struct Operand_));
    tmp->kind = OP_CONST;
    tmp->u.value = val;
    return tmp;
}

Operand new_func(char* val) {
    Operand tmp = (Operand)malloc(sizeof(struct Operand_));
    tmp->kind = OP_FUNC;
    strcpy(tmp->u.func_name, val);
    return tmp;
}

Operand new_relop(char* relop){
    Operand tmp = (Operand)malloc(sizeof(struct Operand_));
    tmp->kind = OP_RELOP;
    strcpy(tmp->u.relop, relop);
    return tmp;
}

Operand new_size(int val) {
    Operand tmp = (Operand)malloc(sizeof(struct Operand_));
    tmp->kind = OP_SIZE;
    tmp->u.value = val;
    return tmp;
}

Operand new_addr(char *val) {
    Operand tmp = (Operand)malloc(sizeof(struct Operand_));
    tmp->kind = OP_ADDR;
    strcpy(tmp->u.addr_name, val);
    return tmp;
}
