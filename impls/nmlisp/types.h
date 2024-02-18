/* 
Types
*/

#ifndef __TYPES__H__
#define __TYPES__H__

typedef struct node {
    int type;
    struct node *left;
    struct node *right;
    union {
        int int_value;
        char *string_value;
    } value;
} node;

node *newnode(int type, node *left, node *right);
void freenode(node *n);

#define NODE_NIL 0
#define NODE_LIST 1
#define NODE_SYMBOL 2
#define NODE_INT 3
#define NODE_STRING 4
#define NODE_VEC 5
#define NODE_HASH 6
#define NODE_QUOTE 7
#define NODE_QQUOTE 8
#define NODE_UQUOTE 9
#define NODE_SUQUOTE 10
#define NODE_DEREF 11
#define NODE_META 12

static char *node_types[] = {"nil", "list", "symbol", "int", "string", "vector", "hashmap",
    "quote", "quasiquote", "unquote", "splice-unquote", "deref", "with-meta"};

#endif