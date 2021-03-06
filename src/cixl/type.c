#include <stdlib.h>
#include <string.h>

#include "cixl/cx.h"
#include "cixl/box.h"
#include "cixl/emit.h"
#include "cixl/error.h"
#include "cixl/mfile.h"
#include "cixl/scope.h"
#include "cixl/type.h"

struct cx_type *cx_type_new(struct cx_lib *lib, const char *id) {
  return cx_type_init(malloc(sizeof(struct cx_type)), lib, id);
}

struct cx_type *cx_type_init(struct cx_type *type,
			     struct cx_lib *lib,
			     const char *id) {
  type->lib = lib;
  type->meta = CX_TYPE_IMP;
  type->id = strdup(id);
  type->emit_id = cx_emit_id("type", id);
  type->tag = lib->cx->next_type_tag++;
  type->level = 0;
  type->raw = type;

  cx_set_init(&type->parents, sizeof(struct cx_type *), cx_cmp_ptr);
  cx_set_init(&type->children, sizeof(struct cx_type *), cx_cmp_ptr);

  cx_vec_init(&type->is, sizeof(struct cx_type *));
  *(struct cx_type **)cx_vec_put(&type->is, type->tag) = type;

  cx_vec_init(&type->args, sizeof(struct cx_type *));
  
  type->new = NULL;
  type->eqval = NULL;
  type->equid = NULL;
  type->cmp = NULL;
  type->ok = NULL;
  type->call = NULL;
  type->copy = NULL;
  type->clone = NULL;
  type->iter = NULL;
  type->sink = NULL;
  type->write = NULL;
  type->dump = NULL;
  type->print = NULL;
  type->emit = NULL;
  type->deinit = NULL;

  type->type_new = NULL;
  type->type_init = NULL;
  type->type_deinit = NULL;
  return type;
}

struct cx_type *cx_type_reinit(struct cx_type *type) {
  type->level = 0;
  
  for (size_t i=0; i < type->is.count; i++) {
    if (i != type->tag) { *(struct cx_type **)cx_vec_get(&type->is, i) = NULL; }
  }

  cx_vec_clear(&type->args);
  
  cx_do_set(&type->parents, struct cx_type *, t) {
    cx_set_delete(&(*t)->children, t);
  }
  
  cx_set_clear(&type->parents);

  cx_do_set(&type->children, struct cx_type *, t) {
    cx_set_delete(&(*t)->parents, &type);
    (*t)->level = 0;
    
    cx_do_set(&(*t)->parents, struct cx_type *, pt) {
      (*t)->level = cx_max((*t)->level, (*pt)->level+1);
    }
    
    *(struct cx_type **)cx_vec_put(&(*t)->is, type->tag) = NULL;
  }
  
  cx_set_clear(&type->children);
  return type;
}

void *cx_type_deinit(struct cx_type *type) {
  void *ptr = type;
  if (type->type_deinit) { ptr = type->type_deinit(type); }  
  cx_set_deinit(&type->parents);
  cx_set_deinit(&type->children);
  cx_vec_deinit(&type->is);
  cx_vec_deinit(&type->args);
  free(type->id);
  free(type->emit_id);
  return ptr;  
}

void cx_type_vpush_args(struct cx_type *t, int nargs, struct cx_type *args[]) {
  for (int i=0; i < nargs; i++) {
    *(struct cx_type **)cx_vec_push(&t->args) = args[i];
  }
}

struct cx_type *cx_type_vget(struct cx_type *t, int nargs, struct cx_type *args[]) {
  struct cx *cx = t->lib->cx;
    
  if (!t->args.count) {
    cx_error(cx, cx->row, cx->col, "Type does not take args: %s", t->id);
    return NULL;
  }

  if (nargs != t->args.count) {
	cx_error(cx, cx->row, cx->col,
		 "Wrong nr of args for type %s: %d (%d)",
		 t->id, nargs, t->args.count);
	
	return NULL;
  }

  struct cx_type **ie = cx_vec_end(&t->args);
  bool is_identical = true;
  
  for (struct cx_type
	 **i = cx_vec_start(&t->args),
	 **j = args;
       i != ie;
       i++, j++) {
    if (*j && *i != *j) {
      is_identical = false;
    
      if ((*j)->meta != CX_TYPE_ARG && !cx_is(*j, *i)) {
	cx_error(cx, cx->row, cx->col,
		 "Expected type arg %s, actual: %s", (*i)->id, (*j)->id);
	
	return NULL;
      }
    }
  }

  if (is_identical) { return t; }

  struct cx_mfile id;
  cx_mfile_open(&id);
  fputs(t->raw->id, id.stream);
  fputc('<', id.stream);
  char sep = 0;
  
  for (int i=0; i < nargs; i++) {
    if (sep) { fputc(sep, id.stream); }
    fputs(args[i]->id, id.stream);
    sep = ' ';
  }
  
  fputc('>', id.stream);
  cx_mfile_close(&id);
  struct cx_type *tt = cx_lib_get_type(t->lib, id.data, true);

  if (tt) {
    free(id.data);
    return tt;
  }

  tt = t->type_new
    ? t->type_new(t, id.data, nargs, args)
    : cx_type_new(t->lib, id.data);

  free(id.data);
  tt->meta = t->meta;
  tt->raw = t->raw;  
  cx_type_copy(tt, t);
  tt->type_new = t->type_new;
  tt->type_init = t->type_init;
  tt->type_deinit = t->type_deinit;

  cx_derive(tt, t);
  cx_type_vpush_args(tt, nargs, args);

  if (t->type_init && !t->type_init(tt, nargs, args)) {
    cx_type_deinit(tt);
    return NULL;
  }

  cx_lib_push_type(t->lib, tt);
  return tt;
}

void cx_type_copy(struct cx_type *dst, struct cx_type *src) {
  dst->write = src->write;
  dst->dump = src->dump;
  dst->print = src->print;
  dst->emit = src->emit;
  dst->deinit = src->deinit;
  dst->eqval = src->eqval;
  dst->equid = src->equid;
  dst->cmp = src->cmp;
  dst->ok = src->ok;
  dst->call = src->call;
  dst->new = src->new;
  dst->iter = src->iter;
  dst->sink = src->sink;
  dst->copy = src->copy;
  dst->clone = src->clone;
}

static void derive(struct cx_type *child, struct cx_type *parent) {
  if (parent->tag < child->is.count &&
      *(struct cx_type **)cx_vec_get(&child->is, parent->tag)) {
    return;
  }

  *(struct cx_type **)cx_vec_put(&child->is, parent->tag) = parent;
  child->level = cx_max(child->level, parent->level+1);
  
  cx_do_set(&parent->parents, struct cx_type *, t) {
    if (*t != child && *t != parent) { derive(child, *t); }
  }
  
  cx_do_set(&child->children, struct cx_type *, t) {
    if (*t != child && *t != parent) { derive(*t, parent); }
  }
}

void cx_derive(struct cx_type *child, struct cx_type *parent) {
  struct cx_type **tp = cx_set_insert(&child->parents, &parent);
  if (tp) { *tp = parent; }
  
  tp = cx_set_insert(&parent->children, &child);
  if (tp) { *tp = child; }

  derive(child, parent);
}

bool cx_is(struct cx_type *child, struct cx_type *parent) {
  if (child == parent) { return true; }
  if (parent->raw->tag >= child->is.count) { return false; }
  struct cx_type **ce = cx_vec_end(&child->is);

  for (struct cx_type **c = cx_vec_get(&child->is, parent->raw->tag);
       c != ce;
       c++) {
    if (*c && (*c)->raw == parent->raw) {
      if (*c == parent) { return true; }

      struct cx_type
	**ie = cx_vec_end(&(*c)->args),
	**je = cx_vec_end(&parent->args);

      bool ok = true;

      for (struct cx_type
	     **i = cx_vec_start(&(*c)->args),
	     **j = cx_vec_start(&parent->args);
	   i != ie && j != je;
	   i++, j++) {
	if (!cx_is(*i, *j)) {
	  ok = false;
	  break;
	}
      }

      if (ok) { return true; }
    }
  }

  return false;
}

struct cx_type *cx_supertype(struct cx_type *t, struct cx_type *pt) {
  size_t i = cx_min(cx_min(t->is.count, pt->is.count)-1, t->tag);
  
  for (struct cx_type
	 **t_is = cx_vec_get(&t->is, i),
	 **pt_is = cx_vec_get(&pt->is, i);
       t_is >= (struct cx_type **)cx_vec_start(&t->is);
       t_is--, pt_is--) {
    if (*t_is && *pt_is) { return *t_is; }
  }

  return NULL;
}

struct cx_type *cx_subtype(struct cx_type *t, struct cx_type *pt) {
  if (t == pt) { return t; }
  if (pt->raw->tag >= t->is.count) { return NULL; }
  struct cx_type **cs = cx_vec_start(&t->is);

  for (struct cx_type **c = cx_vec_peek(&t->is, 0);
       c != cs;
       c--) {
    if (*c && (*c)->raw == pt->raw) { return *c; }
  }

  return NULL;
}

struct cx_type *cx_type_arg(struct cx_type *t, int i) {
  struct cx *cx = t->lib->cx;
  
  if (i >= t->args.count) {
    cx_error(cx, cx->row, cx->col, "Type arg out of bounds for type %s: %d",
	     t->id, i);
  }

  return *(struct cx_type **)cx_vec_get(&t->args, i);
}

bool cx_type_has_refs(struct cx_type *t) {
  if (t->meta == CX_TYPE_ARG) { return true; }

  cx_do_vec(&t->args, struct cx_type *, at) {
    if (*at != t && cx_type_has_refs(*at)) { return true; }
  }

  return false;
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_ptr == y->as_ptr;
}

static void dump_imp(struct cx_box *value, FILE *out) {
  struct cx_type *type = value->as_ptr;
  fputs(type->id, out);
}

static bool emit_imp(struct cx_box *v, const char *exp, FILE *out) {
  struct cx_type *t = v->as_ptr;
  struct cx *cx = t->lib->cx;
  struct cx_sym t_var = cx_gsym(cx, "t"), v_var = cx_gsym(cx, "v");
  
  fprintf(out,
	  "struct cx_type *%s = cx_get_type(cx, \"%s\", false);\n"
	  "struct cx_box *%s = %s;\n"
	  "cx_box_init(%s, cx_type_get(cx->meta_type, %s))->as_ptr = %s;\n",
	  t_var.id, t->id, v_var.id, exp, v_var.id, t_var.id, t_var.id);
  
  return true;
}

struct cx_type *cx_init_meta_type(struct cx_lib *lib) {
  struct cx *cx = lib->cx;
  struct cx_type *t = cx_add_type(lib, "Type", cx->any_type);
  cx_type_push_args(t, cx->opt_type);

  t->equid = equid_imp;
  t->write = dump_imp;
  t->dump = dump_imp;
  t->emit = emit_imp;
  return t;
}
