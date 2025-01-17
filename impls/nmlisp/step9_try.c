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

node *quasiquote(node *tree, int in_a_list, int from_vector)
{
	/* The horror that is quasiquote and friends */
	/* Tree is a list that we know has a first element of type NODE_QUOTE */
	/* The second elemnent is a form to be processed */
	node *ast = tree;

	if(!ast)
		return newnode(NODE_LIST, NULL, NULL);
	/*
	printf("in quasiquote, ast type = %d\n", ast -> type);
	printnode(tree);
	printf("\n");
	*/
	/* Deal with vectors */
	if(tree && tree -> type == NODE_VEC)
	{
		node *v = newnode(NODE_LIST, NULL, NULL);
		v -> left = newnode(NODE_SYMBOL, NULL, NULL);
		v -> left -> value.string_value = "vec";
		/* v -> right = newnode(NODE_LIST, NULL, NULL); */
		v -> right = 
		  newnode(NODE_LIST, 
				  quasiquote(
					vec2list(newnode(NODE_VEC,tree,NULL)),in_a_list,1),
				  NULL);
		return v;
	}

	/*  need to deal with unquote out of position
		 - only counts if at the first of a list,
		 - but recursing along a list ruins that! */

	if(ast && ast -> type == NODE_LIST && ast -> left && 
	   ast -> left -> type == NODE_UQUOTE && !in_a_list && !from_vector)
		return ast -> right -> left;
	
	/* Recurse */
	if(ast -> type == NODE_LIST) {
		node *elt;
		if(ast -> left == NULL)
			return ast;
		elt = ast -> left;
		if(elt && elt -> type == NODE_LIST && elt -> left && 
		   elt -> left -> type == NODE_SUQUOTE) {
			node *res = newnode(NODE_LIST, NULL, NULL);
			res -> left = newnode(NODE_SYMBOL, NULL, NULL);
			res -> left -> value.string_value = "concat";
			res -> right = 
			    newnode(NODE_LIST, elt -> right -> left, 
				        newnode(NODE_LIST, 
						        quasiquote(ast -> right,1,0), NULL));
			return res;
		}
		else {
			node *res = newnode(NODE_LIST, NULL, NULL);
			res -> left = newnode(NODE_SYMBOL, NULL, NULL);
			res -> left -> value.string_value = "cons";
			res -> right = newnode(NODE_LIST, quasiquote(elt,0,0), NULL);
			res -> right -> right = 
			    newnode(NODE_LIST,quasiquote(ast -> right,1,0), NULL);
			return res;
		}
	}

	if(ast && (ast -> type == NODE_SYMBOL || ast -> type == NODE_HASH)) {
		node *res = newnode(NODE_LIST, NULL, NULL);
		res -> left = newnode(NODE_QUOTE, NULL, NULL);
		res -> right = newnode(NODE_LIST, ast, NULL);
		return res;
	}
	return ast;
}

/* Deal with macro expansion */
node *macroexpand(node *tree, Env *env)
{
	while(is_macro_call(tree, env)) {
		node *sym = tree -> left;
		node *fn = env_get(env, sym);
		tree = EVAL(newnode(NODE_LIST,fn,tree -> right), env);
	}
	return tree;
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
	if((t -> type == NODE_LIST || t -> type == NODE_VEC) && 
	    t -> left == NULL)
		return t;

	/* Perform macro expansion */
	t = macroexpand(t, env);
	if(t -> type != NODE_LIST)
		return eval_tree(t, env);

	/* Check for special forms */
	if(t -> type == NODE_LIST && t ->left && 
	   t -> left -> type >= NODE_SPECIAL_START) {
		switch(t -> left -> type) {
			case NODE_QUOTE:
				return t -> right -> left;
				break;
			case NODE_QQUOTE:
				return EVAL(quasiquote(t -> right -> left,0,0), env);
				break;
			case NODE_QQE:
				return quasiquote(t -> right -> left,0,0);
				break;
			case NODE_DEFBANG:
				if(t -> right && t -> right -> left && 
				   t -> right -> right && t -> right -> right -> left) {
					a = EVAL(t -> right -> right -> left, env);
					env_set(env, t -> right -> left, a);
				}
				return a;
			case NODE_DEFMACRO:
				if(t -> right && t -> right -> left && 
				   t -> right -> right && 
				   t -> right -> right -> left) {
					a = EVAL(t -> right -> right -> left, env);
					if(a && a -> type == NODE_LAMBDA)
						a -> type = NODE_MACRO;
					env_set(env, t -> right -> left, a);
				}
				return a;
				break;
			case NODE_MACROEXPAND:
				if(t -> right && t -> right -> left)
					return macroexpand(t -> right -> left, env);
				return NULL;
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

					return a? a -> left: NULL;
				}
				break;
			case NODE_IF:
				{
					node *cond = EVAL(t -> right -> left, env);
					if(!cond || cond -> type == NODE_NIL || 
					            cond -> type == NODE_FALSE) {
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
					c -> right = t -> right -> right -> left; /* body */
					c -> value.node_env = env; /* environment */
					return c;
				}
				break;
			case NODE_TRY:
				{
					node *clause = NULL;
					node *catcher = NULL;
					Env *e = NULL;

					if(!t || !t -> right || !t -> right -> left) {
						/* try with no clause */
						return NULL;
					}
					clause = t -> right -> left;
					if(!t -> right -> right) {
						/* try with no catcher  */
						return EVAL(clause, env);
					}
					catcher = t -> right -> right -> left;
					if(catcher -> left -> type == NODE_CATCH) {
						node *sym = catcher -> right -> left;
						node *except = catcher -> right -> right -> left;
						node *r = NULL;
						try
							r = EVAL(clause, env);
						catch(err)
							e = newenv(env, 
								       newnode(NODE_LIST, sym, NULL), 
									   newnode(NODE_LIST, err.value, NULL));
							r = EVAL(except,e);
						end_try_catch
						return r;
					}
				}
			break;
			case NODE_CATCH:
				{
					/* Should not occur outside a try */
					return NULL;
				}
		}
	}
	/* Now it is a list so apply first element to the rest of the list */
	if(t && t -> left && t -> left -> type != NODE_MACRO) {
		e = eval_tree(t, env);
		f = e -> left;
	}
	else {
		e = t;
		f = e -> left;
	}
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
		if(f && (f -> type == NODE_LAMBDA || f -> type == NODE_MACRO)) {
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
	rep("(def! load-file \
	     (fn* (f) \
		 (eval (read-string (str \"(do \" (slurp f) \"\nnil)\")))))", env);
	rep("(defmacro! cond \
	     (fn* (& xs) (if (> (count xs) 0) \
		             (list 'if (first xs) \
					        (if (> (count xs) 1) (nth xs 1) \
							(throw \"odd number of forms to cond\")) \
					 (cons 'cond (rest (rest xs)))))))", env);
}

node *make_argv(int argc, char *argv[])
{
	node *args = NULL;
	int i = argc - 1;

	while(i > 0) {
		node *n = newnode(NODE_LIST, NULL, args);
		args = n;
		args -> left = newnode(NODE_STRING, NULL, NULL);
		args -> left -> value.string_value = argv[i];
		i--;
	}
	if(!args) return newnode(NODE_LIST, NULL, NULL);
	return args;
}

int main(int argc, char *argv[])
{
	static char *line_read = (char *)NULL;
	char *result;
	Env *env = repl_environment = new_repl_env();

	exception_init();
	initialise(env);
	env_set(env, newsymbol("*ARGV*"), make_argv(argc, argv));
	
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
				printf("Exception %d: %s %s\n", err.code, err.msg, 
					                            pr_str(err.value, 1));
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
