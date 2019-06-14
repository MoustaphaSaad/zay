#include "zay/Typer.h"

#include <mn/Memory.h>

namespace zay
{
	using namespace mn;

	inline static void
	typer_scope_enter(Typer self, Scope scope)
	{
		buf_push(self->scope_stack, scope);
	}

	inline static void
	typer_scope_leave(Typer self)
	{
		buf_pop(self->scope_stack);
	}


	//API
	Typer
	typer_new(Src src)
	{
		Typer self = alloc<ITyper>();
		self->src = src;
		self->scope_stack = buf_new<Scope>();
		self->global_scope = src_scope_new(self->src, nullptr, nullptr);

		typer_scope_enter(self, self->global_scope);
		return self;
	}

	void
	typer_free(Typer self)
	{
		buf_free(self->scope_stack);
		free(self);
	}

	void
	typer_check(Typer )
	{
		//so we need to do type checking here
		//but first let's check that we compile
		//okay we compile
		//let's have a break
	}
}