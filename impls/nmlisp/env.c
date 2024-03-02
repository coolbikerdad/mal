#include "types.h"
#include "env.h"
#include <gc.h>
#include <stdio.h>
#include <string.h>
#include "exceptions.h"
#include "printer.h"

/* Global repl_env needed for eval */
Env *repl_environment = NULL;

/* An environment is a hashmap (herein a list of pairs) of NODE_SYMBOLs with nodes following as values */

Env *newenv(Env* outer, node *binds, node *exprs)
{
    Env *e = GC_MALLOC(sizeof(Env));
    e -> outer = outer;
    e -> hashmap = NULL;

    while(binds && binds -> left) {
        if(strcmp(binds -> left -> value.string_value, "&") == 0) {
            binds = binds -> right;
            if(!exprs) exprs = newnode(NODE_LIST,NULL,NULL);
            env_set(e, binds -> left, exprs);
            printf("tail binding %s\n",binds -> left -> value.string_value);
            (void) prn(exprs);
            return e;
        }
        if(!exprs || exprs -> left == NULL)
            env_set(e, binds -> left, newnode(NODE_LIST,NULL,NULL));
        else {
            env_set(e, binds -> left, exprs -> left);
            exprs = exprs -> right;
        }
        binds = binds -> right;
    }
    return e;
}

void env_set(Env *env, node *sym, node *val)
{
    env -> hashmap = newnode(NODE_HASH, val, env -> hashmap);
    env -> hashmap = newnode(NODE_HASH, sym, env -> hashmap);
}

Env *env_find(Env *env, node *sym)
{
    node *e = env -> hashmap;
    char *s = sym -> value.string_value;

    while(e) {
        node *n = e -> left;
        if(n -> type == NODE_SYMBOL && strcmp(n -> value.string_value, s) == 0)
            return env;
        e = e -> right;
        e = e -> right;
    }
    if(env -> outer)
        return env_find(env -> outer, sym);
    
    return NULL;
}

node *env_get(Env *env, node *sym)
{
    node *e = env -> hashmap;
    char *s = sym -> value.string_value;

    while(e) {
        node *n = e -> left;
        if(n -> type == NODE_SYMBOL && strcmp(n -> value.string_value, s) == 0) {
            return e -> right -> left;
        }       
        e = e -> right;
        e = e -> right;
    }
    if(env -> outer)
        return env_get(env -> outer, sym);
    
    /* raise not found error */
    printf("symbol %s not found\n", s);
    throw_exception("symbol not found", 1);
    return NULL;
}

int is_macro_call(node *tree, Env *env)
{
    if(tree && tree -> type == NODE_LIST && tree -> left && tree -> left ->type == NODE_SYMBOL) {
        node *sym = tree -> left;
        node *v = env_get(env, sym);
        if(v && v -> type == NODE_MACRO)
            return 1;
    }
    return 0;
}
