#include "zay/ir/Func.h"

#include <mn/Memory.h>

namespace zay::ir
{
	using namespace mn;

	//API
	Func
	func_new(const Str& name, Type type)
	{
		Func self = alloc<IFunc>();
		self->name = name;
		self->type = type;
		return self;
	}

	void
	func_free(Func& self)
	{
		str_free(self->name);
		type_free(self->type);
		free(self);
	}
}
