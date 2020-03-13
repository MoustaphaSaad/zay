#include "zay/CGen.h"

#include <mn/Memory.h>

#include <assert.h>

namespace zay
{
	inline static void
	cgen_expr_gen(CGen self, Expr* expr);

	inline static void
	cgen_indent(CGen self)
	{
		for (size_t i = 0; i < self->indent; ++i)
			mn::print_to(self->out, "\t");
	}

	inline static void
	cgen_newline(CGen self)
	{
		mn::print_to(self->out, "\n");
		cgen_indent(self);
	}

	inline static Scope*
	cgen_scope(CGen self)
	{
		return buf_top(self->scope_stack);
	}

	inline static void
	cgen_scope_enter(CGen self, Scope* scope)
	{
		buf_push(self->scope_stack, scope);
	}

	inline static void
	cgen_scope_leave(CGen self)
	{
		buf_pop(self->scope_stack);
	}

	inline static Sym*
	cgen_sym(CGen self, const char* name)
	{
		return scope_find(cgen_scope(self), name);
	}

	inline static mn::Str
	cdecl_paren(mn::Str str, mn::Rune c)
	{
		if (c && c != '[')
			return mn::str_tmpf("({})", str);
		return str;
	}

	inline static mn::Str
	cgen_write_field(CGen self, Type* type, const mn::Str& name)
	{
		mn::Rune c = 0;
		if (name.count > 0)
			c = mn::rune_read(name.ptr);

		switch(type->kind)
		{
		case Type::KIND_PTR:
			return cgen_write_field(self, type->ptr.base, cdecl_paren(mn::str_tmpf("*{}", name), c));
		case Type::KIND_ARRAY:
			return cgen_write_field(self, type->array.base, cdecl_paren(mn::str_tmpf("{}[{}]", name, type->array.count), c));
		case Type::KIND_FUNC:
		{
			mn::Str res = mn::str_tmpf("(*{})(", name);
			if(type->func.args.count == 0)
			{
				mn::str_push(res, "ZayVoid");
			}
			else
			{
				for(size_t i = 0; i < type->func.args.count; ++i)
				{
					if (i != 0)
						res = mn::strf(res, ", ");
					res = mn::strf(res, "{}", cgen_write_field(self, type->func.args[i], mn::str_lit("")));
				}
			}
			res = mn::strf(res, ")");
			return cgen_write_field(self, type->func.ret, res);
		}
		case Type::KIND_VOID:
			if (name.count)
				return mn::str_tmpf("void {}", name);
			else
				return mn::str_tmpf("void");
		case Type::KIND_BOOL:
			if (name.count)
				return mn::str_tmpf("bool {}", name);
			else
				return mn::str_tmpf("bool");
		case Type::KIND_INT:
			if (name.count)
				return mn::str_tmpf("ZayInt {}", name);
			else
				return mn::str_tmpf("ZayInt");
		case Type::KIND_UINT:
			if (name.count)
				return mn::str_tmpf("ZayUint {}", name);
			else
				return mn::str_tmpf("ZayUint");
		case Type::KIND_INT8:
			if (name.count)
				return mn::str_tmpf("int8_t {}", name);
			else
				return mn::str_tmpf("int8_t");
		case Type::KIND_UINT8:
			if (name.count)
				return mn::str_tmpf("uint8_t {}", name);
			else
				return mn::str_tmpf("uint8_t");
		case Type::KIND_INT16:
			if (name.count)
				return mn::str_tmpf("int16_t {}", name);
			else
				return mn::str_tmpf("int16_t");
		case Type::KIND_UINT16:
			if (name.count)
				return mn::str_tmpf("uint16_t {}", name);
			else
				return mn::str_tmpf("uint16_t");
		case Type::KIND_INT32:
			if (name.count)
				return mn::str_tmpf("int32_t {}", name);
			else
				return mn::str_tmpf("int32_t");
		case Type::KIND_UINT32:
			if (name.count)
				return mn::str_tmpf("uint32_t {}", name);
			else
				return mn::str_tmpf("uint32_t");
		case Type::KIND_INT64:
			if (name.count)
				return mn::str_tmpf("int64_t {}", name);
			else
				return mn::str_tmpf("int64_t");
		case Type::KIND_UINT64:
			if (name.count)
				return mn::str_tmpf("uint64_t {}", name);
			else
				return mn::str_tmpf("uint64_t");
		case Type::KIND_FLOAT32:
			if (name.count)
				return mn::str_tmpf("float {}", name);
			else
				return mn::str_tmpf("float");
		case Type::KIND_FLOAT64:
			if (name.count)
				return mn::str_tmpf("double {}", name);
			else
				return mn::str_tmpf("double");
		case Type::KIND_STRING:
			if (name.count)
				return mn::str_tmpf("ZayString {}", name);
			else
				return mn::str_tmpf("ZayString");
		case Type::KIND_STRUCT:
		case Type::KIND_UNION:
		case Type::KIND_ENUM:
			if (name.count)
				return mn::str_tmpf("{} {}", type->sym->name, name);
			else
				return mn::str_tmpf("{}", type->sym->name);
		case Type::KIND_ALIAS:
			return cgen_write_field(self, type->alias, name);
		default:
			return mn::str_tmpf("<UNDEFINED TYPE>");
		}
	}

	inline static mn::Str
	cgen_write_field(CGen self, Type* type, const char* name)
	{
		return cgen_write_field(self, type, mn::str_lit(name));
	}

	inline static void
	cgen_write_type(CGen self, Type* type, const char* type_name, const mn::Str& name)
	{
		auto c = 0;
		if (name.count > 0)
			c = mn::rune_read(name.ptr);

		switch(type->kind)
		{
		case Type::KIND_PTR:
			cgen_write_type(self, type->ptr.base, type_name, cdecl_paren(mn::str_tmpf("*{}", name), c));
			break;
		case Type::KIND_ARRAY:
			cgen_write_type(self, type->array.base, type_name, cdecl_paren(mn::str_tmpf("{}[{}]", name, type->array.count), c));
			break;
		case Type::KIND_FUNC:
		{
			auto res = mn::str_tmpf("(*{})(", name);
			if(type->func.args.count == 0)
			{
				mn::str_push(res, "ZayVoid");
			}
			else
			{
				for(size_t i = 0; i < type->func.args.count; ++i)
				{
					if (i != 0)
						res = mn::strf(res, ", ");
					res = mn::strf(res, "{}", cgen_write_field(self, type->func.args[i], mn::str_lit("")));
				}
			}
			res = mn::strf(res, ")");
			mn::print_to(self->out, "{}", cgen_write_field(self, type->func.ret, res));
			break;
		}
		case Type::KIND_VOID:
			if (name.count)
				mn::print_to(self->out, "void {}", name);
			else
				mn::print_to(self->out, "void");
			break;
		case Type::KIND_BOOL:
			if (name.count)
				mn::print_to(self->out, "bool {}", name);
			else
				mn::print_to(self->out, "bool");
			break;
		case Type::KIND_INT:
			if (name.count)
				mn::print_to(self->out, "ZayInt {}", name);
			else
				mn::print_to(self->out, "ZayInt");
			break;
		case Type::KIND_UINT:
			if (name.count)
				mn::print_to(self->out, "ZayUint {}", name);
			else
				mn::print_to(self->out, "ZayUint");
			break;
		case Type::KIND_INT8:
			if (name.count)
				mn::print_to(self->out, "int8_t {}", name);
			else
				mn::print_to(self->out, "int8_t");
			break;
		case Type::KIND_UINT8:
			if (name.count)
				mn::print_to(self->out, "uint8_t {}", name);
			else
				mn::print_to(self->out, "uint8_t");
			break;
		case Type::KIND_INT16:
			if (name.count)
				mn::print_to(self->out, "int16_t {}", name);
			else
				mn::print_to(self->out, "int16_t");
			break;
		case Type::KIND_UINT16:
			if (name.count)
				mn::print_to(self->out, "uint16_t {}", name);
			else
				mn::print_to(self->out, "uint16_t");
			break;
		case Type::KIND_INT32:
			if (name.count)
				mn::print_to(self->out, "int32_t {}", name);
			else
				mn::print_to(self->out, "int32_t");
			break;
		case Type::KIND_UINT32:
			if (name.count)
				mn::print_to(self->out, "uint32_t {}", name);
			else
				mn::print_to(self->out, "uint32_t");
			break;
		case Type::KIND_INT64:
			if (name.count)
				mn::print_to(self->out, "int64_t {}", name);
			else
				mn::print_to(self->out, "int64_t");
			break;
		case Type::KIND_UINT64:
			if (name.count)
				mn::print_to(self->out, "uint64_t {}", name);
			else
				mn::print_to(self->out, "uint64_t");
			break;
		case Type::KIND_FLOAT32:
			if (name.count)
				mn::print_to(self->out, "float {}", name);
			else
				mn::print_to(self->out, "float");
			break;
		case Type::KIND_FLOAT64:
			if (name.count)
				mn::print_to(self->out, "double {}", name);
			else
				mn::print_to(self->out, "double");
			break;
		case Type::KIND_STRING:
			if (name.count)
				mn::print_to(self->out, "ZayString {}", name);
			else
				mn::print_to(self->out, "ZayString");
			break;
		case Type::KIND_STRUCT:
		{
			mn::print_to(self->out, "struct {} {{", type_name);
			self->indent++;
			for(Field_Sign& f: type->fields)
			{
				cgen_newline(self);
				mn::print_to(self->out, "{};", cgen_write_field(self, f.type, f.name));
			}
			self->indent--;
			cgen_newline(self);
			mn::print_to(self->out, "}} {}", name);
			break;
		}
		case Type::KIND_UNION:
		{
			mn::print_to(self->out, "union {} {{", type_name);
			self->indent++;
			for(Field_Sign& f: type->fields)
			{
				cgen_newline(self);
				mn::print_to(self->out, "{};", cgen_write_field(self, f.type, f.name));
			}
			self->indent--;
			cgen_newline(self);
			mn::print_to(self->out, "}} {}", name);
			break;
		}
		case Type::KIND_ENUM:
		{
			mn::print_to(self->out, "enum {} {{", type_name);
			self->indent++;
			for(size_t i = 0; i < type->enum_values.count; ++i)
			{
				if (i != 0)
					mn::print_to(self->out, ", ");
				cgen_newline(self);
				mn::print_to(self->out, "{}", type->enum_values[i].id.str);
				if(type->enum_values[i].value)
				{
					mn::print_to(self->out, " = ");
					cgen_expr_gen(self, type->enum_values[i].value);
				}
			}
			self->indent--;
			cgen_newline(self);
			mn::print_to(self->out, "}} {}", name);
			break;
		}
		case Type::KIND_ALIAS:
			cgen_write_type(self, type->alias, type_name, name);
			break;
		default:
			mn::print_to(self->out, "<UNDEFINED TYPE>");
			break;
		}
	}

	inline static void
	cgen_write_type(CGen self, Type* type, const char* type_name, const char* name)
	{
		cgen_write_type(self, type, type_name, mn::str_lit(name));
	}

	//Exprs
	inline static void
	cgen_expr_gen(CGen self, Expr* expr);

	inline static void
	cgen_expr_atom(CGen self, Expr* expr)
	{
		assert(expr->kind == Expr::KIND_ATOM);
		mn::print_to(self->out, "{}", expr->atom.str);
	}

	inline static void
	cgen_expr_binary(CGen self, Expr* expr)
	{
		assert(expr->kind == Expr::KIND_BINARY);
		cgen_expr_gen(self, expr->binary.lhs);
		mn::print_to(self->out, " {} ", expr->binary.op.str);
		cgen_expr_gen(self, expr->binary.rhs);
	}

	inline static void
	cgen_expr_unary(CGen self, Expr* expr)
	{
		assert(expr->kind == Expr::KIND_UNARY);
		mn::print_to(self->out, "{}", expr->unary.op.str);
		cgen_expr_gen(self, expr->unary.expr);
	}

	inline static void
	cgen_expr_dot(CGen self, Expr* expr)
	{
		assert(expr->kind == Expr::KIND_DOT);
		cgen_expr_gen(self, expr->dot.base);
		if(expr->dot.base->type->kind == Type::KIND_ENUM)
			mn::print_to(self->out, "::{}", expr->dot.member.str);
		else if(expr->dot.base->type->kind == Type::KIND_PTR)
			mn::print_to(self->out, "->{}", expr->dot.member.str);
		else
			mn::print_to(self->out, ".{}", expr->dot.member.str);
	}

	inline static void
	cgen_expr_indexed(CGen self, Expr* expr)
	{
		assert(expr->kind == Expr::KIND_INDEXED);
		cgen_expr_gen(self, expr->indexed.base);
		mn::print_to(self->out, "[");
		cgen_expr_gen(self, expr->indexed.index);
		mn::print_to(self->out, "]");
	}

	inline static void
	cgen_expr_call(CGen self, Expr* expr)
	{
		assert(expr->kind == Expr::KIND_CALL);
		cgen_expr_gen(self, expr->call.base);
		mn::print_to(self->out, "(");
		for(size_t i = 0; i < expr->call.args.count; ++i)
		{
			if (i != 0)
				mn::print_to(self->out, ", ");
			cgen_expr_gen(self, expr->call.args[i]);
		}
		mn::print_to(self->out, ")");
	}

	inline static void
	cgen_expr_cast(CGen self, Expr* expr)
	{
		assert(expr->kind == Expr::KIND_CAST);
		mn::print_to(self->out, "({})", cgen_write_field(self, expr->type, ""));
		cgen_expr_gen(self, expr->cast.base);
	}

	inline static void
	cgen_expr_paren(CGen self, Expr* expr)
	{
		assert(expr->kind == Expr::KIND_PAREN);
		mn::print_to(self->out, "(");
		cgen_expr_gen(self, expr->paren);
		mn::print_to(self->out, ")");
	}

	inline static void
	cgen_expr_complit(CGen self, Expr* expr)
	{
		assert(expr->kind == Expr::KIND_COMPLIT);
		if(expr->type->kind != Type::KIND_ARRAY)
			mn::print_to(self->out, "({})", cgen_write_field(self, expr->type, ""));
		mn::print_to(self->out, "{{");
		self->indent++;
		for (size_t i = 0; i < expr->complit.fields.count; ++i)
		{
			if(i != 0)
				mn::print_to(self->out, ",");
			cgen_newline(self);
			if (expr->complit.fields[i].kind == Complit_Field::KIND_MEMBER)
			{
				mn::print_to(self->out, ".");
				cgen_expr_gen(self, expr->complit.fields[i].left);
			}
			else if(expr->complit.fields[i].kind == Complit_Field::KIND_ARRAY)
			{
				mn::print_to(self->out, "[");
				cgen_expr_gen(self, expr->complit.fields[i].left);
				mn::print_to(self->out, "]");
			}
			mn::print_to(self->out, " = ");
			cgen_expr_gen(self, expr->complit.fields[i].right);
		}
		self->indent--;
		if(expr->complit.fields.count > 0)
			cgen_newline(self);
		mn::print_to(self->out, "}}");
	}

	inline static void
	cgen_expr_gen(CGen self, Expr* expr)
	{
		switch(expr->kind)
		{
		case Expr::KIND_ATOM:
			cgen_expr_atom(self, expr);
			break;
		case Expr::KIND_BINARY:
			cgen_expr_binary(self, expr);
			break;
		case Expr::KIND_UNARY:
			cgen_expr_unary(self, expr);
			break;
		case Expr::KIND_DOT:
			cgen_expr_dot(self, expr);
			break;
		case Expr::KIND_INDEXED:
			cgen_expr_indexed(self, expr);
			break;
		case Expr::KIND_CALL:
			cgen_expr_call(self, expr);
			break;
		case Expr::KIND_CAST:
			cgen_expr_cast(self, expr);
			break;
		case Expr::KIND_PAREN:
			cgen_expr_paren(self, expr);
			break;
		case Expr::KIND_COMPLIT:
			cgen_expr_complit(self, expr);
			break;
		default:
			assert(false && "unreachable");
			break;
		}
	}


	//Stmts
	inline static void
	cgen_stmt_gen(CGen self, Stmt* stmt);

	inline static void
	cgen_stmt_block_gen(CGen self, Stmt* stmt);

	inline static void
	cgen_stmt_return(CGen self, Stmt* stmt)
	{
		assert(stmt->kind == Stmt::KIND_RETURN);
		if(stmt->return_stmt)
		{
			mn::print_to(self->out, "return ");
			cgen_expr_gen(self, stmt->return_stmt);
		}
		else
		{
			mn::print_to(self->out, "return");
		}
	}

	inline static void
	cgen_stmt_if(CGen self, Stmt* stmt)
	{
		assert(stmt->kind == Stmt::KIND_IF);

		mn::print_to(self->out, "if (");
		cgen_expr_gen(self, stmt->if_stmt.if_cond);
		mn::print_to(self->out, ") ");
		cgen_stmt_gen(self, stmt->if_stmt.if_body);

		for(const Else_If& e: stmt->if_stmt.else_ifs)
		{
			mn::print_to(self->out, " else if (");
			cgen_expr_gen(self, e.cond);
			mn::print_to(self->out, ") ");
			cgen_stmt_gen(self, e.body);
		}

		if(stmt->if_stmt.else_body)
		{
			mn::print_to(self->out, " else ");
			cgen_stmt_gen(self, stmt->if_stmt.else_body);
		}
	}

	inline static void
	cgen_stmt_for(CGen self, Stmt* stmt)
	{
		assert(stmt->kind == Stmt::KIND_FOR);

		cgen_scope_enter(self, src_scope_of(self->src, stmt));

		mn::print_to(self->out, "for (");
		if (stmt->for_stmt.init_stmt)
		{
			cgen_stmt_gen(self, stmt->for_stmt.init_stmt);
			mn::print_to(self->out, "; ");
		}
		else
		{
			mn::print_to(self->out, ";");
		}

		if (stmt->for_stmt.loop_cond)
		{
			cgen_expr_gen(self, stmt->for_stmt.loop_cond);
			mn::print_to(self->out, "; ");
		}
		else
		{
			mn::print_to(self->out, ";");
		}

		if (stmt->for_stmt.post_stmt)
			cgen_stmt_gen(self, stmt->for_stmt.post_stmt);

		mn::print_to(self->out, ") ");

		cgen_stmt_gen(self, stmt->for_stmt.loop_body);

		cgen_scope_leave(self);
	}

	inline static void
	cgen_stmt_var(CGen self, Stmt* stmt)
	{
		assert(stmt->kind == Stmt::KIND_VAR);

		for(size_t i = 0; i < stmt->var_stmt.ids.count; ++i)
		{
			if(i != 0)
			{
				mn::print_to(self->out, ";");
				cgen_newline(self);
			}

			Type* t = cgen_sym(self, stmt->var_stmt.ids[i].str)->type;
			mn::print_to(self->out, "{}", cgen_write_field(self, t, stmt->var_stmt.ids[i].str));

			if(i < stmt->var_stmt.exprs.count)
			{
				mn::print_to(self->out, " = ");
				cgen_expr_gen(self, stmt->var_stmt.exprs[i]);
			}
		}
	}

	inline static void
	cgen_stmt_assign_gen(CGen self, Stmt* stmt)
	{
		assert(stmt->kind == Stmt::KIND_ASSIGN);
		for(size_t i = 0; i < stmt->assign_stmt.lhs.count; ++i)
		{
			if (i != 0)
			{
				mn::print_to(self->out, ";");
				cgen_newline(self);
			}

			cgen_expr_gen(self, stmt->assign_stmt.lhs[i]);
			mn::print_to(self->out, " {} ", stmt->assign_stmt.op.str);
			cgen_expr_gen(self, stmt->assign_stmt.rhs[i]);
		}
	}

	inline static void
	cgen_stmt_anonymous_block_gen(CGen self, Stmt* stmt)
	{
		assert(stmt->kind == Stmt::KIND_BLOCK);

		cgen_scope_enter(self, src_scope_of(self->src, stmt));

		mn::print_to(self->out, "{{");

		self->indent++;

		for (Stmt* s : stmt->block_stmt)
		{
			cgen_newline(self);
			cgen_stmt_gen(self, s);
			if (s->kind == Stmt::KIND_ASSIGN ||
				s->kind == Stmt::KIND_BREAK ||
				s->kind == Stmt::KIND_CONTINUE ||
				s->kind == Stmt::KIND_RETURN ||
				s->kind == Stmt::KIND_VAR)
			{
				mn::print_to(self->out, ";");
			}
		}

		self->indent--;
		cgen_newline(self);

		mn::print_to(self->out, "}}");

		cgen_scope_leave(self);
	}

	inline static void
	cgen_stmt_block_gen(CGen self, Stmt* stmt)
	{
		assert(stmt->kind == Stmt::KIND_BLOCK);

		mn::print_to(self->out, "{{");

		self->indent++;

		for(Stmt* s: stmt->block_stmt)
		{
			cgen_newline(self);
			cgen_stmt_gen(self, s);
			if (s->kind == Stmt::KIND_ASSIGN ||
				s->kind == Stmt::KIND_BREAK ||
				s->kind == Stmt::KIND_CONTINUE ||
				s->kind == Stmt::KIND_RETURN ||
				s->kind == Stmt::KIND_VAR)
			{
				mn::print_to(self->out, ";");
			}
		}

		self->indent--;
		cgen_newline(self);

		mn::print_to(self->out, "}}");
	}

	inline static void
	cgen_stmt_gen(CGen self, Stmt* stmt)
	{
		switch(stmt->kind)
		{
		case Stmt::KIND_BREAK:
			mn::print_to(self->out, "break");
			break;
		case Stmt::KIND_CONTINUE:
			mn::print_to(self->out, "continue");
			break;
		case Stmt::KIND_RETURN:
			cgen_stmt_return(self, stmt);
			break;
		case Stmt::KIND_IF:
			cgen_stmt_if(self, stmt);
			break;
		case Stmt::KIND_FOR:
			cgen_stmt_for(self, stmt);
			break;
		case Stmt::KIND_VAR:
			cgen_stmt_var(self, stmt);
			break;
		case Stmt::KIND_ASSIGN:
			cgen_stmt_assign_gen(self, stmt);
			break;
		case Stmt::KIND_EXPR:
			cgen_expr_gen(self, stmt->expr_stmt);
			break;
		case Stmt::KIND_BLOCK:
			cgen_stmt_anonymous_block_gen(self, stmt);
			break;
		default:
			assert(false && "unreachable");
			break;
		}
	}


	//symbols
	inline static void
	cgen_sym_func_gen(CGen self, Sym* sym)
	{
		assert(sym->kind == Sym::KIND_FUNC);
		Decl* decl = sym->func_sym;
		
		mn::print_to(
			self->out,
			"{} {}(",
			cgen_write_field(self, sym->type->func.ret, ""),
			sym->name
		);

		if(sym->type->func.args.count == 0)
		{
			mn::print_to(self->out, "void");
		}
		else
		{
			size_t i = 0;
			for(const Arg& arg: decl->func_decl.args)
			{
				Type* t = sym->type->func.args[i];
				for(const Tkn& id: arg.ids)
				{
					if (i != 0)
						mn::print_to(self->out, ", ");
					mn::print_to(self->out, "{}", cgen_write_field(self, t, id.str));
					++i;
				}
			}
		}
		mn::print_to(self->out, ") ");

		cgen_scope_enter(self, src_scope_of(self->src, decl));
		cgen_stmt_block_gen(self, decl->func_decl.body);
		cgen_scope_leave(self);
	}

	inline static void
	cgen_sym_var_gen(CGen self, Sym* sym)
	{
		assert(sym->kind == Sym::KIND_VAR);
		mn::print_to(self->out, "{}", cgen_write_field(self, sym->type, sym->name));
		if(sym->var_sym.expr)
		{
			mn::print_to(self->out, " = ");
			cgen_expr_gen(self, sym->var_sym.expr);
		}
		mn::print_to(self->out, ";");
	}

	inline static void
	cgen_sym_type_gen(CGen self, Sym* sym)
	{
		assert(sym->kind == Sym::KIND_TYPE);
		mn::print_to(self->out, "typedef ");
		cgen_write_type(self, sym->type, sym->name, sym->name);
		mn::print_to(self->out, ";");
	}

	inline static void
	cgen_sym_gen(CGen self, Sym* sym)
	{
		switch(sym->kind)
		{
		case Sym::KIND_FUNC:
			cgen_sym_func_gen(self, sym);
			break;
		case Sym::KIND_VAR:
			cgen_sym_var_gen(self, sym);
			break;
		case Sym::KIND_TYPE:
			cgen_sym_type_gen(self, sym);
			break;
		default:
			assert(false && "unreachable");
			break;
		}
	}


	//API
	CGen
	cgen_new(Src *src)
	{
		auto self = mn::alloc<ICGen>();
		self->indent = 0;
		self->src = src;
		self->out = mn::memory_stream_new();
		self->scope_stack = mn::buf_new<Scope*>();

		mn::buf_push(self->scope_stack, src_scope_of(src, nullptr));
		return self;
	}

	void
	cgen_free(CGen self)
	{
		mn::stream_free(self->out);
		mn::buf_free(self->scope_stack);
		mn::free(self);
	}

	void
	cgen_gen(CGen self)
	{
		for (size_t i = 0; i < self->src->reachable_syms.count; ++i)
		{
			if (i != 0)
				cgen_newline(self);
			cgen_sym_gen(self, self->src->reachable_syms[i]);
		}
	}
}