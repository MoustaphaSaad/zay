#pragma once

#include "zay/Exports.h"
#include "zay/Rng.h"
#include "zay/Pos.h"
#include "zay/Tkn.h"
#include "zay/Type_Sign.h"

#include <mn/Buf.h>

namespace zay
{
	typedef struct IType* Type;

	//Expressions
	typedef struct IExpr* Expr;
	struct IExpr
	{
		enum KIND
		{
			KIND_NONE,
			KIND_ATOM,
			KIND_BINARY,
			KIND_UNARY,
			KIND_DOT,
			KIND_INDEXED,
			KIND_CALL,
			KIND_CAST,
			KIND_PAREN
		};

		KIND kind;
		Rng rng;
		Pos pos;
		Type type;
		union
		{
			Tkn atom;

			struct
			{
				Expr lhs;
				Tkn op;
				Expr rhs;
			} binary;

			struct
			{
				Tkn op;
				Expr expr;
			} unary;

			struct
			{
				Expr base;
				Tkn member;
			} dot;

			struct
			{
				Expr base;
				Expr index;
			} indexed;

			struct
			{
				Expr base;
				mn::Buf<Expr> args;
			} call;

			struct
			{
				Expr base;
				Type_Sign type;
			} cast;

			Expr paren;
		};
	};

	ZAY_EXPORT Expr
	expr_atom(const Tkn& t);

	ZAY_EXPORT Expr
	expr_paren(Expr e);

	ZAY_EXPORT Expr
	expr_call(Expr base, const mn::Buf<Expr>& args);

	ZAY_EXPORT Expr
	expr_indexed(Expr base, Expr index);

	ZAY_EXPORT Expr
	expr_dot(Expr base, const Tkn& t);

	ZAY_EXPORT Expr
	expr_unary(const Tkn& op, Expr expr);

	ZAY_EXPORT Expr
	expr_cast(Expr expr, const Type_Sign& type);

	ZAY_EXPORT Expr
	expr_binary(Expr lhs, const Tkn& op, Expr rhs);

	ZAY_EXPORT void
	expr_free(Expr self);

	inline static void
	destruct(Expr self)
	{
		expr_free(self);
	}
}
