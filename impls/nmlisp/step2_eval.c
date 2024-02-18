#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "types.h"
#include "reader.h"
#include "printer.h"
#include "functions.h"

node *READ(char *s)
{
	return read_str(s);
}

node *EVAL(node *, node *);
node *eval_apply(node *, node *, node *);

node *eval_tree(node *tree, node *env) {
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
			return env_lookup(tree -> value.string_value, env);
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

/* Main Evaluation of a tree in  an environment */
node *EVAL(node *t, node *env)
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

	/* Now it is a list so apply first element to the rest of the list */
	e = eval_tree(t, env);
	f = e -> left;
	a = eval_apply(f, e -> right, env);
	if(a) return a;
	return e;
}

/* Apply a function to a list of arguments */
node *eval_apply(node *f, node *args, node *env) {
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

char *rep(char *s, node *env)
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
	node *env = repl_env();

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
			result = rep(line_read, env);
			printf("%s\n",result);
		}	
		else {
			printf("\n");
			break;
		}
	}
	return 0;
}

