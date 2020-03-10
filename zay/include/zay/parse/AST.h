#pragma once

#include "zay/Exports.h"
#include "zay/parse/Decl.h"

#include <mn/Buf.h>

namespace zay
{
	struct AST
	{
		Tkn package;
		mn::Buf<Decl*> decls;
	};

	ZAY_EXPORT AST
	ast_new();

	ZAY_EXPORT void
	ast_free(AST &self);

	inline static void
	destruct(AST &self)
	{
		ast_free(self);
	}
}