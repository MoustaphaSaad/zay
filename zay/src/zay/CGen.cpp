#include "zay/CGen.h"

#include <mn/Memory.h>

#include <assert.h>

namespace zay
{
	using namespace mn;

	inline static void
	cgen_expr_gen(CGen self, Expr expr);

	inline static void
	cgen_indent(CGen self)
	{
		for (size_t i = 0; i < self->indent; ++i)
			print_to(self->out, "\t");
	}

	inline static void
	cgen_newline(CGen self)
	{
		print_to(self->out, "\n");
		cgen_indent(self);
	}

	inline static Scope
	cgen_scope(CGen self)
	{
		return buf_top(self->scope_stack);
	}

	inline static void
	cgen_scope_enter(CGen self, Scope scope)
	{
		buf_push(self->scope_stack, scope);
	}

	inline static void
	cgen_scope_leave(CGen self)
	{
		buf_pop(self->scope_stack);
	}

	inline static Sym
	cgen_sym(CGen self, const char* name)
	{
		return scope_find(cgen_scope(self), name);
	}

	inline static Str
	cdecl_paren(Str str, Rune c)
	{
		if (c && c != '[')
			return str_tmpf("({})", str);
		return str;
	}

	inline static Str
	cgen_write_field(CGen self, Type type, const Str& name)
	{
		Rune c = 0;
		if (name.count > 0)
			c = rune_read(name.ptr);

		switch(type->kind)
		{
		case IType::KIND_PTR:
			return cgen_write_field(self, type->ptr.base, cdecl_paren(str_tmpf("*{}", name), c));
		case IType::KIND_ARRAY:
			return cgen_write_field(self, type->array.base, cdecl_paren(str_tmpf("{}[{}]", name, type->array.count), c));
		case IType::KIND_FUNC:
		{
			Str res = str_tmpf("(*{})(", name);
			if(type->func.args.count == 0)
			{
				str_push(res, "ZayVoid");
			}
			else
			{
				for(size_t i = 0; i < type->func.args.count; ++i)
				{
					if (i != 0)
						res = strf(res, ", ");
					res = strf(res, "{}", cgen_write_field(self, type->func.args[i], str_lit("")));
				}
			}
			res = strf(res, ")");
			return cgen_write_field(self, type->func.ret, res);
		}
		case IType::KIND_VOID:
			if (name.count)
				return str_tmpf("void {}", name);
			else
				return str_tmpf("void");
		case IType::KIND_BOOL:
			if (name.count)
				return str_tmpf("bool {}", name);
			else
				return str_tmpf("bool");
		case IType::KIND_INT:
			if (name.count)
				return str_tmpf("ZayInt {}", name);
			else
				return str_tmpf("ZayInt");
		case IType::KIND_UINT:
			if (name.count)
				return str_tmpf("ZayUint {}", name);
			else
				return str_tmpf("ZayUint");
		case IType::KIND_INT8:
			if (name.count)
				return str_tmpf("int8_t {}", name);
			else
				return str_tmpf("int8_t");
		case IType::KIND_UINT8:
			if (name.count)
				return str_tmpf("uint8_t {}", name);
			else
				return str_tmpf("uint8_t");
		case IType::KIND_INT16:
			if (name.count)
				return str_tmpf("int16_t {}", name);
			else
				return str_tmpf("int16_t");
		case IType::KIND_UINT16:
			if (name.count)
				return str_tmpf("uint16_t {}", name);
			else
				return str_tmpf("uint16_t");
		case IType::KIND_INT32:
			if (name.count)
				return str_tmpf("int32_t {}", name);
			else
				return str_tmpf("int32_t");
		case IType::KIND_UINT32:
			if (name.count)
				return str_tmpf("uint32_t {}", name);
			else
				return str_tmpf("uint32_t");
		case IType::KIND_INT64:
			if (name.count)
				return str_tmpf("int64_t {}", name);
			else
				return str_tmpf("int64_t");
		case IType::KIND_UINT64:
			if (name.count)
				return str_tmpf("uint64_t {}", name);
			else
				return str_tmpf("uint64_t");
		case IType::KIND_FLOAT32:
			if (name.count)
				return str_tmpf("float {}", name);
			else
				return str_tmpf("float");
		case IType::KIND_FLOAT64:
			if (name.count)
				return str_tmpf("double {}", name);
			else
				return str_tmpf("double");
		case IType::KIND_STRING:
			if (name.count)
				return str_tmpf("ZayString {}", name);
			else
				return str_tmpf("ZayString");
		case IType::KIND_STRUCT:
		case IType::KIND_UNION:
		case IType::KIND_ENUM:
			if (name.count)
				return str_tmpf("{} {}", type->sym->name, name);
			else
				return str_tmpf("{}", type->sym->name);
		case IType::KIND_ALIAS:
			return cgen_write_field(self, type->alias, name);
		default:
			return str_tmpf("<UNDEFINED TYPE>");
		}
	}

	inline static Str
	cgen_write_field(CGen self, Type type, const char* name)
	{
		return cgen_write_field(self, type, str_lit(name));
	}

	inline static void
	cgen_write_type(CGen self, Type type, const char* type_name, const Str& name)
	{
		Rune c = 0;
		if (name.count > 0)
			c = rune_read(name.ptr);

		switch(type->kind)
		{
		case IType::KIND_PTR:
			cgen_write_type(self, type->ptr.base, type_name, cdecl_paren(str_tmpf("*{}", name), c));
			break;
		case IType::KIND_ARRAY:
			cgen_write_type(self, type->array.base, type_name, cdecl_paren(str_tmpf("{}[{}]", name, type->array.count), c));
			break;
		case IType::KIND_FUNC:
		{
			Str res = str_tmpf("(*{})(", name);
			if(type->func.args.count == 0)
			{
				str_push(res, "ZayVoid");
			}
			else
			{
				for(size_t i = 0; i < type->func.args.count; ++i)
				{
					if (i != 0)
						res = strf(res, ", ");
					res = strf(res, "{}", cgen_write_field(self, type->func.args[i], str_lit("")));
				}
			}
			res = strf(res, ")");
			print_to(self->out, "{}", cgen_write_field(self, type->func.ret, res));
			break;
		}
		case IType::KIND_VOID:
			if (name.count)
				print_to(self->out, "void {}", name);
			else
				print_to(self->out, "void");
			break;
		case IType::KIND_BOOL:
			if (name.count)
				print_to(self->out, "bool {}", name);
			else
				print_to(self->out, "bool");
			break;
		case IType::KIND_INT:
			if (name.count)
				print_to(self->out, "ZayInt {}", name);
			else
				print_to(self->out, "ZayInt");
			break;
		case IType::KIND_UINT:
			if (name.count)
				print_to(self->out, "ZayUint {}", name);
			else
				print_to(self->out, "ZayUint");
			break;
		case IType::KIND_INT8:
			if (name.count)
				print_to(self->out, "int8_t {}", name);
			else
				print_to(self->out, "int8_t");
			break;
		case IType::KIND_UINT8:
			if (name.count)
				print_to(self->out, "uint8_t {}", name);
			else
				print_to(self->out, "uint8_t");
			break;
		case IType::KIND_INT16:
			if (name.count)
				print_to(self->out, "int16_t {}", name);
			else
				print_to(self->out, "int16_t");
			break;
		case IType::KIND_UINT16:
			if (name.count)
				print_to(self->out, "uint16_t {}", name);
			else
				print_to(self->out, "uint16_t");
			break;
		case IType::KIND_INT32:
			if (name.count)
				print_to(self->out, "int32_t {}", name);
			else
				print_to(self->out, "int32_t");
			break;
		case IType::KIND_UINT32:
			if (name.count)
				print_to(self->out, "uint32_t {}", name);
			else
				print_to(self->out, "uint32_t");
			break;
		case IType::KIND_INT64:
			if (name.count)
				print_to(self->out, "int64_t {}", name);
			else
				print_to(self->out, "int64_t");
			break;
		case IType::KIND_UINT64:
			if (name.count)
				print_to(self->out, "uint64_t {}", name);
			else
				print_to(self->out, "uint64_t");
			break;
		case IType::KIND_FLOAT32:
			if (name.count)
				print_to(self->out, "float {}", name);
			else
				print_to(self->out, "float");
			break;
		case IType::KIND_FLOAT64:
			if (name.count)
				print_to(self->out, "double {}", name);
			else
				print_to(self->out, "double");
			break;
		case IType::KIND_STRING:
			if (name.count)
				print_to(self->out, "ZayString {}", name);
			else
				print_to(self->out, "ZayString");
			break;
		case IType::KIND_STRUCT:
		{
			print_to(self->out, "struct {} {{", type_name);
			self->indent++;
			for(Field_Sign& f: type->fields)
			{
				cgen_newline(self);
				print_to(self->out, "{};", cgen_write_field(self, f.type, f.name));
			}
			self->indent--;
			cgen_newline(self);
			print_to(self->out, "}} {}", name);
			break;
		}
		case IType::KIND_UNION:
		{
			print_to(self->out, "union {} {{", type_name);
			self->indent++;
			for(Field_Sign& f: type->fields)
			{
				cgen_newline(self);
				print_to(self->out, "{};", cgen_write_field(self, f.type, f.name));
			}
			self->indent--;
			cgen_newline(self);
			print_to(self->out, "}} {}", name);
			break;
		}
		case IType::KIND_ENUM:
		{
			print_to(self->out, "enum {} {{", type_name);
			self->indent++;
			for(size_t i = 0; i < type->enum_values.count; ++i)
			{
				if (i != 0)
					print_to(self->out, ", ");
				cgen_newline(self);
				print_to(self->out, "{}", type->enum_values[i].id.str);
				if(type->enum_values[i].value)
				{
					print_to(self->out, " = ");
					cgen_expr_gen(self, type->enum_values[i].value);
				}
			}
			self->indent--;
			cgen_newline(self);
			print_to(self->out, "}} {}", name);
			break;
		}
		case IType::KIND_ALIAS:
			cgen_write_type(self, type->alias, type_name, name);
			break;
		default:
			print_to(self->out, "<UNDEFINED TYPE>");
			break;
		}
	}

	inline static void
	cgen_write_type(CGen self, Type type, const char* type_name, const char* name)
	{
		cgen_write_type(self, type, type_name, str_lit(name));
	}

	//Exprs
	inline static void
	cgen_expr_gen(CGen self, Expr expr);

	inline static void
	cgen_expr_atom(CGen self, Expr expr)
	{
		assert(expr->kind == IExpr::KIND_ATOM);
		print_to(self->out, "{}", expr->atom.str);
	}

	inline static void
	cgen_expr_binary(CGen self, Expr expr)
	{
		assert(expr->kind == IExpr::KIND_BINARY);
		cgen_expr_gen(self, expr->binary.lhs);
		print_to(self->out, " {} ", expr->binary.op.str);
		cgen_expr_gen(self, expr->binary.rhs);
	}

	inline static void
	cgen_expr_unary(CGen self, Expr expr)
	{
		assert(expr->kind == IExpr::KIND_UNARY);
		print_to(self->out, "{}", expr->unary.op.str);
		cgen_expr_gen(self, expr->unary.expr);
	}

	inline static void
	cgen_expr_dot(CGen self, Expr expr)
	{
		assert(expr->kind == IExpr::KIND_DOT);
		cgen_expr_gen(self, expr->dot.base);
		if(expr->dot.base->type->kind == IType::KIND_ENUM)
			print_to(self->out, "::{}", expr->dot.member.str);
		else if(expr->dot.base->type->kind == IType::KIND_PTR)
			print_to(self->out, "->{}", expr->dot.member.str);
		else
			print_to(self->out, ".{}", expr->dot.member.str);
	}

	inline static void
	cgen_expr_indexed(CGen self, Expr expr)
	{
		assert(expr->kind == IExpr::KIND_INDEXED);
		cgen_expr_gen(self, expr->indexed.base);
		print_to(self->out, "[");
		cgen_expr_gen(self, expr->indexed.index);
		print_to(self->out, "]");
	}

	inline static void
	cgen_expr_call(CGen self, Expr expr)
	{
		assert(expr->kind == IExpr::KIND_CALL);
		cgen_expr_gen(self, expr->call.base);
		print_to(self->out, "(");
		for(size_t i = 0; i < expr->call.args.count; ++i)
		{
			if (i != 0)
				print_to(self->out, ", ");
			cgen_expr_gen(self, expr->call.args[i]);
		}
		print_to(self->out, ")");
	}

	inline static void
	cgen_expr_cast(CGen self, Expr expr)
	{
		assert(expr->kind == IExpr::KIND_CAST);
		print_to(self->out, "({})", cgen_write_field(self, expr->type, ""));
		cgen_expr_gen(self, expr->cast.base);
	}

	inline static void
	cgen_expr_paren(CGen self, Expr expr)
	{
		assert(expr->kind == IExpr::KIND_PAREN);
		print_to(self->out, "(");
		cgen_expr_gen(self, expr->paren);
		print_to(self->out, ")");
	}

	inline static void
	cgen_expr_complit(CGen self, Expr expr)
	{
		assert(expr->kind == IExpr::KIND_COMPLIT);
		if(expr->type->kind != IType::KIND_ARRAY)
			print_to(self->out, "({})", cgen_write_field(self, expr->type, ""));
		print_to(self->out, "{{");
		self->indent++;
		for (size_t i = 0; i < expr->complit.fields.count; ++i)
		{
			if(i != 0)
				print_to(self->out, ",");
			cgen_newline(self);
			if (expr->complit.fields[i].kind == Complit_Field::KIND_MEMBER)
			{
				print_to(self->out, ".");
				cgen_expr_gen(self, expr->complit.fields[i].left);
			}
			else if(expr->complit.fields[i].kind == Complit_Field::KIND_ARRAY)
			{
				print_to(self->out, "[");
				cgen_expr_gen(self, expr->complit.fields[i].left);
				print_to(self->out, "]");
			}
			print_to(self->out, " = ");
			cgen_expr_gen(self, expr->complit.fields[i].right);
		}
		self->indent--;
		if(expr->complit.fields.count > 0)
			cgen_newline(self);
		print_to(self->out, "}}");
	}

	inline static void
	cgen_expr_gen(CGen self, Expr expr)
	{
		switch(expr->kind)
		{
		case IExpr::KIND_ATOM:
			cgen_expr_atom(self, expr);
			break;
		case IExpr::KIND_BINARY:
			cgen_expr_binary(self, expr);
			break;
		case IExpr::KIND_UNARY:
			cgen_expr_unary(self, expr);
			break;
		case IExpr::KIND_DOT:
			cgen_expr_dot(self, expr);
			break;
		case IExpr::KIND_INDEXED:
			cgen_expr_indexed(self, expr);
			break;
		case IExpr::KIND_CALL:
			cgen_expr_call(self, expr);
			break;
		case IExpr::KIND_CAST:
			cgen_expr_cast(self, expr);
			break;
		case IExpr::KIND_PAREN:
			cgen_expr_paren(self, expr);
			break;
		case IExpr::KIND_COMPLIT:
			cgen_expr_complit(self, expr);
			break;
		default:
			assert(false && "unreachable");
			break;
		}
	}


	//Stmts
	inline static void
	cgen_stmt_gen(CGen self, Stmt stmt);

	inline static void
	cgen_stmt_block_gen(CGen self, Stmt stmt);

	inline static void
	cgen_stmt_return(CGen self, Stmt stmt)
	{
		assert(stmt->kind == IStmt::KIND_RETURN);
		if(stmt->return_stmt)
		{
			print_to(self->out, "return ");
			cgen_expr_gen(self, stmt->return_stmt);
		}
		else
		{
			print_to(self->out, "return");
		}
	}

	inline static void
	cgen_stmt_if(CGen self, Stmt stmt)
	{
		assert(stmt->kind == IStmt::KIND_IF);

		print_to(self->out, "if (");
		cgen_expr_gen(self, stmt->if_stmt.if_cond);
		print_to(self->out, ") ");
		cgen_stmt_gen(self, stmt->if_stmt.if_body);

		for(const Else_If& e: stmt->if_stmt.else_ifs)
		{
			print_to(self->out, " else if (");
			cgen_expr_gen(self, e.cond);
			print_to(self->out, ") ");
			cgen_stmt_gen(self, e.body);
		}

		if(stmt->if_stmt.else_body)
		{
			print_to(self->out, " else ");
			cgen_stmt_gen(self, stmt->if_stmt.else_body);
		}
	}

	inline static void
	cgen_stmt_for(CGen self, Stmt stmt)
	{
		assert(stmt->kind == IStmt::KIND_FOR);

		cgen_scope_enter(self, src_scope_of(self->src, stmt));

		print_to(self->out, "for (");
		if (stmt->for_stmt.init_stmt)
		{
			cgen_stmt_gen(self, stmt->for_stmt.init_stmt);
			print_to(self->out, "; ");
		}
		else
		{
			print_to(self->out, ";");
		}

		if (stmt->for_stmt.loop_cond)
		{
			cgen_expr_gen(self, stmt->for_stmt.loop_cond);
			print_to(self->out, "; ");
		}
		else
		{
			print_to(self->out, ";");
		}

		if (stmt->for_stmt.post_stmt)
			cgen_stmt_gen(self, stmt->for_stmt.post_stmt);

		print_to(self->out, ") ");

		cgen_stmt_gen(self, stmt->for_stmt.loop_body);

		cgen_scope_leave(self);
	}

	inline static void
	cgen_stmt_var(CGen self, Stmt stmt)
	{
		assert(stmt->kind == IStmt::KIND_VAR);

		for(size_t i = 0; i < stmt->var_stmt.ids.count; ++i)
		{
			if(i != 0)
			{
				print_to(self->out, ";");
				cgen_newline(self);
			}

			Type t = cgen_sym(self, stmt->var_stmt.ids[i].str)->type;
			print_to(self->out, "{}", cgen_write_field(self, t, stmt->var_stmt.ids[i].str));

			if(i < stmt->var_stmt.exprs.count)
			{
				print_to(self->out, " = ");
				cgen_expr_gen(self, stmt->var_stmt.exprs[i]);
			}
		}
	}

	inline static void
	cgen_stmt_assign_gen(CGen self, Stmt stmt)
	{
		assert(stmt->kind == IStmt::KIND_ASSIGN);
		for(size_t i = 0; i < stmt->assign_stmt.lhs.count; ++i)
		{
			if (i != 0)
			{
				print_to(self->out, ";");
				cgen_newline(self);
			}

			cgen_expr_gen(self, stmt->assign_stmt.lhs[i]);
			print_to(self->out, " {} ", stmt->assign_stmt.op.str);
			cgen_expr_gen(self, stmt->assign_stmt.rhs[i]);
		}
	}

	inline static void
	cgen_stmt_anonymous_block_gen(CGen self, Stmt stmt)
	{
		assert(stmt->kind == IStmt::KIND_BLOCK);

		cgen_scope_enter(self, src_scope_of(self->src, stmt));

		print_to(self->out, "{{");

		self->indent++;

		for (Stmt s : stmt->block_stmt)
		{
			cgen_newline(self);
			cgen_stmt_gen(self, s);
			if (s->kind == IStmt::KIND_ASSIGN ||
				s->kind == IStmt::KIND_BREAK ||
				s->kind == IStmt::KIND_CONTINUE ||
				s->kind == IStmt::KIND_RETURN ||
				s->kind == IStmt::KIND_VAR)
			{
				print_to(self->out, ";");
			}
		}

		self->indent--;
		cgen_newline(self);

		print_to(self->out, "}}");

		cgen_scope_leave(self);
	}

	inline static void
	cgen_stmt_block_gen(CGen self, Stmt stmt)
	{
		assert(stmt->kind == IStmt::KIND_BLOCK);

		print_to(self->out, "{{");

		self->indent++;

		for(Stmt s: stmt->block_stmt)
		{
			cgen_newline(self);
			cgen_stmt_gen(self, s);
			if (s->kind == IStmt::KIND_ASSIGN ||
				s->kind == IStmt::KIND_BREAK ||
				s->kind == IStmt::KIND_CONTINUE ||
				s->kind == IStmt::KIND_RETURN ||
				s->kind == IStmt::KIND_VAR)
			{
				print_to(self->out, ";");
			}
		}

		self->indent--;
		cgen_newline(self);

		print_to(self->out, "}}");
	}

	inline static void
	cgen_stmt_gen(CGen self, Stmt stmt)
	{
		switch(stmt->kind)
		{
		case IStmt::KIND_BREAK:
			print_to(self->out, "break");
			break;
		case IStmt::KIND_CONTINUE:
			print_to(self->out, "continue");
			break;
		case IStmt::KIND_RETURN:
			cgen_stmt_return(self, stmt);
			break;
		case IStmt::KIND_IF:
			cgen_stmt_if(self, stmt);
			break;
		case IStmt::KIND_FOR:
			cgen_stmt_for(self, stmt);
			break;
		case IStmt::KIND_VAR:
			cgen_stmt_var(self, stmt);
			break;
		case IStmt::KIND_ASSIGN:
			cgen_stmt_assign_gen(self, stmt);
			break;
		case IStmt::KIND_EXPR:
			cgen_expr_gen(self, stmt->expr_stmt);
			break;
		case IStmt::KIND_BLOCK:
			cgen_stmt_anonymous_block_gen(self, stmt);
			break;
		default:
			assert(false && "unreachable");
			break;
		}
	}


	//symbols
	inline static void
	cgen_sym_func_gen(CGen self, Sym sym)
	{
		assert(sym->kind == ISym::KIND_FUNC);
		Decl decl = sym->func_sym;
		
		print_to(
			self->out,
			"{} {}(",
			cgen_write_field(self, sym->type->func.ret, ""),
			sym->name
		);

		if(sym->type->func.args.count == 0)
		{
			print_to(self->out, "void");
		}
		else
		{
			size_t i = 0;
			for(const Arg& arg: decl->func_decl.args)
			{
				Type t = sym->type->func.args[i];
				for(const Tkn& id: arg.ids)
				{
					if (i != 0)
						print_to(self->out, ", ");
					print_to(self->out, "{}", cgen_write_field(self, t, id.str));
					++i;
				}
			}
		}
		print_to(self->out, ") ");

		cgen_scope_enter(self, src_scope_of(self->src, decl));
		cgen_stmt_block_gen(self, decl->func_decl.body);
		cgen_scope_leave(self);
	}

	inline static void
	cgen_sym_var_gen(CGen self, Sym sym)
	{
		assert(sym->kind == ISym::KIND_VAR);
		print_to(self->out, "{}", cgen_write_field(self, sym->type, sym->name));
		if(sym->var_sym.expr)
		{
			print_to(self->out, " = ");
			cgen_expr_gen(self, sym->var_sym.expr);
		}
		print_to(self->out, ";");
	}

	inline static void
	cgen_sym_type_gen(CGen self, Sym sym)
	{
		assert(sym->kind == ISym::KIND_TYPE);
		print_to(self->out, "typedef ");
		cgen_write_type(self, sym->type, sym->name, sym->name);
		print_to(self->out, ";");
	}

	inline static void
	cgen_sym_gen(CGen self, Sym sym)
	{
		switch(sym->kind)
		{
		case ISym::KIND_FUNC:
			cgen_sym_func_gen(self, sym);
			break;
		case ISym::KIND_VAR:
			cgen_sym_var_gen(self, sym);
			break;
		case ISym::KIND_TYPE:
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
		CGen self = alloc<ICGen>();
		self->indent = 0;
		self->src = src;
		self->out = memory_stream_new();
		self->scope_stack = buf_new<Scope>();

		buf_push(self->scope_stack, src_scope_of(src, nullptr));
		return self;
	}

	void
	cgen_free(CGen self)
	{
		stream_free(self->out);
		buf_free(self->scope_stack);
		free(self);
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