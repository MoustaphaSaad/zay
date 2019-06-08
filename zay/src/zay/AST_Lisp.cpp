#include "zay/AST_Lisp.h"

#include <mn/IO.h>

#include <assert.h>

namespace zay
{
	using namespace mn;

	inline static void
	ast_lisp_indent(AST_Lisp& self)
	{
		for(size_t i = 0; i < self.level; ++i)
			vprintf(self.out, "\t");
	}

	inline static void
	ast_lisp_type(AST_Lisp& self, const Type_Sign& type)
	{
		vprintf(self.out, "(type ");
		for(size_t i = 0; i < type.count; ++i)
		{
			switch(type[i].kind)
			{
			case Type_Atom::KIND_NAMED:
				vprintf(self.out, " {}", type[i].named.str);
				break;

			case Type_Atom::KIND_PTR:
				vprintf(self.out, "*");
				break;

			case Type_Atom::KIND_ARRAY:
				vprintf(self.out, "[{}]", type[i].count.str);
				break;

			default:
				assert(false && "unreachable");
				break;
			}
		}
		vprintf(self.out, ")");
	}

	inline static void
	ast_lisp_field(AST_Lisp& self, const Field& field)
	{
		ast_lisp_indent(self);
		vprintf(self.out, "(field ");

		for(size_t i = 0; i < field.ids.count; ++i)
		{
			if(i != 0)
				vprintf(self.out, ", ");
			vprintf(self.out, "{}", field.ids[i].str);
		}

		vprintf(self.out, ": ");

		ast_lisp_type(self, field.type);

		vprintf(self.out, ")");
	}

	inline static void
	ast_lisp_aggregate(AST_Lisp& self, Decl decl)
	{
		ast_lisp_indent(self);

		const char* type;
		switch(decl->kind)
		{
		case IDecl::KIND_STRUCT: type = "struct"; break;
		case IDecl::KIND_UNION: type = "union"; break;
		default: assert(false && "unreachable"); type = "<UNIDENTIFIED>"; break;
		}

		vprintf(self.out, "({}", type);

		const char* name = "<UNNAMED>";
		if(decl->name)
			name = decl->name.str;
		vprintf(self.out, " {}\n", name);

		self.level++;

		for(size_t i = 0; i < decl->struct_decl.count; ++i)
		{
			ast_lisp_field(self, decl->struct_decl[i]);
			vprintf(self.out, "\n");
		}

		self.level--;

		ast_lisp_indent(self);
		vprintf(self.out, ")");
	}

	inline static void
	ast_lisp_enum(AST_Lisp& self, Decl decl)
	{
		ast_lisp_indent(self);

		vprintf(self.out, "(enum {}\n", decl->name.str);

		self.level++;
		for(size_t i = 0; i < decl->enum_decl.count; ++i)
		{
			ast_lisp_indent(self);
			vprintf(self.out, "{}", decl->enum_decl[i].id.str);
			if(decl->enum_decl[i].expr)
			{
				vprintf(self.out , " = ");
				ast_lisp_expr(self, decl->enum_decl[i].expr);
			}
			vprintf(self.out, "\n");
		}
		self.level--;

		ast_lisp_indent(self);
		vprintf(self.out, ")");
	}

	inline static void
	ast_lisp_variable(AST_Lisp& self, const Variable& v)
	{
		ast_lisp_indent(self);
		vprintf(self.out, "(var ");

		for(size_t i = 0; i < v.ids.count; ++i)
		{
			if(i != 0)
				vprintf(self.out, ", ");
			vprintf(self.out, "{}", v.ids[i].str);
		}

		self.level++;

		if(v.type.count != 0)
		{
			vprintf(self.out, "\n");
			ast_lisp_indent(self);
			ast_lisp_type(self, v.type);
		}

		for(size_t i = 0; i < v.exprs.count; ++i)
		{
			if(i == 0)
			{
				vprintf(self.out, "\n");
				ast_lisp_indent(self);
			}
			else
			{
				vprintf(self.out, ", ");
			}
			ast_lisp_expr(self, v.exprs[i]);
		}

		self.level--;

		vprintf(self.out, "\n");
		ast_lisp_indent(self);

		vprintf(self.out, ")");
	}

	inline static void
	ast_lisp_var(AST_Lisp& self, Decl decl)
	{
		ast_lisp_variable(self, decl->var_decl);
	}

	inline static void
	ast_lisp_func(AST_Lisp& self, Decl decl)
	{
		ast_lisp_indent(self);
		vprintf(self.out, "(func {}", decl->name.str);

		self.level++;

		vprintf(self.out, "\n");
		ast_lisp_indent(self);

		//write the args
		for(const Arg& a: decl->func_decl.args)
		{
			for(size_t i = 0; i < a.ids.count; ++i)
			{
				if(i != 0)
					vprintf(self.out, ", ");
				vprintf(self.out, "{}", a.ids[i].str);
			}

			vprintf(self.out, ": ");
			ast_lisp_type(self, a.type);
			vprintf(self.out, " ");
		}

		//write the ret type
		if(decl->func_decl.ret_type.count != 0)
		{
			vprintf(self.out, "\n");
			ast_lisp_indent(self);
			vprintf(self.out, ": ");
			ast_lisp_type(self, decl->func_decl.ret_type);
		}

		if(decl->func_decl.body)
		{
			vprintf(self.out, "\n");
			ast_lisp_stmt(self, decl->func_decl.body);
		}

		self.level--;

		vprintf(self.out, "\n");
		ast_lisp_indent(self);

		vprintf(self.out, ")");
	}

	inline static void
	ast_lisp_atom(AST_Lisp& self, Expr expr)
	{
		vprintf(self.out, "(atom {})", expr->atom.str);
	}

	inline static void
	ast_lisp_binary(AST_Lisp& self, Expr expr)
	{
		vprintf(self.out, "(binary {} ", expr->binary.op.str);
		ast_lisp_expr(self, expr->binary.lhs);

		vprintf(self.out, " ");

		ast_lisp_expr(self, expr->binary.rhs);

		vprintf(self.out, ")");
	}

	inline static void
	ast_lisp_unary(AST_Lisp& self, Expr expr)
	{
		vprintf(self.out, "(unary {} ", expr->unary.op.str);
		ast_lisp_expr(self, expr->unary.expr);
		vprintf(self.out, ")");
	}

	inline static void
	ast_lisp_dot(AST_Lisp& self, Expr expr)
	{
		vprintf(self.out, "(dot ");
		ast_lisp_expr(self, expr->dot.base);
		vprintf(self.out, ".{})", expr->dot.member.str);
	}

	inline static void
	ast_lisp_indexed(AST_Lisp& self, Expr expr)
	{
		vprintf(self.out, "(indexed ");
		ast_lisp_expr(self, expr->indexed.base);
		vprintf(self.out, "[");
		ast_lisp_expr(self, expr->indexed.index);
		vprintf(self.out, "])");
	}

	inline static void
	ast_lisp_call(AST_Lisp& self, Expr expr)
	{
		vprintf(self.out, "(call ");
		ast_lisp_expr(self, expr->call.base);
		vprintf(self.out, " ");
		for(size_t i = 0; i < expr->call.args.count; ++i)
		{
			if(i != 0)
				vprintf(self.out, ", ");
			ast_lisp_expr(self, expr->call.args[i]);
		}
		vprintf(self.out, ")");
	}

	inline static void
	ast_lisp_cast(AST_Lisp& self, Expr expr)
	{
		vprintf(self.out, "(cast ");
		ast_lisp_expr(self, expr->cast.base);
		ast_lisp_type(self, expr->cast.type);
		vprintf(self.out, ")");
	}

	inline static void
	ast_lisp_paren(AST_Lisp& self, Expr expr)
	{
		vprintf(self.out, "(paren ");
		ast_lisp_expr(self, expr->paren);
		vprintf(self.out, ")");
	}

	inline static void
	ast_lisp_stmt_break(AST_Lisp& self, Stmt stmt)
	{
		ast_lisp_indent(self);
		vprintf(self.out, "({})", stmt->break_stmt.str);
	}

	inline static void
	ast_lisp_stmt_continue(AST_Lisp& self, Stmt stmt)
	{
		ast_lisp_indent(self);
		vprintf(self.out, "({})", stmt->continue_stmt.str);
	}

	inline static void
	ast_lisp_stmt_return(AST_Lisp& self, Stmt stmt)
	{
		ast_lisp_indent(self);
		vprintf(self.out, "(return ");

		ast_lisp_expr(self, stmt->return_stmt);
		vprintf(self.out, ")");
	}

	inline static void
	ast_lisp_stmt_if(AST_Lisp& self, Stmt stmt)
	{
		ast_lisp_indent(self);
		vprintf(self.out, "(if ");
		ast_lisp_expr(self, stmt->if_stmt.if_cond);

		self.level++;

		vprintf(self.out, "\n");
		ast_lisp_stmt(self, stmt->if_stmt.if_body);

		for(const Else_If& e: stmt->if_stmt.else_ifs)
		{
			vprintf(self.out, "\n");
			ast_lisp_indent(self);
			ast_lisp_expr(self, e.cond);

			vprintf(self.out, "\n");
			ast_lisp_stmt(self, e.body);
		}

		vprintf(self.out, "\n");
		ast_lisp_stmt(self, stmt->if_stmt.else_body);

		self.level--;

		vprintf(self.out, "\n");
		ast_lisp_indent(self);

		vprintf(self.out, ")");
	}

	inline static void
	ast_lisp_stmt_for(AST_Lisp& self, Stmt stmt)
	{
		ast_lisp_indent(self);
		vprintf(self.out, "(for");

		self.level++;

		if(stmt->for_stmt.init_stmt)
		{
			vprintf(self.out, "\n");
			ast_lisp_stmt(self, stmt->for_stmt.init_stmt);
		}

		if(stmt->for_stmt.loop_cond)
		{
			vprintf(self.out, "\n");
			ast_lisp_indent(self);
			ast_lisp_expr(self, stmt->for_stmt.loop_cond);
		}

		if(stmt->for_stmt.post_stmt)
		{
			vprintf(self.out, "\n");
			ast_lisp_stmt(self, stmt->for_stmt.post_stmt);
		}

		vprintf(self.out, "\n");
		ast_lisp_stmt(self, stmt->for_stmt.loop_body);

		self.level--;

		vprintf(self.out, "\n");
		ast_lisp_indent(self);

		vprintf(self.out, ")");
	}

	inline static void
	ast_lisp_stmt_var(AST_Lisp& self, Stmt stmt)
	{
		ast_lisp_variable(self, stmt->var_stmt);
	}

	inline static void
	ast_lisp_stmt_assign(AST_Lisp& self, Stmt stmt)
	{
		ast_lisp_indent(self);
		vprintf(self.out, "({} ", stmt->assign_stmt.op.str);

		self.level++;

		for(size_t i = 0; i < stmt->assign_stmt.lhs.count; ++i)
		{
			if(i == 0)
			{
				vprintf(self.out, "\n");
				ast_lisp_indent(self);
			}
			else
			{
				vprintf(self.out, ", ");
			}
			ast_lisp_expr(self, stmt->assign_stmt.lhs[i]);
		}

		for(size_t i = 0; i < stmt->assign_stmt.rhs.count; ++i)
		{
			if(i == 0)
			{
				vprintf(self.out, "\n");
				ast_lisp_indent(self);
			}
			else
			{
				vprintf(self.out, ", ");
			}
			ast_lisp_expr(self, stmt->assign_stmt.rhs[i]);
		}

		self.level--;

		vprintf(self.out, "\n");
		ast_lisp_indent(self);

		vprintf(self.out, ")");
	}

	inline static void
	ast_lisp_stmt_expr(AST_Lisp& self, Stmt stmt)
	{
		ast_lisp_indent(self);
		vprintf(self.out, "(expr-stmt");

		self.level++;

		vprintf(self.out, "\n");
		ast_lisp_indent(self);
		ast_lisp_expr(self, stmt->expr_stmt);

		self.level--;

		vprintf(self.out, "\n");
		ast_lisp_indent(self);

		vprintf(self.out, ")");
	}

	inline static void
	ast_lisp_stmt_block(AST_Lisp& self, Stmt stmt)
	{
		ast_lisp_indent(self);
		vprintf(self.out, "(block-stmt");

		self.level++;
		for(size_t i = 0; i < stmt->block_stmt.count; ++i)
		{
			vprintf(self.out, "\n");
			ast_lisp_stmt(self, stmt->block_stmt[i]);
		}
		self.level--;

		vprintf(self.out, "\n");
		ast_lisp_indent(self);

		vprintf(self.out, ")");
	}


	//API
	void
	ast_lisp_expr(AST_Lisp& self, Expr expr)
	{
		switch(expr->kind)
		{
		case IExpr::KIND_ATOM: ast_lisp_atom(self, expr); break;
		case IExpr::KIND_BINARY: ast_lisp_binary(self, expr); break;
		case IExpr::KIND_UNARY: ast_lisp_unary(self, expr); break;
		case IExpr::KIND_DOT: ast_lisp_dot(self, expr); break;
		case IExpr::KIND_INDEXED: ast_lisp_indexed(self, expr); break;
		case IExpr::KIND_CALL: ast_lisp_call(self, expr); break;
		case IExpr::KIND_CAST: ast_lisp_cast(self, expr); break;
		case IExpr::KIND_PAREN: ast_lisp_paren(self, expr); break;
		default: assert(false && "unreachable"); break;
		}
	}

	void
	ast_lisp_stmt(AST_Lisp& self, Stmt stmt)
	{
		switch(stmt->kind)
		{
		case IStmt::KIND_BREAK: ast_lisp_stmt_break(self, stmt); break;
		case IStmt::KIND_CONTINUE: ast_lisp_stmt_continue(self, stmt); break;
		case IStmt::KIND_RETURN: ast_lisp_stmt_return(self, stmt); break;
		case IStmt::KIND_IF: ast_lisp_stmt_if(self, stmt); break;
		case IStmt::KIND_FOR: ast_lisp_stmt_for(self, stmt); break;
		case IStmt::KIND_VAR: ast_lisp_stmt_var(self, stmt); break;
		case IStmt::KIND_ASSIGN: ast_lisp_stmt_assign(self, stmt); break;
		case IStmt::KIND_EXPR: ast_lisp_stmt_expr(self, stmt); break;
		case IStmt::KIND_BLOCK: ast_lisp_stmt_block(self, stmt); break;
		default: assert(false && "unreachable"); break;
		}
	}

	void
	ast_lisp_decl(AST_Lisp& self, Decl decl)
	{
		switch(decl->kind)
		{
		case IDecl::KIND_STRUCT:
		case IDecl::KIND_UNION:
			ast_lisp_aggregate(self, decl);
			break;

		case IDecl::KIND_ENUM:
			ast_lisp_enum(self, decl);
			break;

		case IDecl::KIND_VAR:
			ast_lisp_var(self, decl);
			break;

		case IDecl::KIND_FUNC:
			ast_lisp_func(self, decl);
			break;

		default: assert(false && "unreachable"); break;
		}
	}
}