#include "cypher/Pkg.h"

#include <mn/Memory.h>

namespace cypher
{
	using namespace mn;

	//API
	Pkg
	pkg_new(const mn::Str& name)
	{
		Pkg self = alloc<IPkg>();
		self->name = name;
		self->funcs = buf_new<Func>();
		return self;
	}

	void
	pkg_free(Pkg self)
	{
		str_free(self->name);
		destruct(self->funcs);
		free(self);
	}
}
