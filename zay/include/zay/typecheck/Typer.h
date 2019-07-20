#pragma once

#include "zay/Exports.h"
#include "zay/Src.h"
#include "zay/typecheck/Scope.h"

#include <mn/Buf.h>

namespace zay
{
	typedef struct ITyper* Typer;
	struct ITyper
	{
		enum MODE
		{
			MODE_NONE,
			MODE_EXE,
			MODE_LIB
		};

		MODE mode;
		Src src;

		mn::Buf<Scope> scope_stack;
		Scope global_scope;

		size_t unnamed_id;
	};

	ZAY_EXPORT Typer
	typer_new(Src src, ITyper::MODE mode);

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
	src_typecheck(Src src, ITyper::MODE mode)
	{
		Typer self = typer_new(src, mode);
		typer_check(self);
		typer_free(self);
		return src_has_err(src) == false;
	}
}