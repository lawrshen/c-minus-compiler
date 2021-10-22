#include "symbol.h"
#include <stdlib.h>
#include <string.h>

Symbol_ptr hash_table[SYMBOL_SIZE];
Symbol_ptr compst[COMPST_SIZE];

unsigned int hash_pjw(char* name) {
    unsigned int val = 0, i;
    for (; *name; ++name) {
        val = (val << 2) + *name;
        i = val & ~0x3fff;
        if (i) val = (val ^ (i >> 12)) & 0x3fff;
    }
    return val;
}

int cmp_hash(Symbol_ptr node_new, Symbol_ptr node_old) {
    int activate = (node_new->region <= node_old->region);
    if (activate && strcmp(node_new->name, node_old->name) == 0) {
        switch (node_new->type->kind) {
            case FUNCTION:
                if (node_old->type->kind == FUNCTION) return 1;
                break;
            default:
                if (node_old->type->kind != FUNCTION) return 1;
        }
    }
    return 0;
}

Symbol_ptr new_symbol(int region) {
    Symbol_ptr sym=(Symbol_ptr)malloc(sizeof(Symbol));
    sym->name=NULL;
    sym->type = NULL;
    sym->nxt = sym->cross_nxt = sym->compst_nxt = NULL;
    sym->region = region;
    return sym;
}

void hash_create(){
    for(int i=0;i<SYMBOL_SIZE;i++){
        hash_table[i]=NULL;
    }
    for(int i=0;i<COMPST_SIZE;i++){
        compst[i]=NULL;
    }
}

int hash_insert(Symbol_ptr node) {
    unsigned int idx = hash_pjw(node->name);
    if (hash_table[idx] == NULL) {
        hash_table[idx] = node;
    } else {
        Symbol_ptr cur = hash_table[idx];
        while (cur->nxt) {
            if (cmp_hash(node, cur)) return 1;
            cur = cur->nxt;
        }
        if (cmp_hash(node, cur)) return 1;
        cur->nxt = node;
    }
//    compst_insert(node);
    return 0;
}

Symbol_ptr hash_find(char* name) {
    unsigned int idx = hash_pjw(name);
    Symbol_ptr cur = hash_table[idx], opt = NULL;
    while (cur) {
        if (strcmp(name, cur->name) == 0) {
                opt = cur;
        }
        cur = cur->nxt;
    }
    return opt;
}

void compst_destroy(int depth) {

}
