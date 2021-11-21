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

void output_op(Operand op, FILE *fp) {}

void output_intercode(InterCodes ir,FILE* fp){
    fputs("\n",fp);
}

void outputInterCodes(FILE *fp) {
    InterCodes p = ir_head;
    while (p) {
        output_intercode(p,fp);
        p = p->next;
    }
}