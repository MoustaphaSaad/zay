#include "zay/typecheck/Sym.h"

#include <mn/Memory.h>

namespace zay
{
	Sym*
	sym_struct(Decl* d)
	{
		auto self = mn::alloc<Sym>();
		self->kind = Sym::KIND_STRUCT;
		self->state = Sym::STATE_UNRESOLVED;
		self->name = d->name.str;
		self->package_name = mn::str_lit(self->name);
		self->type = nullptr;
		self->struct_sym = d;
		return self;
	}

	Sym*
	sym_union(Decl* d)
	{
		auto self = mn::alloc<Sym>();
		self->kind = Sym::KIND_UNION;
		self->state = Sym::STATE_UNRESOLVED;
		self->name = d->name.str;
		self->package_name = mn::str_lit(self->name);
		self->type = nullptr;
		self->union_sym = d;
		return self;
	}

	Sym*
	sym_enum(Decl* d)
	{
		auto self = mn::alloc<Sym>();
		self->kind = Sym::KIND_ENUM;
		self->state = Sym::STATE_UNRESOLVED;
		self->name = d->name.str;
		self->package_name = mn::str_lit(self->name);
		self->type = nullptr;
		self->enum_sym = d;
		return self;
	}

	Sym*
	sym_var(const Tkn& id, Decl* decl, const Type_Sign& type, Expr *expr)
	{
		auto self = mn::alloc<Sym>();
		self->kind = Sym::KIND_VAR;
		self->state = Sym::STATE_UNRESOLVED;
		self->name = id.str;
		self->package_name = mn::str_lit(self->name);
		self->type = nullptr;
		self->var_sym.id = id;
		self->var_sym.decl = decl;
		self->var_sym.type = type;
		self->var_sym.expr = expr;
		return self;
	}

	Sym*
	sym_func(Decl* d)
	{
		auto self = mn::alloc<Sym>();
		self->kind = Sym::KIND_FUNC;
		self->state = Sym::STATE_UNRESOLVED;
		self->name = d->name.str;
		self->package_name = mn::str_lit(self->name);
		self->type = nullptr;
		self->func_sym = d;
		return self;
	}

	Sym*
	sym_type(Decl* d)
	{
		auto self = mn::alloc<Sym>();
		self->kind = Sym::KIND_TYPE;
		self->state = Sym::STATE_UNRESOLVED;
		self->name = d->name.str;
		self->package_name = mn::str_lit(self->name);
		self->type = nullptr;
		self->type_sym = d;
		return self;
	}

	void
	sym_free(Sym* self)
	{
		mn::str_free(self->package_name);
		mn::free(self);
	}
}