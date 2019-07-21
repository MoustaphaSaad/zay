#pragma once

#include "cypher/Ins.h"

#include <mn/Buf.h>

#include <stddef.h>

namespace cypher
{
	typedef size_t Basic_Block_ID;

	struct Basic_Block
	{
		mn::Buf<Ins_ID> instructions;
	};

	inline static Basic_Block
	basic_block_new()
	{
		Basic_Block self{};
		self.instructions = mn::buf_new<Ins_ID>();
		return self;
	}

	inline static void
	basic_block_free(Basic_Block& self)
	{
		mn::buf_free(self.instructions);
	}

	inline static void
	destruct(Basic_Block& self)
	{
		basic_block_free(self);
	}
}
