#pragma once

#include "zsm/Exports.h"
#include "zsm/Src.h"

#include <mn/Buf.h>

#include <stddef.h>

namespace zsm
{
	struct IParser
	{
		Src src;
		mn::Buf<Tkn> tkns;
		size_t ix;
	};
	typedef IParser* Parser;

	ZSM_EXPORT Parser
	parser_new(Src src);

	ZSM_EXPORT void
	parser_free(Parser self);

	inline static void
	destruct(Parser self)
	{
		parser_free(self);
	}

	ZSM_EXPORT Func
	parser_func(Parser self);

	inline static bool
	src_parse(Src src)
	{
		Parser self = parser_new(src);

		//then everything else
		while(self->ix < self->tkns.count)
		{
			mn::buf_push(src->funcs, parser_func(self));
		}
		parser_free(self);
		return src_has_err(src) == false;
	}
}