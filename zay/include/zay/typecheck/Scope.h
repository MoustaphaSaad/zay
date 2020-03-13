#pragma once

#include "zay/Exports.h"
#include "zay/typecheck/Sym.h"

#include <mn/Buf.h>
#include <mn/Map.h>

namespace zay
{
	struct Scope
	{
		Scope* parent;
		mn::Buf<Sym*> syms;
		mn::Map<const char*, Sym*> table;
		bool inside_loop;
		Type* ret;
	};

	ZAY_EXPORT Scope*
	scope_new(Scope* parent, bool inside_loop, Type* ret);

	ZAY_EXPORT void
	scope_free(Scope* self);

	inline static void
	destruct(Scope* self)
	{
		scope_free(self);
	}

	ZAY_EXPORT Sym*
	scope_has(Scope* self, const char* name);

	ZAY_EXPORT Sym*
	scope_find(Scope* self, const char* name);

	ZAY_EXPORT Sym*
	scope_add(Scope* self, Sym* sym);

	ZAY_EXPORT bool
	scope_inside_loop(Scope* self);

	ZAY_EXPORT Type*
	scope_ret(Scope* self);
}