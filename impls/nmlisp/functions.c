#include <stdio.h>
#include <string.h>
#include "types.h"
#include "env.h"
#include "printer.h"

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

node *integer_le(node *list)
{
    /* The '<=' function */

    int a = 1, b = 0;
    node *l = list;
    node *answer = newnode(NODE_FALSE,NULL,NULL);

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
    answer -> type = (a <= b? NODE_TRUE: NODE_FALSE);
    return answer;
}

node *integer_ge(node *list)
{
    /* The >= function */

    int a = 0, b = 1;
    node *l = list;
    node *answer = newnode(NODE_FALSE,NULL,NULL);

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
    answer -> type = (a >= b? NODE_TRUE: NODE_FALSE);
    return answer;
}

node *integer_gt(node *list)
{
    node *a = integer_le(list);
    a -> type = (a -> type == NODE_TRUE? NODE_FALSE: NODE_TRUE);
    return a;
}

node *integer_lt(node *list)
{
    node *a = integer_ge(list);
    a -> type = (a -> type == NODE_TRUE? NODE_FALSE: NODE_TRUE);
    return a;
}

node *list(node *l)
{
    if(!l)
        return newnode(NODE_LIST, NULL, NULL);
    return l;
}

node *list_query(node *l)
{
    if(l && l -> left && l -> left -> type == NODE_LIST)
        return newnode(NODE_TRUE, NULL, NULL);
    else
        return newnode(NODE_FALSE, NULL, NULL);
}

node *empty_query(node *t)
{
    node *l;

    if(t && t -> left && (t -> left -> type == NODE_LIST || t -> left -> type == NODE_VEC)) {
        l = t -> left;
        /* First parameter is a list. Is it empty? */
        if(l -> left == NULL && l -> right == NULL)
            return newnode(NODE_TRUE, NULL, NULL);
    }
    return newnode(NODE_FALSE, NULL, NULL);
}

node *count_list(node *t)
{
    node *l = t -> left;
    node *a = newnode(NODE_INT, NULL, NULL);
    a -> value.int_value = 0;

    while(l && (l -> type == NODE_LIST || l -> type == NODE_VEC) && l -> left) {
        a -> value.int_value++;
        l = l -> right;
    }
    return a;
}

int node_equal(node *);
int nodes_equal(node *, node *);

node *equal(node *t)
{
    node *a = newnode(node_equal(t), NULL, NULL);
    return a;
}

int node_equal(node *t)
{
    node *x = (t? t -> left: NULL);
    node *y = (t && t -> right? t -> right -> left: NULL);
    return nodes_equal(x,y);
}

int nodes_equal(node *x, node *y) 
{
    int a = NODE_FALSE;
    int xtype, ytype;

    if(!x && !y)
        return NODE_TRUE;

    if(!x || !y)
        return NODE_FALSE;

    /* Need to treat lists and vectors as basically the same */
    xtype = x -> type;
    ytype = y -> type;

    if(xtype == NODE_VEC) xtype = NODE_LIST;
    if(ytype == NODE_VEC) ytype = NODE_LIST;

    if(!x || !y || xtype != ytype)
        return a;

    if(x -> type == NODE_NIL || x -> type == NODE_TRUE || x -> type == NODE_FALSE)
        return NODE_TRUE;

    if(x -> type == NODE_INT) {
        if(x -> value.int_value == y -> value.int_value)
            a = NODE_TRUE;
        return a;
    }

    if(x -> type == NODE_STRING || x -> type == NODE_KEY) {
        if(strcmp(x -> value.string_value, y -> value.string_value) == 0)
            a = NODE_TRUE;
        return a;
    }

    if(xtype == NODE_LIST || xtype == NODE_HASH || xtype == NODE_VEC) {
        if(x -> left == NULL && y -> left == NULL)
            return NODE_TRUE;
        if(nodes_equal(x -> left, y -> left) == NODE_FALSE)
            return NODE_FALSE;
        return nodes_equal(x -> right, y -> right);
    }

    return a;
}

node *repl_env() 
{
    node *env = NULL;
    env = add_func(env, "+", integer_add);
    env = add_func(env, "-", integer_sub);
    env = add_func(env, "*", integer_mul);
    env = add_func(env, "/", integer_div);
    return env;
}

void env_add_func(Env *env, char *name, node *(*func)(node *))
{
    env_set(env, newsymbol(name), newfunc(func));
}

Env *new_repl_env() 
{
    Env *env = newenv(NULL, NULL, NULL);
    env_add_func(env, "+", integer_add);
    env_add_func(env, "-", integer_sub);
    env_add_func(env, "*", integer_mul);
    env_add_func(env, "/", integer_div);
    env_add_func(env, "<", integer_lt);
    env_add_func(env, "<=", integer_le);
    env_add_func(env, ">", integer_gt);
    env_add_func(env, ">=", integer_ge);  
    env_add_func(env, "list", list);
    env_add_func(env, "list?", list_query);
    env_add_func(env, "empty?", empty_query);
    env_add_func(env, "count", count_list);
    env_add_func(env, "=", equal);
    env_add_func(env, "prn", prn);
    env_add_func(env, "pr-str", pr_dash_str);
    env_add_func(env, "println", println);
    env_add_func(env, "str", str);
    return env;
}
