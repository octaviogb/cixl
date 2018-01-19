#include "cixl/box.h"
#include "cixl/cx.h"
#include "cixl/error.h"
#include "cixl/scope.h"
#include "cixl/libs/type.h"
#include "cixl/types/fimp.h"
#include "cixl/types/func.h"

static bool type_imp(struct cx_scope *scope) {
  struct cx_box v = *cx_test(cx_pop(scope, false));
  cx_box_init(cx_push(scope), scope->cx->meta_type)->as_ptr = v.type;
  return true;
}

static bool is_imp(struct cx_scope *scope) {
  struct cx_type
    *y = cx_test(cx_pop(scope, false))->as_ptr,
    *x = cx_test(cx_pop(scope, false))->as_ptr;

  cx_box_init(cx_push(scope), scope->cx->bool_type)->as_bool = cx_is(x, y);
  return true;
}

void cx_init_type(struct cx *cx) {
  cx_add_cfunc(cx, "type", type_imp, cx_arg("v", cx->opt_type));
  cx_add_cfunc(cx, "is", is_imp,
	       cx_arg("x", cx->meta_type), cx_arg("y", cx->meta_type));
}