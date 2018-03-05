#include <stdlib.h>
#include <string.h>

#include "cixl/env.h"
#include "cixl/malloc.h"

struct cx_env *cx_env_init(struct cx_env *env, struct cx_malloc *alloc) {
  env->alloc = alloc;
  for (unsigned int i = 0; i < CX_ENV_SLOTS; i++) { env->slots[i] = NULL; }
  env->count = 0;
  return env;
}

struct cx_env *cx_env_deinit(struct cx_env *env) {
  for (unsigned int i = 0; i < CX_ENV_SLOTS; i++) {
    for (struct cx_var *v = env->slots[i]; v;) {
      struct cx_var *nv = v->next;
      cx_box_deinit(&v->value);
      cx_free(env->alloc, v);
      v = nv;
    }
  }
  
  return env;
}

struct cx_var *cx_env_get(struct cx_env *env, struct cx_sym id) {
  for (struct cx_var *v = env->slots[id.tag % CX_ENV_SLOTS]; v; v = v->next) {
    if (v->id.tag == id.tag) { return v; }
  }
  
  return NULL;
}

struct cx_box *cx_env_put(struct cx_env *env, struct cx_sym id) {
  struct cx_var **slot = env->slots + id.tag % CX_ENV_SLOTS;
  struct cx_var *var = cx_malloc(env->alloc);
  env->count++;
  var->id = id;
  var->next = NULL;
  var->next = *slot;
  *slot = var;
  return &var->value;
}
