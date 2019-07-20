#pragma once

#include "zay/Exports.h"
#include "zay/ir/Func.h"

#include <mn/Str.h>
#include <mn/Buf.h>

namespace zay::ir
{
	typedef struct IPkg* Pkg;
	struct IPkg
	{
		mn::Str name;
		mn::Buf<Func> functions;
	};

	ZAY_EXPORT Pkg
	pkg_new(const mn::Str& name);

	ZAY_EXPORT void
	pkg_free(Pkg self);

	inline static void
	destruct(Pkg self)
	{
		pkg_free(self);
	}
}
