#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "types.h"
#include "reader.h"
#include "printer.h"
#include "functions.h"
#include "env.h"
#include "exceptions.h"

node *READ(char *s)
{
	return read_str(s);
}

node *EVAL(node *, Env *);
node *eval_apply(node *, node *, Env *);

node *eval_tree(node *tree, Env *env) {
	node *answer;
	node *t = tree;
	node *tail = NULL;

	/* Propagate NULL values upwards */
	if(!tree) return tree;

	switch(tree -> type) {

		/* Keywords  and functions do not evaluate */
		case NODE_KEY:
		case NODE_FUNC:
			return tree;
			break;

		/* Symbols evaluate to their values in the environment */
		case NODE_SYMBOL:
			return env_get(env, tree);
			break;
		
		/* Lists, vectors and hashmaps get EVALed item by item */
		case NODE_LIST:
		case NODE_VEC:
		case NODE_HASH:
			answer = newnode(tree -> type, EVAL(t -> left, env), NULL);
			tail = answer;
			t = t -> right;
			while(t) {
				node *n = EVAL(t -> left, env);
				node *l = newnode(tree -> type, n, NULL);
				tail -> right = l;
				tail = l;
				t  = t -> right;
			}
			return answer;
			break;

		/* All else just pass through */
		default:
			return tree;
	}
}

/* Main Evaluation of a tree in an environment */
node *EVAL(node *t, Env *env)
{
  node *e = NULL, *f = NULL, *a = NULL;
  tco: /* For tail call optimisation */
	if(!t) 
		return NULL;

	/* If it is not a list */
	if(t -> type != NODE_LIST)
		return eval_tree(t, env);
	
	/* If it is the empty list or vector */
	if((t -> type == NODE_LIST || t -> type == NODE_VEC) && t -> left == NULL)
		return t;

	/* Check for special forms */
	if(t -> type == NODE_LIST && t ->left && t -> left -> type >= NODE_SPECIAL_START) {
		switch(t -> left -> type) {
			case NODE_QUOTE:
				return t -> right -> left;
				break;
			case NODE_DEFBANG:
				if(t -> right && t -> right -> left && t -> right -> right && t -> right -> right -> left) {
					a = EVAL(t -> right -> right -> left, env);
					env_set(env, t -> right -> left, a);
				}
				return a;
				break;
			case NODE_LETSTAR:
				{
					Env *e = newenv(env, NULL, NULL);
					node *bindings = t -> right -> left;
					node *clause = t -> right -> right -> left;

					while(bindings) {
						node *sym = bindings -> left;
						node *val = EVAL(bindings -> right -> left, e);
						env_set(e, sym, val);
						bindings = bindings -> right -> right;
					}
					env = e; t = clause; /* TCO */
					goto tco;
				}
				break;
			case NODE_DO:
				{
					node *clauses =  t -> right;
					node *a = eval_tree(clauses,env);

					while(a && a -> right)
						a = a -> right;

					return a -> left;
				}
				break;
			case NODE_IF:
				{
					node *cond = EVAL(t -> right -> left, env);
					if(!cond || cond -> type == NODE_NIL || cond -> type == NODE_FALSE) {
						if(!t -> right -> right -> right)
							return newnode(NODE_NIL,NULL,NULL);
						t = t -> right -> right -> right -> left; /* TCO */
						goto tco;
					}
					t = t -> right -> right -> left; /* TCO */
					goto tco;
				}
				break;
			case NODE_FNSTAR:
				{
					/* Return a function closure */	
					node *c = newnode(NODE_LAMBDA, NULL, NULL);
					c -> left = t -> right -> left; /* parameter list */
					c -> right = t -> right -> right -> left; /* function body */
					c -> value.node_env = env; /* environment */
					return c;
				}
				break;
		}
	}
	/* Now it is a list so apply first element to the rest of the list */
	e = eval_tree(t, env);
	f = e -> left;

	/* Apply a function */
	/* a = eval_apply(f, e -> right, env); */
	{
		node *args = e -> right;
		node *(*func)(node *) = NULL;

		/* Is it an internal function */
		if(f && f -> type == NODE_FUNC) {
			func = f -> value.func_value;
			return func(args);
		}

		/* Is it a lambda closure */
		if(f && f -> type == NODE_LAMBDA) {
			Env *e = newenv(f -> value.node_env, f -> left, args);
			env = e;
			t = f -> right;
			goto tco;
		}
	}
	if(a) return a;
	return e;
}

char *PRINT(node *t)
{
	return pr_str(t,1);
}

char *rep(char *s, Env *env)
{
	node *t;
	char *o;
	t = READ(s);
	t = EVAL(t,env);
	o = PRINT(t);
	return o;
}

void initialise(Env *env)
{
	rep("(def! not (fn* (a) (if a false true)))", env);
	rep("(def! fact (fn* (n) (if (< n 1) 1 (* n (fact (- n 1))))))", env);
}

int main()
{
	static char *line_read = (char *)NULL;
	char *result;
	Env *env = new_repl_env();

	exception_init();
	initialise(env);

	while(1) {
	  	/* If the buffer has already been allocated,
     			return the memory to the free pool. */
  		if(line_read) {
			free(line_read);
      			line_read = (char *)NULL;
    		}
		line_read = readline("user> ");
		/* If the line has any text in it,
     			save it on the history. */
  		if(line_read && *line_read)
    			add_history(line_read);
		if(line_read) {
			try
				result = rep(line_read, env);
				printf("%s\n",result);
			catch(err)
				printf("Exception %d: %s\n", err.code, err.msg);
			end_try_catch
		}	
		else {
			printf("\n");
			break;
		}
	}
	exception_end();
	return 0;
}

