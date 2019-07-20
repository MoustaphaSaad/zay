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
		buf_push(self->func->blocks, Basic_Block{});
		return id;
	}

	Val
	builder_add(Builder self, Val x, Val y)
	{
		assert(self->block != Basic_Block_ID(-1) && "you need to start a block before adding ins");
		Ins_ID id = self->func->ins.count;
		buf_push(self->func->ins, ins_add(x, y));
		return val_ins(self->func, id);
	}
}
