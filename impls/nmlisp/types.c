#include "types.h"
#include <stdio.h>
#include <signal.h>
#include <gc.h>

node *newnode(int type, node *left, node *right)
{
    node *n = GC_MALLOC(sizeof(node));
    if(!n) {
        printf("out of memory\n");
        raise(SIGSEGV);
    }
    n -> type = type;
    n -> left = left;
    n -> right = right;
    return n;
}

void freenode(node *n)
{
        /* May want to zero fields, or recurse the structure one day */
        GC_FREE(n);
}
