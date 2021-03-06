#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "cixl/arg.h"
#include "cixl/cx.h"
#include "cixl/emit.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/str.h"
#include "cixl/sym.h"

struct cx_sym *cx_sym_init(struct cx_sym *sym, const char *id, size_t tag) {
  sym->id = strdup(id);
  sym->emit_id = cx_emit_id("sym", id);
  sym->tag = tag;
  return sym;
}

struct cx_sym *cx_sym_deinit(struct cx_sym *sym) {
  free(sym->id);
  free(sym->emit_id);
  return sym;
}

enum cx_cmp cx_cmp_sym(const void *x, const void *y) {
  const struct cx_sym *xs = x, *ys = y;
  return cx_cmp_int(&xs->tag, &ys->tag);
}

static void new_imp(struct cx_box *out) {
  out->as_sym = cx_gsym(out->type->lib->cx, "s");
}

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_sym.tag == y->as_sym.tag;
}

static enum cx_cmp cmp_imp(const struct cx_box *x, const struct cx_box *y) {
  return cx_cmp_sym(&x->as_sym, &y->as_sym);
}

static void dump_imp(struct cx_box *v, FILE *out) {
  fprintf(out, "`%s", v->as_sym.id);
}

static void print_imp(struct cx_box *v, FILE *out) {
  fputs(v->as_sym.id, out);
}

static bool emit_imp(struct cx_box *v, const char *exp, FILE *out) {
  fprintf(out,
	  "cx_box_init(%s, cx->sym_type)->as_sym = %s;\n",
	  exp, v->as_sym.emit_id);
  return true;
}

struct cx_type *cx_init_sym_type(struct cx_lib *lib) {
  struct cx_type *t = cx_add_type(lib, "Sym", lib->cx->cmp_type);
  t->new = new_imp;
  t->equid = equid_imp;
  t->cmp = cmp_imp;
  t->write = dump_imp;
  t->dump = dump_imp;
  t->print = print_imp;
  t->emit = emit_imp;
  return t;
}
