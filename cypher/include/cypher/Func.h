#pragma once

#include "cypher/Exports.h"
#include "cypher/Type.h"
#include "cypher/Basic_Block.h"
#include "cypher/Val.h"
#include "cypher/Ins.h"

#include <mn/Str.h>
#include <mn/Buf.h>

namespace cypher
{
	typedef struct IFunc* Func;

	struct IFunc
	{
		mn::Str name;
		Type type;
		mn::Buf<Basic_Block> blocks;
		mn::Buf<Ins> ins;
	};

	CYPHER_EXPORT Func
	func_new(const mn::Str& name, Type type);

	CYPHER_EXPORT void
	func_free(Func self);

	inline static void
	destruct(Func self)
	{
		func_free(self);
	}

	CYPHER_EXPORT Val
	func_arg(Func self, size_t ix);
}
