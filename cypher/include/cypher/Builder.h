#pragma once

#include "cypher/Exports.h"
#include "cypher/Func.h"
#include "cypher/Val.h"

#include <stddef.h>

namespace cypher
{
	typedef struct IBuilder* Builder;
	struct IBuilder
	{
		Func func;
		Basic_Block_ID block;
	};

	CYPHER_EXPORT Builder
	builder_new(Func f);

	CYPHER_EXPORT void
	builder_free(Builder self);

	inline static void
	destruct(Builder self)
	{
		builder_free(self);
	}

	CYPHER_EXPORT Basic_Block_ID
	builder_basic_block(Builder self);

	inline static Val
	builder_arg(Builder self, size_t ix)
	{
		return func_arg(self->func, ix);
	}

	inline static void
	builder_start_block(Builder self, Basic_Block_ID b)
	{
		self->block = b;
	}

	inline static void
	builder_end_block(Builder self)
	{
		self->block = Basic_Block_ID(-1);
	}

	CYPHER_EXPORT Val
	builder_add(Builder self, Val x, Val y);

	CYPHER_EXPORT Val
	builder_ret(Builder self, Val x);
}
