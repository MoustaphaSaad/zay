#pragma once

#include "zay/Exports.h"
#include "zay/Rng.h"
#include "zay/Pos.h"
#include "zay/Tkn.h"
#include "zay/Var.h"

#include <mn/Buf.h>

namespace zay
{
	typedef struct IExpr* Expr;

	//Statements
	typedef struct IStmt* Stmt;

	struct Else_If
	{
		Expr cond;
		Stmt body;
	};

	struct IStmt
	{
		enum KIND
		{
			KIND_NONE,
			KIND_BREAK,
			KIND_CONTINUE,
			KIND_RETURN,
			KIND_IF,
			KIND_FOR,
			KIND_VAR,
			KIND_ASSIGN,
			KIND_EXPR,
			KIND_BLOCK
		};

		KIND kind;
		Rng rng;
		Pos pos;
		union
		{
			Tkn break_stmt;

			Tkn continue_stmt;

			Expr return_stmt;

			struct
			{
				Expr if_cond;
				Stmt if_body;
				mn::Buf<Else_If> else_ifs;
				Stmt else_body;
			} if_stmt;

			struct
			{
				Stmt init_stmt;
				Expr loop_cond;
				Stmt post_stmt;
				Stmt loop_body;
			} for_stmt;

			Var var_stmt;

			struct
			{
				mn::Buf<Expr> lhs;
				Tkn op;
				mn::Buf<Expr> rhs;
			} assign_stmt;

			Expr expr_stmt;

			mn::Buf<Stmt> block_stmt;
		};
	};

	ZAY_EXPORT Stmt
	stmt_break(const Tkn& t);

	ZAY_EXPORT Stmt
	stmt_continue(const Tkn& t);

	ZAY_EXPORT Stmt
	stmt_return(Expr e);

	ZAY_EXPORT Stmt
	stmt_if(Expr if_cond, Stmt if_body, const mn::Buf<Else_If>& else_ifs, Stmt else_body);

	ZAY_EXPORT Stmt
	stmt_for(Stmt init_stmt, Expr loop_cond, Stmt post_stmt, Stmt loop_body);

	ZAY_EXPORT Stmt
	stmt_var(Var v);

	ZAY_EXPORT Stmt
	stmt_block(const mn::Buf<Stmt>& stmts);

	ZAY_EXPORT Stmt
	stmt_assign(const mn::Buf<Expr>& lhs, const Tkn& op, const mn::Buf<Expr>& rhs);

	ZAY_EXPORT Stmt
	stmt_expr(Expr e);

	ZAY_EXPORT Expr
	stmt_expr_decay(Stmt expr);

	ZAY_EXPORT void
	stmt_free(Stmt self);

	inline static void
	destruct(Stmt self)
	{
		stmt_free(self);
	}
}
