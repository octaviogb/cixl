#include <ctype.h>

#include "cixl/box.h"
#include "cixl/char.h"
#include "cixl/cx.h"
#include "cixl/emit.h"
#include "cixl/error.h"
#include "cixl/scope.h"

static bool equid_imp(struct cx_box *x, struct cx_box *y) {
  return x->as_char == y->as_char;
}

static enum cx_cmp cmp_imp(const struct cx_box *x, const struct cx_box *y) {
  return cx_cmp_char(&x->as_char, &y->as_char);
}

static bool ok_imp(struct cx_box *v) {
  return v->as_char;
}

static void dump_imp(struct cx_box *v, FILE *out) {
  unsigned char c = v->as_char;
  
  switch (c) {
  case '\n':
    fputs("@@n", out);
    break;
  case '\r':
    fputs("@@r", out);
    break;
  case ' ':
    fputs("@@s", out);
    break;
  case '\t':
    fputs("@@t", out);
    break;
  default:
    if (isgraph(c)) {
      fprintf(out, "@%c", v->as_char);
    } else {
      fprintf(out, "@@%03d", v->as_char);
    }
  }
}

static void print_imp(struct cx_box *v, FILE *out) {
  fputc(v->as_char, out);
}

static bool emit_imp(struct cx_box *v, const char *exp, FILE *out) {
  fprintf(out,
	  "cx_box_init(%s, cx->char_type)->as_char = %d;\n",
	  exp, v->as_char);
  
  return true;
}

struct cx_type *cx_init_char_type(struct cx_lib *lib) {
  struct cx_type *t = cx_add_type(lib, "Char", lib->cx->cmp_type);
  t->equid = equid_imp;
  t->cmp = cmp_imp;
  t->ok = ok_imp;
  t->write = dump_imp;
  t->dump = dump_imp; 
  t->print = print_imp;
  t->emit = emit_imp;
  return t;
}
