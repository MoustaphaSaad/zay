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
		self->cast.to_type = nullptr;
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


	Stmt
	stmt_break(const Tkn& t)
	{
		Stmt self = alloc<IStmt>();
		self->kind = IStmt::KIND_BREAK;
		self->break_stmt = t;
		return self;
	}

	Stmt
	stmt_continue(const Tkn& t)
	{
		Stmt self = alloc<IStmt>();
		self->kind = IStmt::KIND_CONTINUE;
		self->continue_stmt = t;
		return self;
	}

	Stmt
	stmt_return(Expr e)
	{
		Stmt self = alloc<IStmt>();
		self->kind = IStmt::KIND_RETURN;
		self->return_stmt = e;
		return self;
	}

	Stmt
	stmt_if(Expr if_cond, Stmt if_body, const mn::Buf<Else_If>& else_ifs, Stmt else_body)
	{
		Stmt self = alloc<IStmt>();
		self->kind = IStmt::KIND_IF;
		self->if_stmt.if_cond = if_cond;
		self->if_stmt.if_body = if_body;
		self->if_stmt.else_ifs = else_ifs;
		self->if_stmt.else_body = else_body;
		return self;
	}

	Stmt
	stmt_for(Stmt init_stmt, Expr loop_cond, Stmt post_stmt, Stmt loop_body)
	{
		Stmt self = alloc<IStmt>();
		self->kind = IStmt::KIND_FOR;
		self->for_stmt.init_stmt = init_stmt;
		self->for_stmt.loop_cond = loop_cond;
		self->for_stmt.post_stmt = post_stmt;
		self->for_stmt.loop_body = loop_body;
		return self;
	}

	Stmt
	stmt_var(Variable v)
	{
		Stmt self = alloc<IStmt>();
		self->kind = IStmt::KIND_VAR;
		self->var_stmt = v;
		return self;
	}

	Stmt
	stmt_block(const mn::Buf<Stmt>& stmts)
	{
		Stmt self = alloc<IStmt>();
		self->kind = IStmt::KIND_BLOCK;
		self->block_stmt = stmts;
		return self;
	}

	Stmt
	stmt_assign(const mn::Buf<Expr>& lhs, const Tkn& op, const mn::Buf<Expr>& rhs)
	{
		Stmt self = alloc<IStmt>();
		self->kind = IStmt::KIND_ASSIGN;
		self->assign_stmt.lhs = lhs;
		self->assign_stmt.op = op;
		self->assign_stmt.rhs = rhs;
		return self;
	}

	Stmt
	stmt_expr(Expr e)
	{
		Stmt self = alloc<IStmt>();
		self->kind = IStmt::KIND_EXPR;
		self->expr_stmt = e;
		return self;
	}

	Expr
	stmt_expr_decay(Stmt expr)
	{
		assert(expr->kind == IStmt::KIND_EXPR);
		Expr res = expr->expr_stmt;
		free(expr);
		return res;
	}

	void
	stmt_free(Stmt self)
	{
		switch(self->kind)
		{
		case IStmt::KIND_BREAK:
		case IStmt::KIND_CONTINUE:
			break;
		case IStmt::KIND_RETURN:
			expr_free(self->return_stmt);
			break;
		case IStmt::KIND_IF:
			expr_free(self->if_stmt.if_cond);
			stmt_free(self->if_stmt.if_body);
			for(const Else_If& e: self->if_stmt.else_ifs)
			{
				expr_free(e.cond);
				stmt_free(e.body);
			}
			buf_free(self->if_stmt.else_ifs);
			if(self->if_stmt.else_body)
				stmt_free(self->if_stmt.else_body);
			break;
		case IStmt::KIND_FOR:
			if(self->for_stmt.init_stmt)
				stmt_free(self->for_stmt.init_stmt);
			if(self->for_stmt.loop_cond)
				expr_free(self->for_stmt.loop_cond);
			if(self->for_stmt.post_stmt)
				stmt_free(self->for_stmt.post_stmt);
			stmt_free(self->for_stmt.loop_body);
			break;
		case IStmt::KIND_VAR:
			variable_free(self->var_stmt);
			break;
		case IStmt::KIND_ASSIGN:
			destruct(self->assign_stmt.lhs);
			destruct(self->assign_stmt.rhs);
			break;
		case IStmt::KIND_EXPR:
			expr_free(self->expr_stmt);
			break;
		case IStmt::KIND_BLOCK:
			destruct(self->block_stmt);
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

	Decl
	decl_enum(const Tkn& name, const mn::Buf<Enum_Field>& fields)
	{
		Decl self = alloc<IDecl>();
		self->kind = IDecl::KIND_ENUM;
		self->name = name;
		self->enum_decl = fields;
		return self;
	}

	Decl
	decl_var(const Variable& v)
	{
		Decl self = alloc<IDecl>();
		self->kind = IDecl::KIND_VAR;
		//let's put the first id as the name of this declaration
		if(v.ids.count > 0)
			self->name = v.ids[0];
		self->var_decl = v;
		return self;
	}

	Decl
	decl_func(const Tkn& name, const Buf<Arg>& args, const Type_Sign& ret_type, Stmt body)
	{
		Decl self = alloc<IDecl>();
		self->kind = IDecl::KIND_FUNC;
		self->name = name;
		self->func_decl.args = args;
		self->func_decl.ret_type = ret_type;
		self->func_decl.body = body;
		return self;
	}

	void
	decl_free(Decl self)
	{
		switch(self->kind)
		{
		case IDecl::KIND_STRUCT: destruct(self->struct_decl); break;
		case IDecl::KIND_UNION: destruct(self->union_decl); break;
		case IDecl::KIND_ENUM:
			for(const Enum_Field& e: self->enum_decl)
				if(e.expr)
					expr_free(e.expr);
			buf_free(self->enum_decl);
			break;
		case IDecl::KIND_VAR:
			variable_free(self->var_decl);
			break;
		case IDecl::KIND_FUNC:
			destruct(self->func_decl.args);
			type_sign_free(self->func_decl.ret_type);
			if(self->func_decl.body)
				stmt_free(self->func_decl.body);
			break;
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