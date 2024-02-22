/* An environment is a hashmap (herein a list of pairs) of NODE_SYMBOLs with nodes following as values */
#include "types.h"

#ifndef __ENV_H__
#define __ENV_H__

typedef struct Env {
    struct Env *outer;
    node *hashmap;
} Env;

Env *newenv(Env *outer);
void env_set(Env *env, node *sym, node *val);
Env *env_find(Env *env, node *sym);
node *env_get(Env *env, node *sym);

#endif