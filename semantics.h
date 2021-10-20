#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "tree.h"
#include "symbol.h"

enum SemanticErrors {
  SE_PSEUDO_ERROR                   = 0,
  SE_VARIABLE_UNDEFINED             = 1,
  SE_FUNCTION_UNDEFINED             = 2,
  SE_VARIABLE_DUPLICATE             = 3,
  SE_FUNCTION_DUPLICATE             = 4,
  SE_MISMATCHED_ASSIGNMENT          = 5,
  SE_RVALUE_ASSIGNMENT              = 6,
  SE_MISMATCHED_OPERANDS            = 7,
  SE_MISMATCHED_RETURN              = 8,
  SE_MISMATCHED_SIGNATURE           = 9,
  SE_ACCESS_TO_NON_ARRAY            = 10,
  SE_ACCESS_TO_NON_FUNCTION         = 11,
  SE_NON_INTEGER_INDEX              = 12,
  SE_ACCESS_TO_NON_STRUCT           = 13,
  SE_STRUCT_FIELD_UNDEFINED         = 14,
  SE_STRUCT_FIELD_DUPLICATE         = 15,
  SE_STRUCT_FIELD_INITIALIZED       = 15, // same ID
  SE_STRUCT_DUPLICATE               = 16,
  SE_STRUCT_UNDEFINED               = 17,
  SE_FUNCTION_DECLARED_NOT_DEFINED  = 18,
  SE_FUNCTION_CONFLICTING           = 19,
};

void semanticScan();
void checkSemantics(syntaxNode *node, syntaxNode *parent);

/*** High-Level Definitions ***/
void ParseProgram(syntaxNode* cur);
void ParseExtDefList(syntaxNode* cur);
void ParseExtDef(syntaxNode* cur);
void ParseExtDecList(syntaxNode* cur, Type_ptr specifier_type);

/*** Specifiers ***/
Type_ptr Specifier(syntaxNode* cur);
Type_ptr StructSpecifier(syntaxNode* cur);

/*** Declarators ***/
Symbol_ptr FunDec(syntaxNode* cur, Type_ptr specifier_type, int);
Symbol_ptr VarList(syntaxNode* cur, Symbol_ptr func);
Symbol_ptr ParamDec(syntaxNode* cur);
Symbol_ptr VarDec(syntaxNode* cur, Type_ptr specifier_type);

/*** Local Definitions ***/
Symbol_ptr DefList(syntaxNode* cur);
Symbol_ptr Def(syntaxNode* cur);
Symbol_ptr DecList(syntaxNode* cur, Type_ptr specifier_type);
Symbol_ptr Dec(syntaxNode* cur, Type_ptr specifier_type);

/*** Statments ***/
void CompSt(syntaxNode* cur);
void StmtList(syntaxNode* cur);
void Stmt(syntaxNode* cur);

/*** Expression ***/
Type_ptr Exp(syntaxNode* cur);

#endif // SEMANTIC_H
