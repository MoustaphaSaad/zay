#pragma once

#include "cypher/Val.h"

#include <stddef.h>

namespace cypher
{
	typedef size_t Ins_ID;

	struct Ins
	{
		enum OP
		{
			OP_NONE,
			OP_ADD
		};

		OP op;
		union
		{
			struct {
				Val a, b;
			} op_add;
		};
	};

	inline static Ins
	ins_add(Val a, Val b)
	{
		Ins self{};
		self.op = Ins::OP_ADD;
		self.op_add = { a, b };
		return self;
	}
}
