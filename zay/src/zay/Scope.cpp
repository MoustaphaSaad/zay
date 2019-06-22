#include "zay/Scope.h"

#include <mn/Memory.h>

namespace zay
{
	using namespace mn;

	Scope
	scope_new(Scope parent)
	{
		Scope self = alloc<IScope>();
		self->parent = parent;
		self->syms = buf_new<Sym>();
		self->table = map_new<const char*, Sym>();
		return self;
	}

	void
	scope_free(Scope self)
	{
		destruct(self->syms);
		map_free(self->table);
		free(self);
	}

	Sym
	scope_has(Scope self, const char* name)
	{
		if(auto it = map_lookup(self->table, name))
			return it->value;
		return nullptr;
	}

	Sym
	scope_find(Scope self, const char* name)
	{
		for(Scope it = self; it != nullptr; it = it->parent)
		{
			if(Sym sym = scope_has(it, name))
				return sym;
		}
		return nullptr;
	}

	Sym
	scope_add(Scope self, Sym sym)
	{
		buf_push(self->syms, sym);
		map_insert(self->table, sym->name, sym);
		return sym;
	}
}