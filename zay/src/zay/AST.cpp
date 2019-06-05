#include "zay/AST.h"

#include <mn/Memory.h>

#include <assert.h>

namespace zay
{
	using namespace mn;

	Decl
	decl_struct(const Tkn& name, const mn::Buf<Field>& fields)
	{
		Decl self = alloc<IDecl>();
		self->kind = IDecl::KIND_STRUCT;
		self->name = name;
		self->struct_decl = fields;
		return self;
	}

	Decl
	decl_union(const Tkn& name, const mn::Buf<Field>& fields)
	{
		Decl self = alloc<IDecl>();
		self->kind = IDecl::KIND_UNION;
		self->name = name;
		self->union_decl = fields;
		return self;
	}

	void
	decl_free(Decl self)
	{
		switch(self->kind)
		{
		case IDecl::KIND_STRUCT: destruct(self->struct_decl); break;
		case IDecl::KIND_UNION: destruct(self->union_decl); break;
		default: assert(false && "unreachable"); break;
		}
		free(self);
	}


	AST
	ast_new()
	{
		AST self = alloc<IAST>();
		self->decls = buf_new<Decl>();
		return self;
	}

	void
	ast_free(AST self)
	{
		destruct(self->decls);
		free(self);
	}
}