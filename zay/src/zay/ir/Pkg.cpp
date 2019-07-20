#include "zay/ir/Pkg.h"

#include <mn/Memory.h>

namespace zay::ir
{
	using namespace mn;

	//API
	Pkg
	pkg_new(const mn::Str& name)
	{
		Pkg self = alloc<IPkg>();
		self->name = name;
		self->functions = buf_new<Func>();
		return self;
	}

	void
	pkg_free(Pkg self)
	{
		str_free(self->name);
		destruct(self->functions);
		free(self);
	}
}
