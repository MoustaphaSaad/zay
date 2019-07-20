#pragma once

#include "zay/Exports.h"
#include "zay/ir/Type.h"
#include "zay/ir/Ins.h"
#include "zay/ir/Basic_Block.h"

#include <mn/Str.h>
#include <mn/Buf.h>

namespace zay::ir
{
	typedef struct IFunc* Func;

	struct IFunc
	{
		mn::Str name;
		Type type;
	};

	ZAY_EXPORT Func
	func_new(const mn::Str& name, Type type);

	ZAY_EXPORT void
	func_free(Func& self);

	inline static void
	destruct(Func& self)
	{
		func_free(self);
	}
}
