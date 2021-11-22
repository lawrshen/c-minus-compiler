/*
 * File: translate_.h
 * Project: lab3
 * File Created: 2021/11/21
 * Author: Tianjie Shen (lawrshen@smail.nju.edu.cn)
 * -----
 * Copyright 2021 NJU, Tianjie Shen
 */

#ifndef LAB3_Translate_H
#define LAB3_Translate_H

#include "tree.h"
#include "ir.h"

/*** High-Level Definitions ***/
void translate_Program(syntaxNode* cur);
void translate_ExtDefList(syntaxNode* cur);
void translate_ExtDef(syntaxNode* cur);

/*** Declarators ***/
void translate_FunDec(syntaxNode* cur);
void translate_VarList(syntaxNode* cur);
void translate_ParamDec(syntaxNode* cur);
void translate_VarDec(syntaxNode* cur);

/*** Statments ***/
void translate_CompSt(syntaxNode* cur);
void translate_StmtList(syntaxNode* cur);
void translate_Stmt(syntaxNode* cur);

/*** Local Definitions ***/
void translate_DefList(syntaxNode* cur);
void translate_Def(syntaxNode* cur);
void translate_DecList(syntaxNode* cur);
void translate_Dec(syntaxNode* cur);

/*** Expression ***/
void translate_Exp(syntaxNode* cur);

#endif //LAB3_Translate_H
 