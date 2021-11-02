#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdbool.h>

#define SYMBOL_SIZE 0x3fff
#define COMPST_SIZE 0xff
#define DECLARE_SIZE 0xff

typedef enum _BASIC_TYPE { B_UNKNOWN, B_INT, B_FLOAT } BASIC_TYPE;
typedef struct _Type* Type_ptr;
typedef struct _Symbol* Symbol_ptr;
typedef struct _Type {
    enum { BASIC, ARRAY, STRUCTURE, FUNCTION } kind;
    union {
        // BASIC
        int basic;
        // ARRAY
        struct {
            Type_ptr elem;
            int size;
        } array;
        // STRUCTRURE
        Symbol_ptr structure;
        // FUNCTION
        struct {
            Type_ptr ret;
            int params_num;
            Symbol_ptr params;
            bool is_declare,is_defed;
            int declare_lineno;
        } function;
    } u;
} Type;

static Type INT_TYPE = {BASIC, {B_INT}};
static Type FLOAT_TYPE = {BASIC, {B_FLOAT}};
static Type UNKNOWN_TYPE = {BASIC, {B_UNKNOWN}};

bool equal_type(Type_ptr t1,Type_ptr t2);

typedef struct _Symbol {
    char* name;
    Type_ptr type;
	// nxt: the next symbol in same hash slot
	// cross_nxt: the components of structure/function
	// compst_nxt: the next symbol in same block
    Symbol_ptr nxt, cross_nxt, compst_nxt;
    int region;
    bool is_activate,is_global;
} Symbol;

Symbol_ptr new_symbol(int region);
// Hash
void hash_create();
bool hash_insert(Symbol_ptr node);
Symbol_ptr hash_find(char* name);
// Compst
void compst_destroy(int depth);

#endif
