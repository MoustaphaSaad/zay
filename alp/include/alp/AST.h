#pragma once

#include "alp/Exports.h"
#include "alp/Decl.h"

#include <mn/Buf.h>

namespace alp
{
	typedef struct IAST *AST;
	struct IAST
	{
		Tkn package;
		mn::Buf<Decl> decls;
	};

	ALP_EXPORT AST
	ast_new();

	ALP_EXPORT void
	ast_free(AST self);

	inline static void
	destruct(AST self)
	{
		ast_free(self);
	}
}