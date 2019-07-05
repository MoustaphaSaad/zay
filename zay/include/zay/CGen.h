#pragma once

#include "zay/Exports.h"
#include "zay/Src.h"
#include "zay/Scope.h"

#include <mn/Stream.h>
#include <mn/Buf.h>

#include <stddef.h>

namespace zay
{
	typedef struct ICGen* CGen;
	struct ICGen
	{
		size_t indent;
		Src src;
		mn::Stream out;
		mn::Buf<Scope> scope_stack;
	};

	ZAY_EXPORT CGen
	cgen_new(Src src);

	ZAY_EXPORT void
	cgen_free(CGen self);

	inline static void
	destruct(CGen self)
	{
		cgen_free(self);
	}

	ZAY_EXPORT void
	cgen_gen(CGen self);

	inline static mn::Str
	src_c(Src src, mn::Allocator allocator = mn::allocator_top())
	{
		CGen self = cgen_new(src);
		cgen_gen(self);
		mn::Str res = mn::str_from_c(mn::stream_str(self->out), allocator);
		cgen_free(self);
		return res;
	}
}