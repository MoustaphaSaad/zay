#include "zay/AST.h"

#include <mn/Memory.h>

#include <assert.h>

namespace zay
{
	using namespace mn;

	Expr
	expr_atom(const Tkn& t)
	{
		Expr self = alloc<IExpr>();
		self->kind = IExpr::KIND_ATOM;
		self->atom = t;
		return self;
	}

	Expr
	expr_paren(Expr e)
	{
		Expr self = alloc<IExpr>();
		self->kind = IExpr::KIND_PAREN;
		self->paren = e;
		return self;
	}

	Expr
	expr_call(Expr base, const mn::Buf<Expr>& args)
	{
		Expr self = alloc<IExpr>();
		self->kind = IExpr::KIND_CALL;
		self->call.base = base;
		self->call.args = args;
		return self;
	}

	Expr
	expr_indexed(Expr base, Expr index)
	{
		Expr self = alloc<IExpr>();
		self->kind = IExpr::KIND_INDEXED;
		self->indexed.base = base;
		self->indexed.index = index;
		return self;
	}

	Expr
	expr_dot(Expr base, const Tkn& t)
	{
		Expr self = alloc<IExpr>();
		self->kind = IExpr::KIND_DOT;
		self->dot.base = base;
		self->dot.member = t;
		return self;
	}

	Expr
	expr_unary(const Tkn& op, Expr expr)
	{
		Expr self = alloc<IExpr>();
		self->kind = IExpr::KIND_UNARY;
		self->unary.op = op;
		self->unary.expr = expr;
		return self;
	}

	Expr
	expr_cast(Expr expr, const Type_Sign& type)
	{
		Expr self = alloc<IExpr>();
		self->kind = IExpr::KIND_CAST;
		self->cast.base = expr;
		self->cast.type = type;
		return self;
	}

	Expr
	expr_binary(Expr lhs, const Tkn& op, Expr rhs)
	{
		Expr self = alloc<IExpr>();
		self->kind = IExpr::KIND_BINARY;
		self->binary.lhs = lhs;
		self->binary.op = op;
		self->binary.rhs = rhs;
		return self;
	}

	void
	expr_free(Expr self)
	{
		switch(self->kind)
		{
		case IExpr::KIND_ATOM: break;
		case IExpr::KIND_BINARY:
			expr_free(self->binary.lhs);
			expr_free(self->binary.rhs);
			break;
		case IExpr::KIND_UNARY:
			expr_free(self->unary.expr);
			break;
		case IExpr::KIND_DOT:
			expr_free(self->dot.base);
			break;
		case IExpr::KIND_INDEXED:
			expr_free(self->indexed.base);
			expr_free(self->indexed.index);
			break;
		case IExpr::KIND_CALL:
			expr_free(self->call.base);
			destruct(self->call.args);
			break;
		case IExpr::KIND_CAST:
			expr_free(self->cast.base);
			type_sign_free(self->cast.type);
			break;
		case IExpr::KIND_PAREN:
			expr_free(self->paren);
			break;
		default: assert(false && "unreachable"); break;
		}
		free(self);
	}



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