#include "symbol.h"
#include <stdlib.h>
#include <string.h>

Symbol_ptr hash_table[SYMBOL_SIZE];
Symbol_ptr compst[COMPST_SIZE];

void compst_insert(Symbol_ptr node){
    if(compst[node->region]==NULL){
        compst[node->region]=node;
    }else{
        node->compst_nxt=compst[node->region];
        compst[node->region]=node;
    }
}

void compst_destroy(int depth) {
    for(Symbol_ptr p=compst[depth];p;p=p->compst_nxt){
        if(p->is_global==false){
            p->is_activate = false;
        }
    }
    compst[depth]=NULL;
}

// equal_type return true if t1 is equal to t2. UNKNOWN only diff from INT & FLOAT
bool equal_type(Type_ptr t1, Type_ptr t2) {
    if(t1->kind!=t2->kind){
        return (t1->kind==BASIC&&t1->u.basic==B_UNKNOWN)||(t2->kind==BASIC&&t2->u.basic==B_UNKNOWN);// duplicate error info
    }else if(t1->kind==BASIC){
        return t1->u.basic==t2->u.basic||t1->u.basic==B_UNKNOWN||t2->u.basic==B_UNKNOWN;
    }else if(t1->kind==ARRAY){
        return equal_type(t1->u.array.elem,t2->u.array.elem);
    }else if(t1->kind==STRUCTURE){
        if(strcmp(t1->u.structure->name,t2->u.structure->name)==0){// Name equivalence
            return true;
        } else{
            Symbol_ptr p1,p2;
            for(p1=t1->u.structure,p2=t2->u.structure;p1&&p2;p1=p1->cross_nxt,p2=p2->cross_nxt){
                if(equal_type(p1->type,p2->type)==false){
                    return false;
                }
            }
            return (p1==NULL)&&(p2==NULL);
        }
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
    bool activate = (node_new->region <= node_old->region && node_old->is_activate) // new symbol region higher than old (eg. 0 > 1)
                ||node_old->is_global||node_new->is_global; 
    if (activate&&strcmp(node_new->name, node_old->name) == 0) {
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
    sym->is_activate = true;
    sym->is_global = false;
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
   compst_insert(node);
    return true;
}

Symbol_ptr hash_find(char* name) {
    unsigned int idx = hash_pjw(name);
    Symbol_ptr cur = hash_table[idx], opt = NULL;
    while (cur) {
        if (cur->is_activate&&strcmp(name, cur->name) == 0) {
            if (!opt || opt->region < cur->region){
                opt = cur;
            }
        }
        cur = cur->nxt;
    }
    return opt;
}

Symbol_ptr hash_find_nocompst(char* name) {
    unsigned int idx = hash_pjw(name);
    Symbol_ptr cur = hash_table[idx], opt = NULL;
    while (cur) {
        if (strcmp(name, cur->name) == 0) {
            if (!opt || opt->region < cur->region){
                opt = cur;
            }
        }
        cur = cur->nxt;
    }
    return opt;
}