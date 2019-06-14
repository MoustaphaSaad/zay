#pragma once

#include "zay/Exports.h"
#include "zay/Src.h"
#include "zay/Scope.h"

#include <mn/Buf.h>

namespace zay
{
	typedef struct ITyper* Typer;
	struct ITyper
	{
		Src src;

		mn::Buf<Scope> scope_stack;
		Scope global_scope;
	};

	ZAY_EXPORT Typer
	typer_new(Src src);

	ZAY_EXPORT void
	typer_free(Typer self);

	inline static void
	destruct(Typer self)
	{
		typer_free(self);
	}

	ZAY_EXPORT void
	typer_check(Typer self);

	inline static bool
	src_typecheck(Src src)
	{
		Typer self = typer_new(src);
		typer_check(self);
		typer_free(self);
		return src_has_err(src) == false;
	}
}