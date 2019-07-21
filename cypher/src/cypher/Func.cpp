#include "cypher/Func.h"

#include <mn/Memory.h>

namespace cypher
{
	using namespace mn;

	//API
	Func
	func_new(const Str& name, Type type)
	{
		assert(type->kind == IType::KIND_FUNC);

		Func self = alloc<IFunc>();
		self->name = name;
		self->type = type;
		self->blocks = buf_new<Basic_Block>();
		self->ins = buf_new<Ins>();
		return self;
	}

	void
	func_free(Func self)
	{
		str_free(self->name);
		type_free(self->type);
		destruct(self->blocks);
		buf_free(self->ins);
		free(self);
	}

	Val
	func_arg(Func self, size_t ix)
	{
		if (ix >= self->type->func.args.count)
			return Val{};
		return val_arg(self, ix);
	}
}
