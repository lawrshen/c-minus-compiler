#ifndef SYMBOL_H
#define SYMBOL_H

#define SYMBOL_SIZE 32767
#define COMPST_SIZE 255
#define CLAIM_SIZE 255

typedef enum _BASIC_TYPE { B_UNKNOWN, B_INT, B_FLOAT } BASIC_TYPE;
typedef enum _KIND { BASIC, ARRAY, STRUCTURE, FUNCTION } KIND;
typedef enum _SEARCH_TYPE {SEARCH_ALL, SEARCH_VARIABLE, SEARCH_FUNCTION, SEARCH_PROTO} SEARCH_TYPE;
typedef struct _Type* Type_ptr;
typedef struct _Symbol* Symbol_ptr;
typedef struct _Type {
    KIND kind;
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
			int is_claim, claim_lineno;
        } function;
    } u;
} Type;

static Type INT_TYPE = {BASIC, {B_INT}};
static Type FLOAT_TYPE = {BASIC, {B_FLOAT}};
static Type UNKNOWN_TYPE = {BASIC, {B_UNKNOWN}};

typedef struct _Symbol {
    char* name;
    Type_ptr type;
	// nxt: the next symbol in same hash slot
	// cross_nxt: the components of structure/function
	// compst_nxt: the next symbol in same block
    Symbol_ptr nxt, cross_nxt, compst_nxt;
    int region, is_activate, is_proto;
} Symbol;

Symbol_ptr new_symbol(int region);
// Hash
void hash_create();
int hash_insert(Symbol_ptr node);
Symbol* hash_find(char* name, SEARCH_TYPE kind);
// Compst
void compst_destroy(int depth);

void _hash_print_all_symbols();

#endif
