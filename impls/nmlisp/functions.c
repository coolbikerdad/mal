#include <stdio.h>
#include <string.h>
#include "types.h"

/*
Functions
*/

/* a NODE_FUNC has a value that is a pointer to a function to call */

node *newfunc(node *(*func)(node *)) 
{
    node *n1 = NULL, *n2 = NULL;
    n1 = newnode(NODE_FUNC,NULL,NULL);
    n1 -> value.func_value = func;
    return n1;
}

node *newsymbol(char *name) 
{
    node *n2 = newnode(NODE_SYMBOL,NULL,NULL);
    n2 -> value.string_value = name;
    return n2;
}

/* An environment is a hashmap (herein a list of pairs) of NODE_SYMBOLs with nodes following as values */

/* For adding values to an environment */
node *add_env(node *env, node *sym, node *val)
{
    node *n1 = newnode(NODE_LIST,val,env);
    node *n2 = newnode(NODE_LIST,sym,n1); 
    return n2;
}

/* Retrieve a value from an environment */
node *env_lookup(char *name, node *env) {
    node *e = env, *s = NULL;
    while(e) {
        if(e -> type != NODE_LIST || !(s = e -> left) || s -> type != NODE_SYMBOL || !e -> right) {
            printf("corrupt environment");
            return NULL;
        }
        if(strcmp(name, s -> value.string_value) == 0) {
            return e -> right -> left;
        }
        e = e -> right -> right;
    }
    return NULL;
}

node *add_func(node *env, char *name, node *(*func)(node *))
{
    return add_env(env, newsymbol(name), newfunc(func));
}

node *integer_add(node *list)
{
    /* The '+' function */
    /* In this case sum all the numbers in the list */
    /* Add error conditions later */

    int sum = 0;
    node *l = list;
    node *answer = newnode(NODE_INT,NULL,NULL);
    while(l && l -> type == NODE_LIST) {
        node *n = l -> left;
        if(n && n -> type == NODE_INT) {
            sum += n -> value.int_value;
        }
        l = l -> right;
    }
    answer -> value.int_value = sum;
    return answer;
}

node *integer_mul(node *list)
{
    /* The '*' function */
    /* In this case multiply all the numbers in the list */
    /* Add error conditions later */

    int sum = 1;
    node *l = list;
    node *answer = newnode(NODE_INT,NULL,NULL);
    while(l && l -> type == NODE_LIST) {
        node *n = l -> left;
        if(n && n -> type == NODE_INT) {
            sum *= n -> value.int_value;
        }
        l = l -> right;
    }
    answer -> value.int_value = sum;
    return answer;
}

node *integer_sub(node *list)
{
    /* The '-' function */
    /* In this case subtract first two numbers in the list */
    /* Add error conditions later */

    int a = 0, b = 0;
    node *l = list;
    node *answer = newnode(NODE_INT,NULL,NULL);

    if(l && l -> type == NODE_LIST) {
        node *n = l -> left;
        if(n && n -> type == NODE_INT) {
            a = n -> value.int_value;
        }
        l = l -> right;
    }
    if(l && l -> type == NODE_LIST) {
        node *n = l -> left;
        if(n && n -> type == NODE_INT) {
            b = n -> value.int_value;
        }
    }
    answer -> value.int_value = a - b;
    return answer;
}

node *integer_div(node *list)
{
    /* The '/' function */
    /* In this case divide first two numbers in the list */
    /* Add error conditions later */

    int a = 0, b = 0;
    node *l = list;
    node *answer = newnode(NODE_INT,NULL,NULL);

    if(l && l -> type == NODE_LIST) {
        node *n = l -> left;
        if(n && n -> type == NODE_INT) {
            a = n -> value.int_value;
        }
        l = l -> right;
    }
    if(l && l -> type == NODE_LIST) {
        node *n = l -> left;
        if(n && n -> type == NODE_INT) {
            b = n -> value.int_value;
        }
    }
    if(b == 0) b = 1;
    answer -> value.int_value = a / b;
    return answer;
}

node *repl_env() {
    node *env = NULL;
    env = add_func(env, "+", integer_add);
    env = add_func(env, "-", integer_sub);
    env = add_func(env, "*", integer_mul);
    env = add_func(env, "/", integer_div);
    return env;
}


