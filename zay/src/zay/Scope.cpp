#include "zay/Scope.h"

#include <mn/Memory.h>

namespace zay
{
	using namespace mn;

	Scope
	scope_new(Scope parent, bool inside_loop, Type ret)
	{
		Scope self = alloc<IScope>();
		self->parent = parent;
		self->syms = buf_new<Sym>();
		self->table = map_new<const char*, Sym>();
		self->inside_loop = inside_loop;
		self->ret = ret;
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

	bool
	scope_inside_loop(Scope self)
	{
		for(Scope it = self; it != nullptr; it = it->parent)
		{
			if (it->inside_loop)
				return true;
		}
		return false;
	}

	Type
	scope_ret(Scope self)
	{
		for(Scope it = self; it != nullptr; it = it->parent)
		{
			if (it->ret)
				return it->ret;
		}
		return nullptr;
	}
}