#pragma once

#include "zay/Exports.h"
#include "zay/Decl.h"

#include <mn/Buf.h>

namespace zay
{
	typedef struct IAST* AST;
	struct IAST
	{
		mn::Buf<Decl> decls;
	};

	ZAY_EXPORT AST
	ast_new();

	ZAY_EXPORT void
	ast_free(AST self);

	inline static void
	destruct(AST self)
	{
		ast_free(self);
	}
}