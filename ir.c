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
    if(ir_head){
        ir_tmp->prev=ir_tail;
        ir_tmp->next=NULL;
        ir_tail->next=ir_tmp;
        ir_tail=ir_tmp;
    }else{
        ir_head=ir_tail=ir_tmp;
    }
}

void output_op(Operand op, FILE *fp) {
    switch (op->kind) {
        case OP_LABEL:
            fprintf(fp,"LABEL %d :",op->u.label_no);
            break;
        case OP_CONSTANT:
            fprintf(fp, "#%d", op->u.value);
            break;
        default:
            break;
    }
}

size_t output_operandf(char *s, Operand op) {
    switch (op->kind) {
        case OP_TEMP:
            return sprintf(s, "t%u", op->u.temp_no);
        case OP_LABEL:
            return sprintf(s, "label%u", op->u.label_no);
        case OP_VARIABLE:
            return sprintf(s, "%s", op->u.var_name);
        case OP_CONSTANT:
            return sprintf(s, "#%d", op->u.value);
        case OP_FUNCTION:
            return sprintf(s, "%s", op->u.func_name);
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
        default:
            break;
    }
    fprintf(fp, "%s\n", ir_buffer);
}

void outputInterCodes(FILE *fp) {
    InterCodes p = ir_head;
    while (p) {
        output_intercode(p->code,fp);
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
        default:break;
    }
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
    tmp->kind = OP_VARIABLE;
    strcpy(tmp->u.var_name, sval);
    return tmp;
}

Operand new_const(char* val) {
    Operand tmp = (Operand)malloc(sizeof(struct Operand_));
    tmp->kind = OP_CONSTANT;
    tmp->u.value = atoi(val);
    return tmp;
}

Operand new_int(int val) {
    Operand tmp = (Operand)malloc(sizeof(struct Operand_));
    tmp->kind = OP_CONSTANT;
    tmp->u.value = val;
    return tmp;
}

Operand new_func(char* val) {
    Operand tmp = (Operand)malloc(sizeof(struct Operand_));
    tmp->kind = OP_FUNCTION;
    strcpy(tmp->u.func_name, val);
    return tmp;
}
