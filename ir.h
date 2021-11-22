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

typedef enum OP_TYPE_ {
    OP_NULL,
    OP_TEMP,
    OP_LABEL,
    OP_VARIABLE,
    OP_VADDRESS,
    OP_MEMBLOCK,
    OP_CONSTANT,
    OP_RELOP,
    OP_FUNCTION,
} OP_TYPE;

typedef struct Operand_* Operand;
struct Operand_ {
    OP_TYPE kind;
    union {
        int value;                          // OP_CONSTANT
        int temp_no,label_no;               // OP_TEMP,OP_LABEL
        char func_name[64],var_name[64];    // OP_FUNC,OP_VARIABLE
    } u;
};

Operand new_label();
Operand new_temp();
Operand new_var(char* sval);
Operand new_int(int val);
Operand new_func(char* val);

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
        struct { Operand label;} label;
        struct { Operand function;} function;
        struct { Operand right, left; } assign, addr, load, save,call,dec;
        struct { Operand result, op1, op2; } binop, addr_offset;
        struct { Operand dest;} jump;
        struct { Operand op1, relop, op2, dest;} jump_cond;
        struct { Operand var;} ret, fence, read, write, arg, param;
    } u;
};

typedef struct InterCodes_* InterCodes;
struct InterCodes_ { InterCode code; struct InterCodes_ *prev, *next; };

void insertInterCode(InterCode ic);
void outputInterCodes(FILE* fp);

void gen_ir_1(IR_TYPE type, Operand op1);
void gen_ir_2(IR_TYPE type, Operand op1, Operand op2);
void gen_ir_3(IR_TYPE type, Operand op1, Operand op2, Operand op3);
void gen_ir_if(char* relop, Operand op1, Operand op2, Operand op3);

#endif //LAB3_IR_H
