#include "zay/Sym.h"

#include <mn/Memory.h>

namespace zay
{
	using namespace mn;

	Sym
	sym_struct(Decl d)
	{
		Sym self = alloc<ISym>();
		self->kind = ISym::KIND_STRUCT;
		self->state = ISym::STATE_UNRESOLVED;
		self->name = d->name.str;
		self->type = nullptr;
		self->struct_sym = d;
		return self;
	}

	Sym
	sym_union(Decl d)
	{
		Sym self = alloc<ISym>();
		self->kind = ISym::KIND_UNION;
		self->state = ISym::STATE_UNRESOLVED;
		self->name = d->name.str;
		self->type = nullptr;
		self->union_sym = d;
		return self;
	}

	Sym
	sym_enum(Decl d)
	{
		Sym self = alloc<ISym>();
		self->kind = ISym::KIND_ENUM;
		self->state = ISym::STATE_UNRESOLVED;
		self->name = d->name.str;
		self->type = nullptr;
		self->enum_sym = d;
		return self;
	}

	Sym
	sym_var(const Tkn& id, const Type_Sign& type, Expr expr)
	{
		Sym self = alloc<ISym>();
		self->kind = ISym::KIND_VAR;
		self->state = ISym::STATE_UNRESOLVED;
		self->name = id.str;
		self->type = nullptr;
		self->var_sym.id = id;
		self->var_sym.type = type;
		self->var_sym.expr = expr;
		return self;
	}

	Sym
	sym_func(Decl d)
	{
		Sym self = alloc<ISym>();
		self->kind = ISym::KIND_FUNC;
		self->state = ISym::STATE_UNRESOLVED;
		self->name = d->name.str;
		self->type = nullptr;
		self->func_sym = d;
		return self;
	}

	void
	sym_free(Sym self)
	{
		free(self);
	}
}