#include "cixl/arg.h"
#include "cixl/bool.h"
#include "cixl/call.h"
#include "cixl/char.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/int.h"
#include "cixl/iter.h"
#include "cixl/lambda.h"
#include "cixl/lib.h"
#include "cixl/nil.h"
#include "cixl/lib/abc.h"
#include "cixl/opt.h"
#include "cixl/scope.h"
#include "cixl/stack.h"
#include "cixl/str.h"
#include "cixl/sym.h"

static bool is_nil_imp(struct cx_call *call) {
  struct cx_scope *s = call->scope;
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  cx_box_init(cx_push(s), s->cx->bool_type)->as_bool = v->type == s->cx->nil_type;
  return true;
}

static bool push_imp(struct cx_call *call) {
  struct cx_box
    *s = cx_test(cx_call_arg(call, 0)),
    *v = cx_test(cx_call_arg(call, 1));

  return cx_sink(s, v);
}

cx_lib(cx_init_abc, "cx/abc") { 
  struct cx *cx = lib->cx;

  cx->opt_type = cx_init_opt_type(lib);

  cx->nil_type = cx_init_nil_type(lib);
  cx_box_init(cx_put_const(lib, cx_sym(cx, "nil"), false), cx->nil_type);

  cx->any_type = cx_add_type(lib, "A", cx->opt_type);
  cx->any_type->meta = CX_TYPE_ID;

  cx->cmp_type = cx_add_type(lib, "Cmp", cx->any_type);
  cx->cmp_type->meta = CX_TYPE_ID;

  cx->seq_type = cx_add_type(lib, "Seq", cx->any_type);
  cx_type_push_args(cx->seq_type, cx->opt_type);
  cx->seq_type->meta = CX_TYPE_ID;

  cx->sink_type = cx_add_type(lib, "Sink", cx->any_type);
  cx_type_push_args(cx->sink_type, cx->opt_type);
  cx->sink_type->meta = CX_TYPE_ID;

  cx->num_type = cx_add_type(lib, "Num", cx->cmp_type);
  cx->num_type->meta = CX_TYPE_ID;

  cx->lib_type = cx_init_lib_type(lib);
  cx->meta_type = cx_init_meta_type(lib);

  cx->bool_type = cx_init_bool_type(lib);
  cx_box_init(cx_put_const(lib, cx_sym(cx, "t"), false),
	      cx->bool_type)->as_bool = true;
  cx_box_init(cx_put_const(lib, cx_sym(cx, "f"), false),
	      cx->bool_type)->as_bool = false;

  cx->int_type = cx_init_int_type(lib);
  cx_box_init(cx_put_const(lib, cx_sym(cx, "min-int"), false),
	      cx->int_type)->as_int = INT64_MIN;
  cx_box_init(cx_put_const(lib, cx_sym(cx, "max-int"), false),
	      cx->int_type)->as_int = INT64_MAX;

  cx->float_type = cx_init_float_type(lib);

  cx->char_type = cx_init_char_type(lib);
  cx->str_type = cx_init_str_type(lib);
  cx->sym_type = cx_init_sym_type(lib);
  cx->func_type = cx_init_func_type(lib);
  cx->fimp_type = cx_init_fimp_type(lib);
  cx->lambda_type = cx_init_lambda_type(lib);
  
  cx->iter_type = cx_init_iter_type(lib);

  cx->stack_type = cx_init_stack_type(lib);

  cx_add_cfunc(lib, "is-nil",
	       cx_args(cx_arg("v", cx->opt_type)), cx_args(),
	       is_nil_imp);

  cx_add_cfunc(lib, "push",
	       cx_args(cx_arg("s", cx->sink_type),
		       cx_narg(cx, "v", 0, 0)),
	       cx_args(),
	       push_imp);

  return true;
}
