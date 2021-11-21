/*
 * File: ir.h
 * Project: lab3
 * File Created: 2021/11/21
 * Author: Tianjie Shen (lawrshen@smail.nju.edu.cn)
 * -----
 * Copyright 2021 NJU, Tianjie Shen
 */

#ifndef LAB3_IR_H
#define LAB3_IR_H
#include "symbol.h"
#include <stdio.h>

typedef struct Operand_* Operand;
struct Operand_ {
    enum {VARIABLE, CONSTANT, ADDRESS} kind;
    union {
        int var_no;           // OP_TEMP
        int value;            // OP_CONSTANT
    } u;
};

typedef enum IR_TYPE_ {
    IR_LABEL,
    IR_FUNC,
    IR_ASSIGN,
    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,
    IR_GET_ADDR,
    IR_GET_VAL,
    IR_ASSIGN_ADDR,
    IR_GOTO,
    IR_RELOP,
    IR_RET,
    IR_DEC,
    IR_ARG,
    IR_CALL,
    IR_PARAM,
    IR_READ,
    IR_WRITE
} IR_TYPE;

typedef struct InterCode_* InterCode;
struct InterCode_ {
    IR_TYPE kind;
    union {
        struct { Operand right, left; } assign;
        struct { Operand result, op1, op2; } binop;
    } u;
};

typedef struct InterCodes_* InterCodes;
struct InterCodes_ { InterCode code; struct InterCodes_ *prev, *next; };

void insertInterCode(InterCode ic);
void outputInterCodes(FILE* fp);

#endif //LAB3_IR_H
