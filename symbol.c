#include "symbol.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Symbol_ptr hash_table[SYMBOL_SIZE];
Symbol_ptr compst[COMPST_SIZE];

// equal_type return true if t1 is equal to t2
bool equal_type(Type_ptr t1, Type_ptr t2) {
    if(t1->kind!=t2->kind){
        return (t1->kind==BASIC&&t1->u.basic==B_UNKNOWN)||(t2->kind==BASIC&&t2->u.basic==B_UNKNOWN);// duplicate error info
    }else if(t1->kind==BASIC){
        return t1->u.basic==t2->u.basic;
    }else if(t1->kind==ARRAY){
        return equal_type(t1->u.array.elem,t2->u.array.elem);
    }else if(t1->kind==STRUCTURE){
        return strcmp(t1->u.structure->name,t2->u.structure->name)==0; // Name equivalence
    }else if(t1->kind==FUNCTION){
        if(!(t1->u.function.params_num==t2->u.function.params_num&&
           equal_type(t1->u.function.ret,t2->u.function.ret))){
            return false;
        }
        for(Symbol_ptr p1=t1->u.function.params,p2=t2->u.function.params;p1&&p2;p1=p1->cross_nxt,p2=p2->cross_nxt){
            if(equal_type(p1->type,p2->type)==false){
                return false;
            }
        }
        return true;
    }
}

unsigned int hash_pjw(char* name) {
    unsigned int val = 0, i;
    for (; *name; ++name) {
        val = (val << 2) + *name;
        i = val & ~0x3fff;
        if (i) val = (val ^ (i >> 12)) & 0x3fff;
    }
    return val;
}

// hash_cmp return 0 if node_new is equal to node_old in hash slot
int hash_cmp(Symbol_ptr node_new, Symbol_ptr node_old) {
    if (strcmp(node_new->name, node_old->name) == 0) {
        switch (node_new->type->kind) {
            case FUNCTION:
                if (node_old->type->kind == FUNCTION) return 0;
                break;
            default:
                if (node_old->type->kind != FUNCTION) return 0;
        }
    }
    return 1;
}

Symbol_ptr new_symbol(int region) {
    Symbol_ptr sym=(Symbol_ptr)malloc(sizeof(Symbol));
    sym->name = NULL;
    sym->type = NULL;
    sym->nxt  = sym->cross_nxt = sym->compst_nxt = NULL;
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

// hash_insert return true if insert success
bool hash_insert(Symbol_ptr node) {
    unsigned int idx = hash_pjw(node->name);
    if (hash_table[idx] == NULL) {
        hash_table[idx] = node;
    } else {
        Symbol_ptr cur = hash_table[idx];
        while (cur->nxt) {
            if (hash_cmp(node, cur)==0){
                return false;
            } 
            cur = cur->nxt;
        }
        if (hash_cmp(node, cur)==0){
            return false;
        } 
        cur->nxt = node;
    }
//    compst_insert(node);
    return true;
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

//info
const char* kind_name[] = {"BASIC", "ARRAY", "STRUCT", "FUNC"};
const char* basic_name[] = {"UNKNOWN", "INT", "FLOAT"};
void log_symbol(Symbol_ptr node) {
    printf("%*s%s:", node->region * 2, "", node->name);
}

void log_type(Type_ptr type) {
    Symbol_ptr tmp;
    switch (type->kind) {
        case BASIC:
            printf("%s", basic_name[type->u.basic]);
            break;
        case ARRAY:
            printf("[%d]", type->u.array.size);
            log_type(type->u.array.elem);
            break;
        case FUNCTION:
            log_type(type->u.function.ret);
            printf("(");
            tmp = type->u.function.params;
            for (int i = 0; i < type->u.function.params_num; ++i) {
                if (i > 0) printf(", ");
                log_symbol(tmp);
                log_type(tmp->type);
                tmp = tmp->cross_nxt;
            }
            printf(")");
            break;
        case STRUCTURE:
            printf("{\n");
            tmp = type->u.structure;
            while (tmp) {
                log_symbol(tmp);
                log_type(tmp->type);
                tmp = tmp->cross_nxt;
                printf(";\n");
            }
            printf("}");
            break;
    }
}

void _hash_print_all_symbols() {
    printf("######## SYMBOLE TABLE ########\n");
    for (int i = 0; i < SYMBOL_SIZE; ++i) {
        Symbol_ptr cur = hash_table[i];
        while (cur) {
            log_symbol(cur);
            log_type(cur->type);
            printf("\n");
            cur = cur->nxt;
        }
    }
    printf("######## SYMBOLE TABLE ########\n");
}
