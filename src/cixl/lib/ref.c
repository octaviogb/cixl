#include <string.h>
#include <inttypes.h>

#include "cixl/arg.h"
#include "cixl/box.h"
#include "cixl/call.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/fimp.h"
#include "cixl/func.h"
#include "cixl/ref.h"
#include "cixl/scope.h"
#include "cixl/lib.h"
#include "cixl/lib/ref.h"

static bool ref_imp(struct cx_call *call) {
  struct cx_box *v = cx_test(cx_call_arg(call, 0));
  struct cx_scope *s = call->scope;
  struct cx_ref *r = cx_ref_new(s->cx, NULL);
  cx_copy(&r->value, v);
  
  cx_box_init(cx_push(s),
	      (v->type == s->cx->nil_type)
	      ? s->cx->ref_type
	      : cx_type_get(s->cx->ref_type, v->type))->as_ref = r;
  
  return true;
}

static bool deref_imp(struct cx_call *call) {
  struct cx_box *r = cx_test(cx_call_arg(call, 0));
  cx_copy(cx_push(call->scope), &r->as_ref->value);
  return true;
}

static bool set_imp(struct cx_call *call) {
  struct cx_box
    *v = cx_test(cx_call_arg(call, 1)),
    *r = cx_test(cx_call_arg(call, 0));

  cx_box_deinit(&r->as_ref->value);
  cx_copy(&r->as_ref->value, v);
  return true;
}

static bool set_call_imp(struct cx_call *call) {
  struct cx_box
    *a = cx_test(cx_call_arg(call, 1)),
    *r = cx_test(cx_call_arg(call, 0));

  struct cx_scope *s = call->scope;
  cx_copy(cx_push(s), &r->as_ref->value);
  if (!cx_call(a, s)) { return false; }
  struct cx_box *v = cx_pop(s, false);

  if (v) {
    cx_box_deinit(&r->as_ref->value);
    r->as_ref->value = *v;
  }

  return true;
}

cx_lib(cx_init_ref, "cx/ref") { 
  struct cx *cx = lib->cx;
    
  if (!cx_use(cx, "cx/abc", "A", "Opt") ||
      !cx_use(cx, "cx/type", "new")) {
    return false;
  }

  cx->ref_type = cx_init_ref_type(lib);

  cx_add_cfunc(lib, "ref",
	       cx_args(cx_arg("val", cx->opt_type)),
	       cx_args(cx_arg(NULL, cx_type_get(cx->ref_type, cx_arg_ref(cx, 0, 0)))),
	       ref_imp);
  
  cx_add_cfunc(lib, "deref",
	       cx_args(cx_arg("ref", cx->ref_type)),
	       cx_args(cx_narg(cx, NULL, 0, 0)),
	       deref_imp);

  cx_add_cfunc(lib, "set",
	       cx_args(cx_arg("ref", cx->ref_type),
		       cx_narg(cx, "val", 0, 0)),
	       cx_args(),
	       set_imp);

  cx_add_cfunc(lib, "set-call",
	       cx_args(cx_arg("ref", cx->ref_type), cx_arg("act", cx->any_type)),
	       cx_args(),
	       set_call_imp);
  
  return true;
}
