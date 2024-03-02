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
node *newsymbol(char *)
;
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
#define NODE_MACRO 12
#define NODE_ATOM 13

/* Special Forms */
#define NODE_SPECIAL_START 100
#define NODE_QUOTE 100
#define NODE_QQUOTE 101
#define NODE_UQUOTE 102
#define NODE_SUQUOTE 103
#define NODE_META 104
#define NODE_LETSTAR 105
#define NODE_DEFBANG 106
#define NODE_DO 107
#define NODE_IF 108
#define NODE_FNSTAR 109
#define NODE_QQE 110
#define NODE_DEFMACRO 111
#define NODE_MACROEXPAND 112

static char *node_types[] = 
    {"nil", "list", "symbol", "int", "string", "vector", "hashmap", "keyword", "function", "true", "false", "lambda", "macro", "atom", NULL};
static char *special_forms[] = 
    {"quote", "quasiquote", "unquote", "splice-unquote", "with-meta", "let*", "def!", "do", "if", "fn*", "quasiquoteexpand", "defmacro!", "macroexpand", NULL};

#endif