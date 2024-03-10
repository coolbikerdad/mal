#include "types.h"
#include "env.h"

node *repl_env();
node *env_lookup(char *name, node *env);
Env *new_repl_env();
node *vec2list(node *);
