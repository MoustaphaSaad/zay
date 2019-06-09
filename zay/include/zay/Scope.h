#pragma once

#include "zay/Exports.h"
#include "zay/Sym.h"

#include <mn/Buf.h>
#include <mn/Map.h>

namespace zay
{
	typedef struct IScope* Scope;
	struct IScope
	{
		Scope parent;
		mn::Buf<Sym> syms;
		mn::Map<const char*, Sym> table;
	};

	ZAY_EXPORT Scope
	scope_new(Scope parent);

	ZAY_EXPORT void
	scope_free(Scope self);

	inline static void
	destruct(Scope self)
	{
		scope_free(self);
	}

	ZAY_EXPORT Sym
	scope_has(Scope self, const char* name);

	ZAY_EXPORT Sym
	scope_find(Scope self, const char* name);

	ZAY_EXPORT Sym
	scope_add(Scope self, Sym sym);
}