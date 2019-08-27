#pragma once

#include "asmm/Exports.h"
#include "asmm/Src.h"

#include <mn/Buf.h>

#include <stddef.h>

namespace asmm
{
	struct IParser
	{
		Src src;
		mn::Buf<Tkn> tkns;
		size_t ix;
	};
	typedef IParser* Parser;

	ASMM_EXPORT Parser
	parser_new(Src src);

	ASMM_EXPORT void
	parser_free(Parser self);

	inline static void
	destruct(Parser self)
	{
		parser_free(self);
	}

	ASMM_EXPORT Func
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