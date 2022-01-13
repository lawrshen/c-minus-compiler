/*
 * File: asm.h
 * Project: lab4
 * File Created: 2021/12/3
 * Author: Tianjie Shen (lawrshen@smail.nju.edu.cn)
 * -----
 * Copyright 2021 NJU, Tianjie Shen
 */

#ifndef LAB4_ASM_H
#define LAB4_ASM_H
#include "ir.h"

static const char *mips_header =
        "     .data\n"
        "_prompt: .asciiz \"Enter an integer:\"\n"
        "_ret: .asciiz \"\\n\"\n"
        "\n"
        "    .text\n"
        "    .globl main\n"
        "\n"
        "read:\n"
        "    li      $v0,4\n"
        "    la      $a0,_prompt\n"
        "    syscall\n"
        "    li      $v0,5\n"
        "    syscall\n"
        "    jr      $ra\n"
        "\n"
        "write:\n"
        "    li      $v0,1\n"
        "    syscall\n"
        "    li      $v0,4\n"
        "    la      $a0,_ret\n"
        "    syscall\n"
        "    move    $v0,$0\n"
        "    jr      $ra\n\n";

static const char *registers[] = {
  "$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3",
  "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7",
  "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7",
  "$t8", "$t9", "$k0", "$k1", "$gp", "$sp", "$fp", "$ra"
};

void outputMips(FILE* fp);

#endif //LAB4_ASM_H
