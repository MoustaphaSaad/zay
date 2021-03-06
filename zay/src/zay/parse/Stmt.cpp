#include "zay/parse/Stmt.h"
#include "zay/parse/Expr.h"
#include "zay/parse/AST.h"

#include <mn/Memory.h>

namespace zay
{
	Stmt*
	stmt_break(const Tkn& t)
	{
		auto self = mn::alloc<Stmt>();
		self->kind = Stmt::KIND_BREAK;
		self->break_stmt = t;
		return self;
	}

	Stmt*
	stmt_continue(const Tkn& t)
	{
		auto self = mn::alloc<Stmt>();
		self->kind = Stmt::KIND_CONTINUE;
		self->continue_stmt = t;
		return self;
	}

	Stmt*
	stmt_return(Expr *e)
	{
		auto self = mn::alloc<Stmt>();
		self->kind = Stmt::KIND_RETURN;
		self->return_stmt = e;
		return self;
	}

	Stmt*
	stmt_if(Expr* if_cond, Stmt* if_body, const mn::Buf<Else_If>& else_ifs, Stmt* else_body)
	{
		auto self = mn::alloc<Stmt>();
		self->kind = Stmt::KIND_IF;
		self->if_stmt.if_cond = if_cond;
		self->if_stmt.if_body = if_body;
		self->if_stmt.else_ifs = else_ifs;
		self->if_stmt.else_body = else_body;
		return self;
	}

	Stmt*
	stmt_for(Stmt* init_stmt, Expr* loop_cond, Stmt* post_stmt, Stmt* loop_body)
	{
		auto self = mn::alloc<Stmt>();
		self->kind = Stmt::KIND_FOR;
		self->for_stmt.init_stmt = init_stmt;
		self->for_stmt.loop_cond = loop_cond;
		self->for_stmt.post_stmt = post_stmt;
		self->for_stmt.loop_body = loop_body;
		return self;
	}

	Stmt*
	stmt_var(Var v)
	{
		auto self = mn::alloc<Stmt>();
		self->kind = Stmt::KIND_VAR;
		self->var_stmt = v;
		return self;
	}

	Stmt*
	stmt_block(const mn::Buf<Stmt*>& stmts)
	{
		auto self = mn::alloc<Stmt>();
		self->kind = Stmt::KIND_BLOCK;
		self->block_stmt = stmts;
		return self;
	}

	Stmt*
	stmt_assign(const mn::Buf<Expr*>& lhs, const Tkn& op, const mn::Buf<Expr*>& rhs)
	{
		auto self = mn::alloc<Stmt>();
		self->kind = Stmt::KIND_ASSIGN;
		self->assign_stmt.lhs = lhs;
		self->assign_stmt.op = op;
		self->assign_stmt.rhs = rhs;
		return self;
	}

	Stmt*
	stmt_expr(Expr* e)
	{
		auto self = mn::alloc<Stmt>();
		self->kind = Stmt::KIND_EXPR;
		self->expr_stmt = e;
		return self;
	}

	Expr*
	stmt_expr_decay(Stmt* expr)
	{
		assert(expr->kind == Stmt::KIND_EXPR);
		Expr* res = expr->expr_stmt;
		mn::free(expr);
		return res;
	}

	void
	stmt_free(Stmt* self)
	{
		switch(self->kind)
		{
		case Stmt::KIND_BREAK:
		case Stmt::KIND_CONTINUE:
			break;
		case Stmt::KIND_RETURN:
			expr_free(self->return_stmt);
			break;
		case Stmt::KIND_IF:
			expr_free(self->if_stmt.if_cond);
			stmt_free(self->if_stmt.if_body);
			for(const Else_If& e: self->if_stmt.else_ifs)
			{
				expr_free(e.cond);
				stmt_free(e.body);
			}
			mn::buf_free(self->if_stmt.else_ifs);
			if(self->if_stmt.else_body)
				stmt_free(self->if_stmt.else_body);
			break;
		case Stmt::KIND_FOR:
			if(self->for_stmt.init_stmt)
				stmt_free(self->for_stmt.init_stmt);
			if(self->for_stmt.loop_cond)
				expr_free(self->for_stmt.loop_cond);
			if(self->for_stmt.post_stmt)
				stmt_free(self->for_stmt.post_stmt);
			stmt_free(self->for_stmt.loop_body);
			break;
		case Stmt::KIND_VAR:
			var_free(self->var_stmt);
			break;
		case Stmt::KIND_ASSIGN:
			destruct(self->assign_stmt.lhs);
			destruct(self->assign_stmt.rhs);
			break;
		case Stmt::KIND_EXPR:
			expr_free(self->expr_stmt);
			break;
		case Stmt::KIND_BLOCK:
			destruct(self->block_stmt);
			break;
		default: assert(false && "unreachable"); break;
		}
		mn::free(self);
	}
}
