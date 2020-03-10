#pragma once

#include "zay/Exports.h"
#include "zay/Src.h"
#include "zay/parse/AST.h"

#include <mn/Buf.h>

#include <stddef.h>

namespace zay
{
	struct IParser
	{
		Src *src;
		mn::Buf<Tkn> tkns;
		size_t ix;
		mn::Buf<Tkn> typenames;
	};
	typedef IParser* Parser;

	ZAY_EXPORT Parser
	parser_new(Src *src);

	ZAY_EXPORT void
	parser_free(Parser self);

	inline static void
	destruct(Parser self)
	{
		parser_free(self);
	}

	ZAY_EXPORT Expr*
	parser_expr(Parser self);

	ZAY_EXPORT Stmt*
	parser_stmt(Parser self);

	ZAY_EXPORT Decl
	parser_decl(Parser self);

	ZAY_EXPORT Tkn
	parser_pkg(Parser self);

	inline static bool
	src_parse(Src *src, MODE mode)
	{
		auto self = parser_new(src);

		//first parse the package declaration
		if (mode != MODE::NONE)
			src->ast.package = parser_pkg(self);

		//then everything else
		while(self->ix < self->tkns.count)
		{
			if (Decl d = parser_decl(self))
				mn::buf_push(src->ast.decls, d);
			else
				break;
		}
		parser_free(self);
		return src_has_err(src) == false;
	}
}