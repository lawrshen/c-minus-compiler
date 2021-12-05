/*
 * File: asm.c
 * Project: lab4
 * File Created: 2021/12/3
 * Author: Tianjie Shen (lawrshen@smail.nju.edu.cn)
 * -----
 * Copyright 2021 NJU, Tianjie Shen
 */

#include "asm.h"
#include <string.h>
#include <stdlib.h>

extern InterCodes ir_head;
int fp_offset=0;

int* tmp_offset;
int var_offset[0x3fff];

extern int temp_num,var_num;
int frame_size;
int get_framesize(){
    frame_size = (temp_num + var_num)*4;
    return frame_size;
}

static int array_size_dec[0x3fff];

int arg_cur=0,arg_cnt=0,param_cnt=0;

#define zero_ registers[0]
#define at_   registers[1]
#define v0_   registers[2]
#define v1_   registers[3]
#define a0_   registers[4]
#define a1_   registers[5]
#define a2_   registers[6]
#define a3_   registers[7]
#define t0_   registers[8]
#define t1_   registers[9]
#define t2_   registers[10]

#define li(reg, value) fprintf(fp, "    li      %s,%d\n", reg, value)
#define lw(reg, value) fprintf(fp, "    lw      %s,%d($fp)\n", reg, value)
#define la(reg, value) fprintf(fp, "    la      %s,%d($fp)\n", reg, value)
#define sw(reg, value) fprintf(fp, "    sw      %s,%d($fp)\n", reg, value)

#define push_reg(reg, op) push_reg_(reg, op, fp)
#define push_reg_op1() push_reg_(t0_, op1, fp)

void push_reg_(const char* reg, Operand op, FILE* fp) {
    int offset=0;
    if(op->kind==OP_TEMP){
        offset=tmp_offset[op->u.temp_no];
    }else if(op->kind==OP_VAR){
        offset=var_offset[hash_pjw(op->u.var_name)];
    }
    if (!offset) {
        fp_offset -= 4;
        offset = fp_offset;
    }
    if(op->kind==OP_TEMP){
        tmp_offset[op->u.temp_no]=offset;
    }else if(op->kind==OP_VAR){
        var_offset[hash_pjw(op->u.var_name)]=offset;
    }
    sw(reg, offset);
}

#define pop_reg(reg, op) pop_reg_(reg, op, fp)
#define pop_reg_op1() pop_reg_(t0_, op1, fp)
#define pop_reg_op2() pop_reg_(t1_, op2, fp)
#define pop_reg_op3() pop_reg_(t2_, op3, fp)

void pop_reg_(const char* reg, Operand op, FILE* fp) {
    if (op->kind == OP_CONST) {
        li(reg, op->u.value);
    } else if (op->kind == OP_TEMP){
        lw(reg, tmp_offset[op->u.temp_no]);
    }else if(op->kind == OP_VAR||op->kind == OP_ADDR){
        lw(reg, var_offset[hash_pjw(op->u.var_name)]);
    }else{
        fprintf(stderr,"undefined pop!\n");
    }
}

#define move_reg(to_reg,from_reg) move_reg_(to_reg,from_reg,fp)
#define move_op2_2_op1() move_reg_(t0_,t1_,fp)

void move_reg_(const char *to_reg, const char *from_reg,FILE *fp) {
    fprintf(fp, "    move    %s,%s\n", to_reg, from_reg);
}

void iter_args(InterCodes ir,FILE* fp){
    while(ir->code->kind == IR_ARG){
        arg_cnt++;
        ir=ir->next;
    }
    arg_cur = arg_cnt - 1;
    if(arg_cnt >= 4){
        fprintf(fp, "    subu       $sp,$sp,%d\n", 4 * (arg_cnt - 4));
    }
}

void output_ic_mips(InterCode ic, FILE* fp){
    switch (ic->kind) {
        case IR_LABEL:{
            fprintf(fp, "label%d:\n", ic->u.label.label->u.label_no);
            break;
        }
        case IR_GOTO:{
            fprintf(fp, "    j       label%d\n", ic->u.jump.dest->u.label_no);
            break;
        }
        case IR_JUMP_COND:{
            char* tmp;
            if (strcmp(ic->u.jump_cond.relop->u.relop, "==") == 0) {
                tmp = "beq";
            } else if (strcmp(ic->u.jump_cond.relop->u.relop, "!=") == 0) {
                tmp = "bne";
            } else if (strcmp(ic->u.jump_cond.relop->u.relop, ">") == 0) {
                tmp = "bgt";
            } else if (strcmp(ic->u.jump_cond.relop->u.relop, "<") == 0) {
                tmp = "blt";
            } else if (strcmp(ic->u.jump_cond.relop->u.relop, ">=") == 0) {
                tmp = "bge";
            } else if (strcmp(ic->u.jump_cond.relop->u.relop, "<=") == 0) {
                tmp = "ble";
            }
            Operand op1=ic->u.jump_cond.op1,op2=ic->u.jump_cond.op2;
            pop_reg_op1();
            pop_reg_op2();
            fprintf(fp, "    %s     $t0,$t1,label%d\n", tmp, ic->u.jump_cond.dest->u.label_no);
            break;
        }
        case IR_ARG:{  // Caller: prepare args
            Operand op1=ic->u.arg.var;
            if (op1->kind == OP_CONST) {
                if (arg_cur < 4)
                    fprintf(fp, "    li      $a%d,%d\n", arg_cur, op1->u.value);
                else {
                    pop_reg_op1();
                    fprintf(fp, "    sw      $t0,%d($sp)\n", 4 * (arg_cur - 4));
                }
            } else {
                pop_reg_op1();
                if (arg_cur < 4)
                    fprintf(fp, "    move    $a%d,$t0\n", arg_cur);
                else {
                    fprintf(fp, "    sw      $t0,%d($sp)\n", 4 * (arg_cur - 4));
                }
            }
            arg_cur--;
            break;
        }
        case IR_CALL:{
            Operand op1=ic->u.call.left,op2=ic->u.call.right;
            fprintf(fp, "    jal     %s\n",op2->u.func_name);
            move_reg(t0_,v0_);
            push_reg_op1();
            if (arg_cnt >= 4) {
                fprintf(fp, "    addi     $sp,$sp,%d\n",4 * (arg_cnt - 4));  // restore $sp
            }
            arg_cnt = 0;
            arg_cur = 0;
            break;
        }
        case IR_PARAM:{
            Operand op1=ic->u.param.var;
            if (param_cnt < 4) {
                fprintf(fp, "    move    $t0,$%d\n", 4 + param_cnt);
                push_reg_op1();
            } else {
                var_offset[hash_pjw(op1->u.var_name)] = 4 + (param_cnt - 3) * 4;
            }
            param_cnt++;
            break;
        }
        case IR_FUNC:{
            fprintf(fp, "%s:\n", ic->u.function.function->u.func_name);
            fprintf(fp, "    subu    $sp,$sp,8\n");
            fprintf(fp, "    sw      $ra,4($sp)\n");  // preserve return address
            fprintf(fp, "    sw      $fp,0($sp)\n");  // preserve old fp
            fprintf(fp, "    addi    $fp,$sp,0\n");  // load new fp
            fprintf(fp, "    subu    $sp $sp,%d\n",get_framesize());  // frame size
            fp_offset = 0;
            break;
        }
        case IR_RET:{
            Operand op1=ic->u.ret.var;
            if (op1->kind == OP_CONST) {
                fprintf(fp, "    li      $v0,%d\n", op1->u.value);
            } else {
                pop_reg_op1();
                fprintf(fp, "   move     $v0,$t1\n");
            }
            fprintf(fp, "    addi    $sp,$sp,%d\n",frame_size);               // frame size
            fprintf(fp, "    lw      $fp,0($sp)\n");  // load old fp
            fprintf(fp, "    lw      $ra,4($sp)\n");  // load return address
            fprintf(fp, "    addi    $sp,$sp,8\n");
            fprintf(fp, "    jr      $ra\n");
            break;
        }
        case IR_READ:{
            Operand op1=ic->u.read.var;
            fprintf(fp, "    addi    $sp,$sp, -4\n    sw      $ra,0($sp)\n");
            fprintf(fp, "    jal     read\n    move    $t0,$v0\n");
            push_reg_op1();
            fprintf(fp, "    lw      $ra,0($sp)\n    addi    $sp,$sp,4\n");
            break;
        }
        case IR_WRITE:{
            Operand op1=ic->u.write.var;
            fprintf(fp, "    addi    $sp,$sp,-4\n    sw      $ra,0($sp)\n");
            pop_reg_op1();
            fprintf(fp, "    move    $a0,$t0\n");
            fprintf(fp, "    jal     write\n");
            fprintf(fp, "    lw      $ra,0($sp)\n    addi    $sp,$sp,4\n");
            break;
        }
        case IR_ASSIGN:{
            Operand op1=ic->u.assign.left,op2=ic->u.assign.right;
            if(op2->kind==OP_CONST){// x := #k
                li(t0_,op2->u.value);
            }else if(op2->kind==OP_ADDR){// x := &y
                if (op2->kind == OP_TEMP) {
                    la(t0_, tmp_offset[op2->u.temp_no]);
                } else if (op2->kind == OP_VAR ){
                    lw(t0_, var_offset[hash_pjw(op2->u.var_name)]);
                }
            }else if(op2->kind==OP_TEMP||op2->kind==OP_VAR){// x := y
                pop_reg_op2();
                move_op2_2_op1();
            }
            push_reg_op1();
            break;
        }
        case IR_ADD:{
            Operand op1=ic->u.binop.result,op2=ic->u.binop.op1,op3=ic->u.binop.op2;
            if(op2->kind == OP_ADDR){ // x := &y + ...
                if(op3->kind == OP_CONST){
                    fprintf(fp, "    la      $t0,%d($fp)\n",op3->u.value-array_size_dec[hash_pjw(op2->u.addr_name)]);
                }else if(op3->kind == OP_TEMP || op3->kind == OP_VAR ){
                    fprintf(fp, "    la      $t3,%d($fp)\n",var_offset[hash_pjw(op2->u.addr_name)]);
                    if(op3->kind == OP_TEMP){
                        fprintf(fp, "    lw      $t4,%d($fp)\n",tmp_offset[op3->u.temp_no]);
                    }else if(op3->kind == OP_VAR){
                        fprintf(fp, "    la      $t4,%d($fp)\n",var_offset[hash_pjw(op3->u.var_name)]);
                    }
                    fprintf(fp, "    add     $t5,$t4,$t3\n");
                    fprintf(fp, "    move    $t0,$t5\n");
                }else{
                    fprintf(stderr,"undefined ADDR add!\n");
                }
            }else{
                if (op3->kind == OP_CONST) {
                    pop_reg_op2();
                    fprintf(fp, "    addi    $t0,$t1,%d\n", op3->u.value);
                } else if (op2->kind == OP_CONST) {
                    pop_reg_op3();
                    fprintf(fp, "    addi    $t0,$t2,%d\n", op2->u.value);
                } else {
                    pop_reg_op2();
                    pop_reg_op3();
                    fprintf(fp, "    add     $t0,$t1,$t2\n");
                }
            }
            push_reg_op1();
            break;
        }
        case IR_SUB:{
            Operand op1=ic->u.binop.result,op2=ic->u.binop.op1,op3=ic->u.binop.op2;
            pop_reg_op2();
            if (op3->kind == OP_CONST) {
                fprintf(fp, "    addi     $t0,$t1,%d\n", op3->u.value);
            } else {
                pop_reg_op3();
                fprintf(fp, "    sub     $t0,$t1,$t2\n");
            }
            push_reg_op1();
            break;
        }
        case IR_MUL:{
            Operand op1=ic->u.binop.result,op2=ic->u.binop.op1,op3=ic->u.binop.op2;
            pop_reg_op2();
            pop_reg_op3();
            fprintf(fp, "    mul     $t0,$t1,$t2\n");
            push_reg_op1();
            break;
        }
        case IR_DIV:{
            Operand op1=ic->u.binop.result,op2=ic->u.binop.op1,op3=ic->u.binop.op2;
            pop_reg_op2();
            pop_reg_op3();
            fprintf(fp, "    div     $t1,$t2\n    mflo        $t0\n");
            push_reg_op1();
            break;
        }
        case IR_LOAD:{// x := *y
            Operand op1=ic->u.load.left,op2=ic->u.load.right;
            pop_reg_op2();
            fprintf(fp, "    lw      $t0,0($t1)\n");
            push_reg_op1();
            break;
        }
        case IR_SAVE:{// *x := y
            Operand op1=ic->u.load.left,op2=ic->u.load.right;
            pop_reg_op1();
            pop_reg_op2();
            fprintf(fp, "    sw      $t1,0($t0)\n");
            break;
        }
        case IR_DEC:{
            Operand op1=ic->u.dec.left,op2=ic->u.dec.right;
            array_size_dec[hash_pjw(op1->u.var_name)] = op2->u.value;
            fp_offset -= op2->u.value;
            var_offset[hash_pjw(op1->u.var_name)] = fp_offset;
            break;
        }
        default:
            fprintf(fp,"\n");
            break;
    }
}

void outputMips(FILE *fp) {
    fprintf(fp, "%s", mips_header);
    memset(var_offset,0,sizeof(var_offset));
    tmp_offset = (int*) calloc(sizeof(int),temp_num);
    InterCodes p = ir_head;
    while (p) {
        if(p->dead==false){
            if(arg_cnt==0&&p->code->kind==IR_ARG){
                iter_args(p,fp);
            }
            output_ic_mips(p->code,fp);
        }
        p = p->next;
    }
}
