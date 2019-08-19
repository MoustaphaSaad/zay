#include "alp/Decl.h"

#include <mn/Memory.h>

#include <assert.h>

namespace alp
{
	using namespace mn;

	//API
	Decl
	decl_token(const Tkn& name, const Buf<Tkn>& regex)
	{
		Decl self = alloc_zerod<IDecl>();
		self->kind = IDecl::KIND_TOKEN;
		self->name = name;
		self->regex = regex;
		return self;
	}

	void
	decl_free(Decl self)
	{
		switch(self->kind)
		{
		case IDecl::KIND_TOKEN:
			buf_free(self->regex);
			break;

		default:
			assert(false && "unreachable");
			break;
		}
		free(self);
	}
}