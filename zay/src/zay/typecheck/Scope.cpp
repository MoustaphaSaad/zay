#include "zay/typecheck/Scope.h"

#include <mn/Memory.h>

namespace zay
{
	Scope*
	scope_new(Scope* parent, bool inside_loop, Type* ret)
	{
		Scope* self = mn::alloc<Scope>();
		self->parent = parent;
		self->syms = mn::buf_new<Sym*>();
		self->table = mn::map_new<const char*, Sym*>();
		self->inside_loop = inside_loop;
		self->ret = ret;
		return self;
	}

	void
	scope_free(Scope* self)
	{
		destruct(self->syms);
		mn::map_free(self->table);
		mn::free(self);
	}

	Sym*
	scope_has(Scope* self, const char* name)
	{
		if(auto it = mn::map_lookup(self->table, name))
			return it->value;
		return nullptr;
	}

	Sym*
	scope_find(Scope* self, const char* name)
	{
		for(auto it = self; it != nullptr; it = it->parent)
		{
			if(auto sym = scope_has(it, name))
				return sym;
		}
		return nullptr;
	}

	Sym*
	scope_add(Scope* self, Sym* sym)
	{
		mn::buf_push(self->syms, sym);
		mn::map_insert(self->table, sym->name, sym);
		return sym;
	}

	bool
	scope_inside_loop(Scope* self)
	{
		for(auto it = self; it != nullptr; it = it->parent)
		{
			if (it->inside_loop)
				return true;
		}
		return false;
	}

	Type*
	scope_ret(Scope* self)
	{
		for(auto it = self; it != nullptr; it = it->parent)
		{
			if (it->ret)
				return it->ret;
		}
		return nullptr;
	}
}