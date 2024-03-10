#include <stdio.h>
#include <string.h>
#include <gc.h>
#include "types.h"
#include "env.h"
#include "printer.h"
#include "reader.h"
#include "exceptions.h"

/* a NODE_FUNC has a value that is a pointer to a function to call */

node *newfunc(node *(*func)(node *)) 
{
    node *n1 = NULL, *n2 = NULL;
    n1 = newnode(NODE_FUNC,NULL,NULL);
    n1 -> value.func_value = func;
    return n1;
}

/* An environment is a hashmap (herein a list of pairs) 
   of NODE_SYMBOLs with nodes following as values */

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
        if(e -> type != NODE_LIST || !(s = e -> left) || 
           s -> type != NODE_SYMBOL || !e -> right) {
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

    int a = 0, b = 1;
    node *l = list;
    node *answer = newnode(NODE_INT,NULL,NULL);
    answer -> value.int_value = 0;

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
    if(b == 0) 
        throw_exception("divide by zero", list, 1);
    else
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

    if(t && t -> left && (t -> left -> type == NODE_LIST || 
                          t -> left -> type == NODE_VEC)) {
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

    while(l && (l -> type == NODE_LIST || l -> type == NODE_VEC) && 
          l -> left) {
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
    int hash_equals(node *, node *);

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

    if(x -> type == NODE_NIL || x -> type == NODE_TRUE || 
                                x -> type == NODE_FALSE)
        return NODE_TRUE;

    if(x -> type == NODE_INT) {
        if(x -> value.int_value == y -> value.int_value)
            a = NODE_TRUE;
        return a;
    }

    if(x -> type == NODE_STRING || x -> type == NODE_KEY || 
                                   x -> type == NODE_SYMBOL) {
        if(strcmp(x -> value.string_value, y -> value.string_value) == 0)
            a = NODE_TRUE;
        return a;
    }

    if(xtype == NODE_HASH)
        return hash_equals(x, y);

    if(xtype == NODE_LIST || xtype == NODE_VEC) {
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

node *EVAL(node *, Env *);

node *eval(node *t)
{
	/* Evaluate the tree in the REPL environment */
	if(!t || t -> type != NODE_LIST || !t -> left)
		return NULL;
	return EVAL(t -> left, repl_environment);
}

node *atom(node *t)
{
    if(!t || t -> type != NODE_LIST)
        return NULL;
    
    return newnode(NODE_ATOM, t -> left, NULL);
}

node *atom_query(node *t)
{
    if(!t || t -> type != NODE_LIST || t -> left == NULL || 
        t -> left -> type != NODE_ATOM)
        return newnode(NODE_FALSE, NULL, NULL);
    return newnode(NODE_TRUE, NULL, NULL);
}

node *deref_atom(node *t)
{
    if(!t || t -> type != NODE_LIST || t -> left == NULL || 
        t -> left -> type != NODE_ATOM)
        return NULL;
    return t -> left -> left;
}

node *reset_bang(node *t)
{
    node *a, *v;
    if(!t || t -> type != NODE_LIST || t -> left == NULL || 
        t -> right == NULL || t -> left -> type != NODE_ATOM)
        return NULL;

    a = t -> left;
    if(!t -> right -> left)
        return NULL;
    v = t -> right -> left;
    a -> left = v;
    return v;
}

node *swap_bang(node *t)
{
    node *a = t -> left;
    node *f = t -> right -> left;
    node *args = newnode(NODE_LIST, a -> left, t -> right -> right);
    node *result = EVAL(newnode(NODE_LIST,f,args), repl_environment);

    a -> left = result;
    return result;
}

node *vec2list(node *t)
{
    /* Convert a vector to a list */
    node *l;
    node *tail;

    t = t -> left;
    l = newnode(NODE_LIST, NULL, NULL);
    tail = l;

    while(t && t -> left && t -> left != NULL) {
        node *n = newnode(NODE_LIST, t -> left, NULL);
        tail -> right = n;
        tail = n;
        t = t -> right;
    }
    if(l -> right) return l -> right;
    return l;
}

node *list2vec(node *t)
{
    /* Convert a list to a vector */
    node *l;
    node *tail;

    t = t -> left;
    l = newnode(NODE_VEC, NULL, NULL);
    tail = l;

    while(t && t -> left && t -> left != NULL) {
        node *n = newnode(NODE_VEC, t -> left, NULL);
        tail -> right = n;
        tail = n;
        t = t -> right;
    }
    if(l -> right) return l -> right;
    return l;
}

node *vector(node *t)
{
    printf("in vector t = %s\n", pr_str(t,1));
    return list2vec(newnode(NODE_LIST,t,NULL));
}

node *cons(node *t)
{
    node *head = t -> left;
    node *tail = t -> right -> left;
    if(tail -> left == NULL)
        tail = NULL;
    if(tail && tail -> type == NODE_VEC) {
        /* Consing into a vector, but we need to create a list */
        tail = vec2list(newnode(NODE_VEC,tail,NULL));
    }
    return newnode(NODE_LIST, head, tail);
}

node *concat(node *t) 
{
    /* Concatenate zero or more lists */
    node *list, *tail;

    if(!t || !t -> left)
        return newnode(NODE_LIST, NULL, NULL);

    list = newnode(NODE_LIST, NULL, NULL);
    tail = list;

    while(t && t -> left != NULL) {
        /* Iterate over list parameters */
        node *l = t -> left;
        while(l && l -> left != NULL) {
            node *n = newnode(NODE_LIST, l -> left, NULL);
            tail -> right = n;
            tail = n;
            l = l -> right;
        }
        t = t -> right;
    }
    if(tail == list)
        return list;
    return list -> right;
}

node *nth(node *t)
{
    node *list = t -> left;
    node *n = t -> right -> left;
    int i = n -> value.int_value;

    while(i--) {
        if(!list) {
            break;
        }
        list = list -> right;
    }
    if(!list) {
        /* raise not found error */
        node *exc;
        char *buf;
        buf = GC_MALLOC(20);
        strcpy(buf,"nth out of bounds");
        exc = newnode(NODE_STRING, NULL, NULL);
        exc -> value.string_value = buf;
        throw_exception(buf, exc, 1);
        return NULL;
    }
    return list -> left;
}

node *first(node *t) {
    if(!t || !t -> left || !t -> left -> left)
        return newnode(NODE_NIL, NULL, NULL);
    return t -> left -> left;
}

node *rest(node *t) {
    node *r;
    if(!t || !t -> left || !t -> left -> right)
        return newnode(NODE_LIST, NULL, NULL);
    r = t -> left -> right;
    if(r -> type == NODE_VEC)
        return vec2list(newnode(NODE_LIST,r,NULL));
    return r;
}

node *throw(node *t)
{
    throw_exception("throw", t -> left, 1);
    return NULL;
}

node *nil_query(node *t)
{
    return newnode(t && t -> left && 
                   t -> left -> type == NODE_NIL? 
                   NODE_TRUE: NODE_FALSE, NULL, NULL);
}

node *true_query(node *t)
{
    return newnode(t && t -> left && 
                   t -> left -> type == NODE_TRUE? 
                   NODE_TRUE: NODE_FALSE, NULL, NULL);
}

node *false_query(node *t)
{
    return newnode(t && t -> left && 
                   t -> left -> type == NODE_FALSE? 
                   NODE_TRUE: NODE_FALSE, NULL, NULL);
}

node *symbol_query(node *t)
{
    return newnode(t && t -> left && 
                   t -> left -> type == NODE_SYMBOL? 
                   NODE_TRUE: NODE_FALSE, NULL, NULL);
}

node *keyword_query(node *t)
{
    return newnode(t && t -> left && 
                   t -> left -> type == NODE_KEY? 
                   NODE_TRUE: NODE_FALSE, NULL, NULL);
}

node *vector_query(node *t)
{
    return newnode(t && t -> left && 
                   t -> left -> type == NODE_VEC? 
                   NODE_TRUE: NODE_FALSE, NULL, NULL);
}

node *map_query(node *t)
{
    return newnode(t && t -> left && 
                   t -> left -> type == NODE_HASH? 
                   NODE_TRUE: NODE_FALSE, NULL, NULL);
}

node *apply(node *t)
{
    /* apply a function to a "flattened" list of arguments */
    /* last argument has to be a list */
    /* (apply f a b (c d)) -> (f a b c d) */
    node *tail = NULL;
    node *args = NULL;
    node *fargs = NULL;
    node *a = NULL;
    int last_arg = 0;

    if(!t || !t -> left || !(t -> left -> type == NODE_FUNC || 
                             t -> left -> type == NODE_LAMBDA))
        throw_exception("applying a non-function", t, 1);
    
    /* Run through the given argument list building the function arguments */
    args = t -> right;
    fargs = newnode(NODE_LIST, t -> left, NULL);
    tail = fargs;
    while(args) {
        if(!last_arg && !args -> right) {
            /* last argument, go into it */
            if(args -> left == NULL || (args -> left -> type != NODE_LIST && 
                                        args -> left -> type != NODE_VEC))
                throw_exception("apply last arg not a list", t, 1);
            args = args -> left;
            last_arg = 1;
        }
        a = newnode(NODE_LIST, args -> left, NULL);
        tail -> right = a;
        tail = tail -> right;
        args = args -> right;
    }
    if(fargs -> left -> type == NODE_FUNC) {
		node *(*func)(node *) = NULL;
		func = fargs -> left -> value.func_value;
		return func(fargs -> right);
	}
    if(fargs -> left -> type == NODE_LAMBDA) {
        Env *e = NULL;
        node *f = fargs -> left;
		e = newenv(f -> value.node_env, f -> left, fargs -> right);
        return EVAL(f -> right,e);
	}
    return NULL;
}

node *map(node *t)
{
    node *fargs = NULL;
    node *args;
    node *tail;
    node *result;
    node *fn;

    if(!t || !t -> left || !(t -> left -> type == NODE_FUNC || 
                             t -> left -> type == NODE_LAMBDA))
        throw_exception("mapping a non-function", t, 1);

    if(!t -> right || !t -> right -> left || 
       !(t -> right -> left -> type == NODE_LIST || 
         t -> right -> left -> type == NODE_VEC))
        throw_exception("mapping onto a non-list", t, 1);

    fn = t -> left;
    args = t -> right -> left;
    result = newnode(NODE_LIST, NULL, NULL);
    tail = result;
    while(args && args -> left) {
        node *r;
        fargs = newnode(NODE_LIST, args -> left, NULL);
        if(fn -> type == NODE_FUNC) {
		    node *(*func)(node *) = NULL;
		    func = fn -> value.func_value;
		    r = func(fargs);
	    }
        if(fn -> type == NODE_LAMBDA) {
            Env *e = NULL;
		    e = newenv(fn -> value.node_env, fn -> left, fargs);
            r = EVAL(fn -> right,e);
        }
        tail -> right = newnode(NODE_LIST, r, NULL);
        tail = tail -> right;
        args = args -> right;
    }
    if(result -> right) return result -> right;
    return result;
}

node* symbol(node *t)
{
    node *result;

    if(!t || !t -> left || t -> left -> type != NODE_STRING) {
        throw_exception("cannot make symbol from non-string", t, 1);
    }

    result = newnode(NODE_SYMBOL, NULL, NULL);
    result -> value.string_value = t -> left -> value.string_value;
    return result;
}

node* keyword(node *t)
{
    node *result;

    if(!t || !t -> left || (t -> left -> type != NODE_STRING && 
                            t -> left -> type != NODE_KEY)) {
        throw_exception("cannot make symbol from non-string", t, 1);
    }

    result = newnode(NODE_KEY, NULL, NULL);
    result -> value.string_value = t -> left -> value.string_value;
    return result;
}

node *sequential_query(node *t)
{
    return newnode(t && 
                   t -> left && 
                   (t -> left -> type == NODE_LIST || 
                    t -> left -> type == NODE_VEC)? 
                   NODE_TRUE: NODE_FALSE, NULL, NULL);    
}

node *hash_map(node *t)
{
    /* Convert arguments to a hashmap */
    node *l;
    node *tail;

    l = newnode(NODE_HASH, NULL, NULL);
    tail = l;

    while(t && t -> left && t -> left != NULL) {
        node *n = newnode(NODE_HASH, t -> left, NULL);
        tail -> right = n;
        tail = n;
        t = t -> right;
    }
    if(l -> right) return l -> right;
    return l;
}

node *hash_get(node *t)
{
    node *hm = NULL;
    node *key = NULL;

    if(!t || !t -> right)
        return newnode(NODE_NIL, NULL, NULL);

    hm = t -> left;
    key = t -> right -> left;

    while(hm && hm -> right) {
        if(nodes_equal(key, hm -> left) == NODE_TRUE)
            return hm -> right -> left;
        hm = hm -> right -> right;
    }
    return newnode(NODE_NIL, NULL, NULL);
}

node *hash_contains(node *t)
{
    node *hm = NULL;
    node *key = NULL;

    if(!t || !t -> right)
        return newnode(NODE_FALSE, NULL, NULL);

    hm = t -> left;
    key = t -> right -> left;

    while(hm && hm -> right) {
        if(nodes_equal(key, hm -> left) == NODE_TRUE)
            return newnode(NODE_TRUE, NULL, NULL);
        hm = hm -> right -> right;
    }
    return newnode(NODE_FALSE, NULL, NULL);        
}    

node *list_contains(node *t)
{
    node *l = NULL;
    node *key = NULL;

    if(!t || !t -> right)
        return newnode(NODE_FALSE, NULL, NULL);

    l = t -> left;
    key = t -> right -> left;
    while(l) {
        if(nodes_equal(key, l -> left) == NODE_TRUE)
            return newnode(NODE_TRUE, NULL, NULL);
        l = l -> right;
    }
    return newnode(NODE_FALSE, NULL, NULL);        
}    

node *hash_keys(node *t)
{
    node *hm = NULL;
    node *a = NULL;

    if(!t)
        return newnode(NODE_LIST, NULL, NULL);

    hm = t -> left;
    while(hm && hm -> right) {
        a = newnode(NODE_LIST, hm -> left, a);
        hm = hm -> right -> right;
    }
    if(!a) return newnode(NODE_LIST, NULL, NULL);
    return a;
}

node *hash_vals(node *t)
{
    node *hm = NULL;
    node *a = NULL;

    if(!t)
        return newnode(NODE_LIST, NULL, NULL);

    hm = t -> left;
    while(hm && hm -> right) {
        a = newnode(NODE_LIST, hm -> right -> left, a);
        hm = hm -> right -> right;
    }
    if(!a) return newnode(NODE_LIST, NULL, NULL);
    return a;
}

int hash_equals(node *x, node *y)
{
    /* Compare two hashmaps */
    /* Will be equal if they have the same keys with the same values */
    /* Note, they may occur in a different order */

    node *xkeys = hash_keys(newnode(NODE_LIST,x,NULL)); 
    node *ykeys = hash_keys(newnode(NODE_LIST,y,NULL));

    /* Must have same number of keys */
    if(count_list(newnode(NODE_LIST,xkeys,NULL)) -> value.int_value != 
       count_list(newnode(NODE_LIST,ykeys,NULL)) -> value.int_value)
        return NODE_FALSE;

    /* Loop through x keys and check present and equal in y */
    while(xkeys && xkeys -> left) {
        if(hash_contains(
            newnode(NODE_LIST, y,
                    newnode(NODE_LIST,xkeys -> left, NULL))) -> type 
            == NODE_FALSE
            /* key is not present */
            || 
            nodes_equal(
                hash_get(newnode(NODE_LIST,x,
                                 newnode(NODE_LIST,xkeys -> left, NULL))),
                hash_get(newnode(NODE_LIST,y,
                                 newnode(NODE_LIST,xkeys -> left, NULL)))) 
                == NODE_FALSE
            /* values do not match*/
        )
            return NODE_FALSE;
        xkeys = xkeys -> right;
    }
    return NODE_TRUE;
}

node *hash_assoc(node *t)
{
    /* Add new keys and values to an existing hashmap */
    node *hm = t -> left;
    node *bindings = t -> right;
    node *new = NULL;

    if(hm -> type != NODE_HASH)
        throw_exception("assoc with non-hashmap", hm, 1);
    
    /* Turn the bindings into a hashmap */
    new = hash_map(bindings);

    /* Now add existing hashmap enties if not already present */
    while(hm && hm -> right) 
    {
        if(hash_contains(
            newnode(NODE_LIST,new,
                    newnode(NODE_LIST,hm -> left,NULL))) -> type 
            == NODE_FALSE) {
            new = newnode(NODE_HASH,hm -> right -> left,new);
            new = newnode(NODE_HASH,hm -> left, new);
        }
        hm = hm -> right -> right;
    }
    return new;
}

node *hash_dissoc(node *t)
{
    /* Remove old keys and values from an existing hashmap */
    node *hm = t -> left;
    node *bindings = t -> right;
    node *new = NULL;

    if(hm -> type != NODE_HASH)
        throw_exception("dissoc with non-hashmap", hm, 1);

    /* Now copy existing hashmap enties if not in the dissoc map */
    while(hm && hm -> right) 
    {
        if(list_contains(
                newnode(NODE_LIST,bindings,
                        newnode(NODE_LIST,hm -> left,NULL))) -> type
            == NODE_FALSE) {
            new = newnode(NODE_HASH,hm -> right -> left,new);
            new = newnode(NODE_HASH,hm -> left, new);
        }
        hm = hm -> right -> right;
    }
    if(!new) return newnode(NODE_HASH, NULL, NULL);
    return new;
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
    env_add_func(env, "read-string", read_dash_string);
    env_add_func(env, "slurp", slurp);
    env_add_func(env, "eval", eval);
    env_add_func(env, "atom", atom);
    env_add_func(env, "atom?", atom_query);
    env_add_func(env, "deref", deref_atom);
    env_add_func(env, "reset!", reset_bang);
    env_add_func(env, "swap!", swap_bang);
    env_add_func(env, "cons", cons);
    env_add_func(env, "vec", list2vec);
    env_add_func(env, "concat", concat);
    env_add_func(env, "vec2list", vec2list);
    env_add_func(env, "nth", nth);
    env_add_func(env, "first", first);
    env_add_func(env, "rest", rest);
    env_add_func(env, "throw", throw);
    env_add_func(env, "nil?", nil_query);
    env_add_func(env, "true?", true_query);
    env_add_func(env, "false?", false_query);
    env_add_func(env, "symbol?", symbol_query);
    env_add_func(env, "keyword?", keyword_query);
    env_add_func(env, "apply", apply);
    env_add_func(env, "map", map);
    env_add_func(env, "symbol", symbol);
    env_add_func(env, "keyword", keyword);
    env_add_func(env, "sequential?", sequential_query);
    env_add_func(env, "vector", vector);
    env_add_func(env, "vector?", vector_query);
    env_add_func(env, "map?", map_query);
    env_add_func(env, "hash-map", hash_map); 
    env_add_func(env, "get", hash_get);
    env_add_func(env, "contains?", hash_contains);
    env_add_func(env, "keys", hash_keys);
    env_add_func(env, "vals", hash_vals);
    env_add_func(env, "assoc", hash_assoc);
    env_add_func(env, "dissoc", hash_dissoc);
    return env; 
}
