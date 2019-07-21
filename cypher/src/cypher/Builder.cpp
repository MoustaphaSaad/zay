#include "cypher/Builder.h"

#include <mn/Memory.h>

namespace cypher
{
	using namespace mn;

	//API
	Builder
	builder_new(Func f)
	{
		Builder self = alloc<IBuilder>();
		self->func = f;
		self->block = Basic_Block_ID(-1);
		return self;
	}

	void
	builder_free(Builder self)
	{
		free(self);
	}

	Basic_Block_ID
	builder_basic_block(Builder self)
	{
		Basic_Block_ID id = self->func->blocks.count;
		buf_push(self->func->blocks, basic_block_new());
		return id;
	}

	Val
	builder_add(Builder self, Val x, Val y)
	{
		assert(self->block != Basic_Block_ID(-1) && "you need to start a block before adding ins");
		Ins_ID id = self->func->ins.count;
		buf_push(self->func->ins, ins_add(x, y));
		buf_push(self->func->blocks[self->block].instructions, id);
		return val_ins(self->func, id);
	}

	Val
	builder_ret(Builder self, Val x)
	{
		assert(self->block != Basic_Block_ID(-1) && "you need to start a block before adding ins");
		Ins_ID id = self->func->ins.count;
		buf_push(self->func->ins, ins_ret(x));
		buf_push(self->func->blocks[self->block].instructions, id);
		return val_ins(self->func, id);
	}
}
