#include "zay/parse/Decl.h"

#include <mn/Memory.h>

namespace zay
{
	Decl
	decl_var(const Var& v)
	{
		Decl self = mn::alloc<IDecl>();
		self->kind = IDecl::KIND_VAR;
		//let's put the first id as the name of this declaration
		if(v.ids.count > 0)
			self->name = v.ids[0];
		self->var_decl = v;
		return self;
	}

	Decl
	decl_func(const Tkn& name, const mn::Buf<Arg>& args, const Type_Sign& ret_type, Stmt body)
	{
		Decl self = mn::alloc<IDecl>();
		self->kind = IDecl::KIND_FUNC;
		self->name = name;
		self->func_decl.args = args;
		self->func_decl.ret_type = ret_type;
		self->func_decl.body = body;
		return self;
	}

	Decl
	decl_type(const Tkn& name, const Type_Sign& type)
	{
		Decl self = mn::alloc<IDecl>();
		self->kind = IDecl::KIND_TYPE;
		self->name = name;
		self->type_decl = type;
		return self;
	}

	void
	decl_free(Decl self)
	{
		switch(self->kind)
		{
		case IDecl::KIND_VAR:
			var_free(self->var_decl);
			break;
		case IDecl::KIND_FUNC:
			destruct(self->func_decl.args);
			type_sign_free(self->func_decl.ret_type);
			if(self->func_decl.body)
				stmt_free(self->func_decl.body);
			break;
		case IDecl::KIND_TYPE:
			type_sign_free(self->type_decl);
			break;
		default: assert(false && "unreachable"); break;
		}
		mn::free(self);
	}
}