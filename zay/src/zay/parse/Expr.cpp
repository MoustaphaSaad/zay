#include "zay/parse/Expr.h"
#include "zay/typecheck/Type_Intern.h"

#include <mn/Memory.h>

namespace zay
{
	Expr*
	expr_atom(const Tkn& t)
	{
		auto self = mn::alloc<Expr>();
		self->type = type_void;
		self->kind = Expr::KIND_ATOM;
		self->atom = t;
		return self;
	}

	Expr*
	expr_paren(Expr* e)
	{
		auto self = mn::alloc<Expr>();
		self->type = type_void;
		self->kind = Expr::KIND_PAREN;
		self->paren = e;
		return self;
	}

	Expr*
	expr_call(Expr* base, const mn::Buf<Expr*>& args)
	{
		auto self = mn::alloc<Expr>();
		self->type = type_void;
		self->kind = Expr::KIND_CALL;
		self->call.base = base;
		self->call.args = args;
		return self;
	}

	Expr*
	expr_indexed(Expr* base, Expr* index)
	{
		auto self = mn::alloc<Expr>();
		self->type = type_void;
		self->kind = Expr::KIND_INDEXED;
		self->indexed.base = base;
		self->indexed.index = index;
		return self;
	}

	Expr*
	expr_dot(Expr* base, const Tkn& t)
	{
		auto self = mn::alloc<Expr>();
		self->type = type_void;
		self->kind = Expr::KIND_DOT;
		self->dot.base = base;
		self->dot.member = t;
		return self;
	}

	Expr*
	expr_unary(const Tkn& op, Expr* expr)
	{
		auto self = mn::alloc<Expr>();
		self->type = type_void;
		self->kind = Expr::KIND_UNARY;
		self->unary.op = op;
		self->unary.expr = expr;
		return self;
	}

	Expr*
	expr_cast(Expr* expr, const Type_Sign& type)
	{
		auto self = mn::alloc<Expr>();
		self->type = type_void;
		self->kind = Expr::KIND_CAST;
		self->cast.base = expr;
		self->cast.type = type;
		return self;
	}

	Expr*
	expr_binary(Expr* lhs, const Tkn& op, Expr* rhs)
	{
		auto self = mn::alloc<Expr>();
		self->type = type_void;
		self->kind = Expr::KIND_BINARY;
		self->binary.lhs = lhs;
		self->binary.op = op;
		self->binary.rhs = rhs;
		return self;
	}

	Expr*
	expr_complit(const Type_Sign& type, const mn::Buf<Complit_Field>& fields)
	{
		auto self = mn::alloc<Expr>();
		self->type = type_void;
		self->kind = Expr::KIND_COMPLIT;
		self->complit.type = type;
		self->complit.fields = fields;
		return self;
	}

	void
	expr_free(Expr* self)
	{
		switch (self->kind)
		{
		case Expr::KIND_ATOM: break;
		case Expr::KIND_BINARY:
			expr_free(self->binary.lhs);
			expr_free(self->binary.rhs);
			break;
		case Expr::KIND_UNARY:
			expr_free(self->unary.expr);
			break;
		case Expr::KIND_DOT:
			expr_free(self->dot.base);
			break;
		case Expr::KIND_INDEXED:
			expr_free(self->indexed.base);
			expr_free(self->indexed.index);
			break;
		case Expr::KIND_CALL:
			expr_free(self->call.base);
			destruct(self->call.args);
			break;
		case Expr::KIND_CAST:
			expr_free(self->cast.base);
			type_sign_free(self->cast.type);
			break;
		case Expr::KIND_PAREN:
			expr_free(self->paren);
			break;
		case Expr::KIND_COMPLIT:
			type_sign_free(self->complit.type);
			for (const Complit_Field& f : self->complit.fields)
			{
				expr_free(f.left);
				expr_free(f.right);
			}
			mn::buf_free(self->complit.fields);
			break;
		default: assert(false && "unreachable"); break;
		}
		mn::free(self);
	}
}
