#pragma once

#include "zay/Exports.h"
#include "zay/Src.h"
#include "zay/AST.h"

#include <mn/Buf.h>

#include <stddef.h>

namespace zay
{
	struct IParser
	{
		Src src;
		mn::Buf<Tkn> tkns;
		size_t ix;
	};
	typedef IParser* Parser;

	ZAY_EXPORT Parser
	parser_new(Src src);

	ZAY_EXPORT void
	parser_free(Parser self);

	inline static void
	destruct(Parser self)
	{
		parser_free(self);
	}

	ZAY_EXPORT Expr
	parser_expr(Parser self);

	ZAY_EXPORT Decl
	parser_decl(Parser self);

	inline static bool
	src_parse(Src src)
	{
		Parser self = parser_new(src);
		while(self->ix < self->tkns.count)
		{
			if (Decl d = parser_decl(self))
				mn::buf_push(src->ast->decls, d);
			else
				break;
		}
		parser_free(self);
		return src_has_err(src) == false;
	}
}