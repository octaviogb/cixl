#include <inttypes.h>
#include <math.h>

#include "cixl/arg.h"
#include "cixl/box.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/lib.h"
#include "cixl/scope.h"
#include "cixl/lib/math.h"

static bool inc_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->int_type)->as_int = v->as_int+1;
  return true;
}

static bool dec_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->int_type)->as_int = v->as_int-1;
  return true;
}

static bool int_add_imp(struct cx_call *call) {
  struct cx_box
    *y = cx_test(cx_call_arg(call, 1)),
    *x = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->int_type)->as_int = x->as_int+y->as_int;
  return true;
}

static bool int_sub_imp(struct cx_call *call) {
  struct cx_box
    *y = cx_test(cx_call_arg(call, 1)),
    *x = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->int_type)->as_int = x->as_int-y->as_int;
  return true;
}

static bool int_mul_imp(struct cx_call *call) {
  struct cx_box
    *y = cx_test(cx_call_arg(call, 1)),
    *x = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->int_type)->as_int = x->as_int*y->as_int;
  return true;
}

static bool int_div_imp(struct cx_call *call) {
  struct cx_box
    *y = cx_test(cx_call_arg(call, 1)),
    *x = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;

  if (!y->as_int) {
    cx_error(s->cx, s->cx->row, s->cx->col, "Division by zero");
    return false;
  }

  cx_box_init(cx_push(s), s->cx->int_type)->as_int = x->as_int / y->as_int;
  return true;
}

static bool int_pow2_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->int_type)->as_int = v->as_int*v->as_int;
  return true;
}

static bool int_mod_imp(struct cx_call *call) {
  struct cx_box
    *y = cx_test(cx_call_arg(call, 1)),
    *x = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->int_type)->as_int = x->as_int % y->as_int;
  return true;
}

static bool int_abs_imp(struct cx_call *call) {
  struct cx_box *n = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  
  cx_box_init(cx_push(s), s->cx->int_type)->as_int =
    (n->as_int < 0) ? -n->as_int : n->as_int;
  
  return true;
}

static bool rand_imp(struct cx_call *call) {
  struct cx_box *max = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->int_type)->as_int = cx_rand(max->as_int);
  return true;
}

static bool float_add_imp(struct cx_call *call) {
  struct cx_box
    *y = cx_test(cx_call_arg(call, 1)),
    *x = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->float_type)->as_float = x->as_float+y->as_float;
  return true;
}

static bool float_sub_imp(struct cx_call *call) {
  struct cx_box
    *y = cx_test(cx_call_arg(call, 1)),
    *x = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->float_type)->as_float = x->as_float-y->as_float;
  return true;
}

static bool int_float_sub_imp(struct cx_call *call) {
  struct cx_box
    *y = cx_test(cx_call_arg(call, 1)),
    *x = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->float_type)->as_float = x->as_int-y->as_float;
  return true;
}

static bool float_mul_imp(struct cx_call *call) {
  struct cx_box
    *y = cx_test(cx_call_arg(call, 1)),
    *x = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->float_type)->as_float = x->as_float*y->as_float;
  return true;
}

static bool float_double_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->float_type)->as_float = v->as_float*2;
  return true;
}

static bool float_div_imp(struct cx_call *call) {
  struct cx_box
    *y = cx_test(cx_call_arg(call, 1)),
    *x = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->float_type)->as_float = x->as_float/y->as_float;
  return true;
}

static bool float_int_div_imp(struct cx_call *call) {
  struct cx_box
    *y = cx_test(cx_call_arg(call, 1)),
    *x = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->float_type)->as_float = x->as_float/y->as_int;
  return true;
}

static bool int_float_div_imp(struct cx_call *call) {
  struct cx_box
    *y = cx_test(cx_call_arg(call, 1)),
    *x = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->float_type)->as_float = x->as_int/y->as_float;
  return true;
}

static bool float_pow2_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->float_type)->as_float = v->as_float*v->as_float;
  return true;
}

static bool sqrt_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->float_type)->as_float = sqrtl(v->as_float);
  return true;
}

static bool log_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->float_type)->as_float = logl(v->as_float);
  return true;
}

static bool float_int_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->int_type)->as_int = v->as_float;
  return true;
}

static bool int_float_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  cx_box_init(cx_push(s), s->cx->float_type)->as_float = v->as_int;
  return true;
}

cx_lib(cx_init_math, "cx/math") {
  struct cx *cx = lib->cx;

  if (!cx_use(cx, "cx/abc",
	      "A", "Fimp", "Float", "Func", "Int", "Num", "Opt", "Seq") ||
      !cx_use(cx, "cx/cond", "=", "?", "if-else") ||
      !cx_use(cx, "cx/func", "recall") ||
      !cx_use(cx, "cx/iter", "for")) {
    return false;
  }

  cx_add_cfunc(lib, "++",
	       cx_args(cx_arg("v", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       inc_imp);
  
  cx_add_cfunc(lib, "--",
	       cx_args(cx_arg("v", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       dec_imp);

  cx_add_cfunc(lib, "+",
	       cx_args(cx_arg("x", cx->int_type), cx_arg("y", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       int_add_imp);
  
  cx_add_cfunc(lib, "-",
	       cx_args(cx_arg("x", cx->int_type), cx_arg("y", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       int_sub_imp);
  
  cx_add_cfunc(lib, "*",
	       cx_args(cx_arg("x", cx->int_type), cx_arg("y", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       int_mul_imp);
  
  cx_add_cfunc(lib, "/",
	       cx_args(cx_arg("x", cx->int_type), cx_arg("y", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       int_div_imp);

  cx_add_cfunc(lib, "**",
	       cx_args(cx_arg("v", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       int_pow2_imp);

  cx_add_cfunc(lib, "mod",
	       cx_args(cx_arg("x", cx->int_type), cx_arg("y", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       int_mod_imp);

  cx_add_cfunc(lib, "abs",
	       cx_args(cx_arg("n", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       int_abs_imp);

  cx_add_cfunc(lib, "rand",
	       cx_args(cx_arg("n", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       rand_imp);

  cx_add_cfunc(lib, "+",
	       cx_args(cx_arg("x", cx->float_type), cx_arg("y", cx->float_type)),
	       cx_args(cx_arg(NULL, cx->float_type)),
	       float_add_imp);

  cx_add_cfunc(lib, "-",
	       cx_args(cx_arg("x", cx->float_type), cx_arg("y", cx->float_type)),
	       cx_args(cx_arg(NULL, cx->float_type)),
	       float_sub_imp);

  cx_add_cfunc(lib, "-",
	       cx_args(cx_arg("x", cx->int_type), cx_arg("y", cx->float_type)),
	       cx_args(cx_arg(NULL, cx->float_type)),
	       int_float_sub_imp);

  cx_add_cfunc(lib, "*",
	       cx_args(cx_arg("x", cx->float_type), cx_arg("y", cx->float_type)),
	       cx_args(cx_arg(NULL, cx->float_type)),
	       float_mul_imp);

  cx_add_cfunc(lib, "*2",
	       cx_args(cx_arg("v", cx->float_type)),
	       cx_args(cx_arg(NULL, cx->float_type)),
	       float_double_imp);

  cx_add_cfunc(lib, "/",
	       cx_args(cx_arg("x", cx->float_type), cx_arg("y", cx->float_type)),
	       cx_args(cx_arg(NULL, cx->float_type)),
	       float_div_imp);

  cx_add_cfunc(lib, "/",
	       cx_args(cx_arg("x", cx->float_type), cx_arg("y", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->float_type)),
	       float_int_div_imp);

  cx_add_cfunc(lib, "/",
	       cx_args(cx_arg("x", cx->int_type), cx_arg("y", cx->float_type)),
	       cx_args(cx_arg(NULL, cx->float_type)),
	       int_float_div_imp);

  cx_add_cfunc(lib, "**",
	       cx_args(cx_arg("v", cx->float_type)),
	       cx_args(cx_arg(NULL, cx->float_type)),
	       float_pow2_imp);

  cx_add_cfunc(lib, "sqrt",
	       cx_args(cx_arg("v", cx->float_type)),
	       cx_args(cx_arg(NULL, cx->float_type)),
	       sqrt_imp);

  cx_add_cfunc(lib, "log",
	       cx_args(cx_arg("v", cx->float_type)),
	       cx_args(cx_arg(NULL, cx->float_type)),
	       log_imp);

  cx_add_cfunc(lib, "int",
	       cx_args(cx_arg("v", cx->float_type)),
	       cx_args(cx_arg(NULL, cx->int_type)),
	       float_int_imp);

  cx_add_cfunc(lib, "float",
	       cx_args(cx_arg("v", cx->int_type)),
	       cx_args(cx_arg(NULL, cx->float_type)),
	       int_float_imp);

  cx_add_cxfunc(lib, "fib-rec",
		cx_args(cx_arg("a", cx->int_type),
			cx_arg("b", cx->int_type),
			cx_arg("n", cx->int_type)),
		cx_args(cx_arg(NULL, cx->int_type)),
		"$n?<Opt> {$b $a $b +<Int Int> $n -- recall} $a if-else");

  cx_add_cxfunc(lib, "fib",
		cx_args(cx_arg("n", cx->int_type)),
		cx_args(cx_arg(NULL, cx->int_type)),
		"0 1 $n fib-rec");
    
  cx_add_cxfunc(lib, "sum",
		cx_args(cx_arg("in", cx->seq_type)),
		cx_args(cx_arg(NULL, cx->any_type)),
		"0 $in &+ for");

  return true;
}
