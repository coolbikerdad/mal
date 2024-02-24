/* 
Types
*/

#ifndef __TYPES__H__
#define __TYPES__H__

#include <stdlib.h>

struct Env;

typedef struct node {
    int type;
    struct node *left;
    struct node *right;
    union {
        int int_value;
        char *string_value;
        struct node *(*func_value)(struct node *);
        struct node *node_value;
        struct Env *node_env;
    } value;
} node;

typedef struct Env {
    struct Env *outer;
    node *hashmap;
} Env;

node *newnode(int type, node *left, node *right);
void freenode(node *n);

/* Node types */
#define NODE_NIL 0
#define NODE_LIST 1
#define NODE_SYMBOL 2
#define NODE_INT 3
#define NODE_STRING 4
#define NODE_VEC 5
#define NODE_HASH 6
#define NODE_KEY 7
#define NODE_FUNC 8
#define NODE_TRUE 9
#define NODE_FALSE 10
#define NODE_LAMBDA 11

/* Special Forms */
#define NODE_SPECIAL_START 100
#define NODE_QUOTE 100
#define NODE_QQUOTE 101
#define NODE_UQUOTE 102
#define NODE_SUQUOTE 103
#define NODE_DEREF 104
#define NODE_META 105
#define NODE_LETSTAR 106
#define NODE_DEFBANG 107
#define NODE_DO 108
#define NODE_IF 109
#define NODE_FNSTAR 110


static char *node_types[] = 
    {"nil", "list", "symbol", "int", "string", "vector", "hashmap", "keyword", "function", "true", "false", "lambda", NULL};
static char *special_forms[] = 
    {"quote", "quasiquote", "unquote", "splice-unquote", "deref", "with-meta", "let*", "def!", "do", "if", "fn*", NULL};

#endif