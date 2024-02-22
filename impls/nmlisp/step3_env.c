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

	if(!t) 
		return NULL;

	/* If it is not a list */
	if(t -> type != NODE_LIST)
		return eval_tree(t, env);
	
	/* If it is the empty list */
	if(t -> type == NODE_LIST && t -> left == NULL)
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
					Env *e = newenv(env);
					node *bindings = t -> right -> left;
					node *clause = t -> right -> right -> left;

					while(bindings) {
						node *sym = bindings -> left;
						node *val = EVAL(bindings -> right -> left, e);
						env_set(e, sym, val);
						bindings = bindings -> right -> right;
					}
					
					return EVAL(clause, e);
				}
				break;
		}
	}
	/* Now it is a list so apply first element to the rest of the list */
	e = eval_tree(t, env);
	f = e -> left;
	a = eval_apply(f, e -> right, env);
	if(a) return a;
	return e;
}

/* Apply a function to a list of (evaluated as much as needed) arguments */
node *eval_apply(node *f, node *args, Env *env) {
	node *(*func)(node *) = NULL;

	/* Is it an internal function */
	if(f && f -> type == NODE_FUNC) {
		func = f -> value.func_value;
		return func(args);
	}

	return NULL;
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

int main()
{
	static char *line_read = (char *)NULL;
	char *result;
	Env *env = new_repl_env();

	exception_init();
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

