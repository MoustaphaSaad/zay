#pragma once

#include "alp/Exports.h"
#include "alp/Src.h"
#include "alp/AST.h"
#include "alp/Tkn.h"

#include <mn/Buf.h>

#include <stddef.h>

namespace alp
{
	typedef struct IParser* Parser;
	struct IParser
	{
		Src src;
		mn::Buf<Tkn> tkns;
		size_t ix;
		mn::Buf<Tkn> typenames;
	};

	ALP_EXPORT Parser
	parser_new(Src src);

	ALP_EXPORT void
	parser_free(Parser self);

	inline static void
	destruct(Parser self)
	{
		parser_free(self);
	}

	ALP_EXPORT Decl
	parser_decl(Parser self);

	ALP_EXPORT Tkn
	parser_pkg(Parser self);

	inline static bool
	src_parse(Src src)
	{
		Parser self = parser_new(src);

		src->ast->package = parser_pkg(self);

		//then everything else
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