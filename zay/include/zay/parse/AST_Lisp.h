#pragma once

#include "zay/Exports.h"
#include "zay/parse/AST.h"

#include <mn/Stream.h>

#include <stddef.h>

namespace zay
{
	struct AST_Lisp
	{
		mn::Stream out;
		size_t level;
	};

	inline static AST_Lisp
	ast_lisp_new(mn::Stream out)
	{
		return AST_Lisp{out, 0};
	}

	ZAY_EXPORT void
	ast_lisp_expr(AST_Lisp& self, Expr *expr);

	ZAY_EXPORT void
	ast_lisp_stmt(AST_Lisp& self, Stmt* stmt);

	ZAY_EXPORT void
	ast_lisp_decl(AST_Lisp& self, Decl decl);
}