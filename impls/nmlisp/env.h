/* An environment is a hashmap (herein a list of pairs) 
   of NODE_SYMBOLs with nodes following as values */
#include "types.h"

#ifndef __ENV_H__
#define __ENV_H__

Env *newenv(Env *outer, node *binds, node *exprs);
void env_set(Env *env, node *sym, node *val);
Env *env_find(Env *env, node *sym);
node *env_get(Env *env, node *sym);
int is_macro_call(node *, Env *);
extern Env *repl_environment;

#endif