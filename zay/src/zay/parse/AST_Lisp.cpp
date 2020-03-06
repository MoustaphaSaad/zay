#include "zay/parse/AST_Lisp.h"

#include <mn/IO.h>

#include <assert.h>

namespace zay
{
	inline static void
	ast_lisp_field(AST_Lisp& self, const Field& field);

	inline static void
	ast_lisp_indent(AST_Lisp& self)
	{
		for(size_t i = 0; i < self.level; ++i)
			mn::print_to(self.out, "\t");
	}

	inline static void
	ast_lisp_type_sign(AST_Lisp& self, const Type_Sign& type)
	{
		mn::print_to(self.out, "(type-sign ");
		for(size_t i = 0; i < type.count; ++i)
		{
			switch(type[i].kind)
			{
			case Type_Atom::KIND_NAMED:
				mn::print_to(self.out, " {}", type[i].named.str);
				break;

			case Type_Atom::KIND_PTR:
				mn::print_to(self.out, "*");
				break;

			case Type_Atom::KIND_ARRAY:
				mn::print_to(self.out, "[{}]", type[i].count.str);
				break;

			case Type_Atom::KIND_STRUCT:
				mn::print_to(self.out, "(struct\n");
				self.level++;
				for (const Field& field : type[i].struct_fields)
				{
					ast_lisp_indent(self);
					ast_lisp_field(self, field);
					mn::print_to(self.out, "\n");
				}
				self.level--;
				ast_lisp_indent(self);
				mn::print_to(self.out, ")");
				break;

			case Type_Atom::KIND_UNION:
				mn::print_to(self.out, "(union\n");
				self.level++;
				for (const Field& field : type[i].union_fields)
				{
					ast_lisp_indent(self);
					ast_lisp_field(self, field);
					mn::print_to(self.out, "\n");
				}
				self.level--;
				ast_lisp_indent(self);
				mn::print_to(self.out, ")");
				break;

			case Type_Atom::KIND_ENUM:
				mn::print_to(self.out, "(enum\n");
				self.level++;
				for (const Enum_Field& field : type[i].enum_fields)
				{
					ast_lisp_indent(self);
					mn::print_to(self.out, "(field {}", field.id.str);
					if (field.expr)
					{
						mn::print_to(self.out, " ");
						ast_lisp_expr(self, field.expr);
					}
					mn::print_to(self.out, ")");
					mn::print_to(self.out, "\n");
				}
				self.level--;
				ast_lisp_indent(self);
				mn::print_to(self.out, ")");
				break;

			default:
				assert(false && "unreachable");
				break;
			}
		}
		mn::print_to(self.out, ")");
	}

	inline static void
	ast_lisp_field(AST_Lisp& self, const Field& field)
	{
		mn::print_to(self.out, "(field ");

		for(size_t i = 0; i < field.ids.count; ++i)
		{
			if(i != 0)
				mn::print_to(self.out, ", ");
			mn::print_to(self.out, "{}", field.ids[i].str);
		}

		mn::print_to(self.out, ": ");

		ast_lisp_type_sign(self, field.type);

		mn::print_to(self.out, ")");
	}

	inline static void
	ast_lisp_variable(AST_Lisp& self, const Var& v)
	{
		ast_lisp_indent(self);
		mn::print_to(self.out, "(var ");

		for(size_t i = 0; i < v.ids.count; ++i)
		{
			if(i != 0)
				mn::print_to(self.out, ", ");
			mn::print_to(self.out, "{}", v.ids[i].str);
		}

		self.level++;

		if(v.type.count != 0)
		{
			mn::print_to(self.out, "\n");
			ast_lisp_indent(self);
			ast_lisp_type_sign(self, v.type);
		}

		for(size_t i = 0; i < v.exprs.count; ++i)
		{
			if(i == 0)
			{
				mn::print_to(self.out, "\n");
				ast_lisp_indent(self);
			}
			else
			{
				mn::print_to(self.out, ", ");
			}
			ast_lisp_expr(self, v.exprs[i]);
		}

		self.level--;

		mn::print_to(self.out, "\n");
		ast_lisp_indent(self);

		mn::print_to(self.out, ")");
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
		mn::print_to(self.out, "(func {}", decl->name.str);

		self.level++;

		mn::print_to(self.out, "\n");
		ast_lisp_indent(self);

		//write the args
		for(const Arg& a: decl->func_decl.args)
		{
			for(size_t i = 0; i < a.ids.count; ++i)
			{
				if(i != 0)
					mn::print_to(self.out, ", ");
				mn::print_to(self.out, "{}", a.ids[i].str);
			}

			mn::print_to(self.out, ": ");
			ast_lisp_type_sign(self, a.type);
			mn::print_to(self.out, " ");
		}

		//write the ret type
		if(decl->func_decl.ret_type.count != 0)
		{
			mn::print_to(self.out, "\n");
			ast_lisp_indent(self);
			mn::print_to(self.out, ": ");
			ast_lisp_type_sign(self, decl->func_decl.ret_type);
		}

		if(decl->func_decl.body)
		{
			mn::print_to(self.out, "\n");
			ast_lisp_stmt(self, decl->func_decl.body);
		}

		self.level--;

		mn::print_to(self.out, "\n");
		ast_lisp_indent(self);

		mn::print_to(self.out, ")");
	}

	inline static void
	ast_lisp_type_decl(AST_Lisp& self, Decl decl)
	{
		ast_lisp_indent(self);

		mn::print_to(self.out, "(type-decl {}\n", decl->name.str);

		self.level++;
		ast_lisp_indent(self);

		ast_lisp_type_sign(self, decl->type_decl);

		mn::print_to(self.out, "\n");
		self.level--;
		ast_lisp_indent(self);
		mn::print_to(self.out, ")");
	}


	inline static void
	ast_lisp_atom(AST_Lisp& self, Expr expr)
	{
		mn::print_to(self.out, "(atom {})", expr->atom.str);
	}

	inline static void
	ast_lisp_binary(AST_Lisp& self, Expr expr)
	{
		mn::print_to(self.out, "(binary {} ", expr->binary.op.str);
		ast_lisp_expr(self, expr->binary.lhs);

		mn::print_to(self.out, " ");

		ast_lisp_expr(self, expr->binary.rhs);

		mn::print_to(self.out, ")");
	}

	inline static void
	ast_lisp_unary(AST_Lisp& self, Expr expr)
	{
		mn::print_to(self.out, "(unary {} ", expr->unary.op.str);
		ast_lisp_expr(self, expr->unary.expr);
		mn::print_to(self.out, ")");
	}

	inline static void
	ast_lisp_dot(AST_Lisp& self, Expr expr)
	{
		mn::print_to(self.out, "(dot ");
		ast_lisp_expr(self, expr->dot.base);
		mn::print_to(self.out, ".{})", expr->dot.member.str);
	}

	inline static void
	ast_lisp_indexed(AST_Lisp& self, Expr expr)
	{
		mn::print_to(self.out, "(indexed ");
		ast_lisp_expr(self, expr->indexed.base);
		mn::print_to(self.out, "[");
		ast_lisp_expr(self, expr->indexed.index);
		mn::print_to(self.out, "])");
	}

	inline static void
	ast_lisp_call(AST_Lisp& self, Expr expr)
	{
		mn::print_to(self.out, "(call ");
		ast_lisp_expr(self, expr->call.base);
		mn::print_to(self.out, " ");
		for(size_t i = 0; i < expr->call.args.count; ++i)
		{
			if(i != 0)
				mn::print_to(self.out, ", ");
			ast_lisp_expr(self, expr->call.args[i]);
		}
		mn::print_to(self.out, ")");
	}

	inline static void
	ast_lisp_cast(AST_Lisp& self, Expr expr)
	{
		mn::print_to(self.out, "(cast ");
		ast_lisp_expr(self, expr->cast.base);
		ast_lisp_type_sign(self, expr->cast.type);
		mn::print_to(self.out, ")");
	}

	inline static void
	ast_lisp_paren(AST_Lisp& self, Expr expr)
	{
		mn::print_to(self.out, "(paren ");
		ast_lisp_expr(self, expr->paren);
		mn::print_to(self.out, ")");
	}

	inline static void
	ast_lisp_complit(AST_Lisp& self, Expr expr)
	{
		mn::print_to(self.out, "(complit ");
		ast_lisp_type_sign(self, expr->complit.type);
		mn::print_to(self.out, "\n");
		self.level++;
		for(const Complit_Field& f: expr->complit.fields)
		{
			ast_lisp_indent(self);
			ast_lisp_expr(self, f.left);
			mn::print_to(self.out, " ");
			ast_lisp_expr(self, f.right);
			mn::print_to(self.out, "\n");
		}
		
		self.level--;
		ast_lisp_indent(self);
		mn::print_to(self.out, ")");
	}



	inline static void
	ast_lisp_stmt_break(AST_Lisp& self, Stmt stmt)
	{
		ast_lisp_indent(self);
		mn::print_to(self.out, "({})", stmt->break_stmt.str);
	}

	inline static void
	ast_lisp_stmt_continue(AST_Lisp& self, Stmt stmt)
	{
		ast_lisp_indent(self);
		mn::print_to(self.out, "({})", stmt->continue_stmt.str);
	}

	inline static void
	ast_lisp_stmt_return(AST_Lisp& self, Stmt stmt)
	{
		ast_lisp_indent(self);
		mn::print_to(self.out, "(return ");

		ast_lisp_expr(self, stmt->return_stmt);
		mn::print_to(self.out, ")");
	}

	inline static void
	ast_lisp_stmt_if(AST_Lisp& self, Stmt stmt)
	{
		ast_lisp_indent(self);
		mn::print_to(self.out, "(if ");
		ast_lisp_expr(self, stmt->if_stmt.if_cond);

		self.level++;

		mn::print_to(self.out, "\n");
		ast_lisp_stmt(self, stmt->if_stmt.if_body);

		for(const Else_If& e: stmt->if_stmt.else_ifs)
		{
			mn::print_to(self.out, "\n");
			ast_lisp_indent(self);
			ast_lisp_expr(self, e.cond);

			mn::print_to(self.out, "\n");
			ast_lisp_stmt(self, e.body);
		}

		mn::print_to(self.out, "\n");
		if(stmt->if_stmt.else_body)
			ast_lisp_stmt(self, stmt->if_stmt.else_body);

		self.level--;

		mn::print_to(self.out, "\n");
		ast_lisp_indent(self);

		mn::print_to(self.out, ")");
	}

	inline static void
	ast_lisp_stmt_for(AST_Lisp& self, Stmt stmt)
	{
		ast_lisp_indent(self);
		mn::print_to(self.out, "(for");

		self.level++;

		if(stmt->for_stmt.init_stmt)
		{
			mn::print_to(self.out, "\n");
			ast_lisp_stmt(self, stmt->for_stmt.init_stmt);
		}

		if(stmt->for_stmt.loop_cond)
		{
			mn::print_to(self.out, "\n");
			ast_lisp_indent(self);
			ast_lisp_expr(self, stmt->for_stmt.loop_cond);
		}

		if(stmt->for_stmt.post_stmt)
		{
			mn::print_to(self.out, "\n");
			ast_lisp_stmt(self, stmt->for_stmt.post_stmt);
		}

		mn::print_to(self.out, "\n");
		ast_lisp_stmt(self, stmt->for_stmt.loop_body);

		self.level--;

		mn::print_to(self.out, "\n");
		ast_lisp_indent(self);

		mn::print_to(self.out, ")");
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
		mn::print_to(self.out, "({} ", stmt->assign_stmt.op.str);

		self.level++;

		for(size_t i = 0; i < stmt->assign_stmt.lhs.count; ++i)
		{
			if(i == 0)
			{
				mn::print_to(self.out, "\n");
				ast_lisp_indent(self);
			}
			else
			{
				mn::print_to(self.out, ", ");
			}
			ast_lisp_expr(self, stmt->assign_stmt.lhs[i]);
		}

		for(size_t i = 0; i < stmt->assign_stmt.rhs.count; ++i)
		{
			if(i == 0)
			{
				mn::print_to(self.out, "\n");
				ast_lisp_indent(self);
			}
			else
			{
				mn::print_to(self.out, ", ");
			}
			ast_lisp_expr(self, stmt->assign_stmt.rhs[i]);
		}

		self.level--;

		mn::print_to(self.out, "\n");
		ast_lisp_indent(self);

		mn::print_to(self.out, ")");
	}

	inline static void
	ast_lisp_stmt_expr(AST_Lisp& self, Stmt stmt)
	{
		ast_lisp_indent(self);
		mn::print_to(self.out, "(expr-stmt");

		self.level++;

		mn::print_to(self.out, "\n");
		ast_lisp_indent(self);
		ast_lisp_expr(self, stmt->expr_stmt);

		self.level--;

		mn::print_to(self.out, "\n");
		ast_lisp_indent(self);

		mn::print_to(self.out, ")");
	}

	inline static void
	ast_lisp_stmt_block(AST_Lisp& self, Stmt stmt)
	{
		ast_lisp_indent(self);
		mn::print_to(self.out, "(block-stmt");

		self.level++;
		for(size_t i = 0; i < stmt->block_stmt.count; ++i)
		{
			mn::print_to(self.out, "\n");
			ast_lisp_stmt(self, stmt->block_stmt[i]);
		}
		self.level--;

		mn::print_to(self.out, "\n");
		ast_lisp_indent(self);

		mn::print_to(self.out, ")");
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
		case IExpr::KIND_COMPLIT: ast_lisp_complit(self, expr); break;
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
		case IDecl::KIND_VAR:
			ast_lisp_var(self, decl);
			break;

		case IDecl::KIND_FUNC:
			ast_lisp_func(self, decl);
			break;

		case IDecl::KIND_TYPE:
			ast_lisp_type_decl(self, decl);
			break;

		default: assert(false && "unreachable"); break;
		}
	}
}