#pragma once

#include "asmm/Exports.h"
#include "asmm/scan/Tkn.h"
#include "asmm/parse/Ins.h"

#include <mn/Buf.h>

namespace asmm
{
	struct Func
	{
		Tkn name;
		mn::Buf<Ins> ins;
	};

	ASMM_EXPORT Func
	func_new(const Tkn& name);

	ASMM_EXPORT void
	func_free(Func& self);

	inline static void
	destruct(Func& self)
	{
		func_free(self);
	}
}