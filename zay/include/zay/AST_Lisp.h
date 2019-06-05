#pragma once

#include "zay/Exports.h"
#include "zay/AST.h"

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
	ast_lisp_decl(AST_Lisp& self, Decl decl);
}