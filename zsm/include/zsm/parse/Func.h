#pragma once

#include "zsm/Exports.h"
#include "zsm/scan/Tkn.h"
#include "zsm/parse/Ins.h"

#include <mn/Buf.h>

namespace zsm
{
	struct Func
	{
		Tkn name;
		mn::Buf<Ins> ins;
	};

	ZSM_EXPORT Func
	func_new(const Tkn& name);

	ZSM_EXPORT void
	func_free(Func& self);

	inline static void
	destruct(Func& self)
	{
		func_free(self);
	}
}