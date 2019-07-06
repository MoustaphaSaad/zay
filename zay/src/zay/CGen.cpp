#include "zay/CGen.h"

#include <mn/Memory.h>

#include <assert.h>

namespace zay
{
	using namespace mn;

	inline static void
	cgen_indent(CGen self)
	{
		for (size_t i = 0; i < self->indent; ++i)
			vprintf(self->out, "\t");
	}

	inline static void
	cgen_newline(CGen self)
	{
		vprintf(self->out, "\n");
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
						strf(res, ", ");
					strf(res, "{}", cgen_write_field(self, type->func.args[i], str_lit("")));
				}
			}
			res = strf(res, ")");
			return cgen_write_field(self, type->func.ret, res);
		}
		case IType::KIND_VOID:
			if (name.count)
				return str_tmpf("ZayVoid {}", name);
			else
				return str_tmpf("ZayVoid");
		case IType::KIND_BOOL:
			if (name.count)
				return str_tmpf("ZayBool {}", name);
			else
				return str_tmpf("ZayBool");
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
				return str_tmpf("ZayInt8 {}", name);
			else
				return str_tmpf("ZayInt8");
		case IType::KIND_UINT8:
			if (name.count)
				return str_tmpf("ZayUint8 {}", name);
			else
				return str_tmpf("ZayUint8");
		case IType::KIND_INT16:
			if (name.count)
				return str_tmpf("ZayInt16 {}", name);
			else
				return str_tmpf("ZayInt16");
		case IType::KIND_UINT16:
			if (name.count)
				return str_tmpf("ZayUint16 {}", name);
			else
				return str_tmpf("ZayUint16");
		case IType::KIND_INT32:
			if (name.count)
				return str_tmpf("ZayInt32 {}", name);
			else
				return str_tmpf("ZayInt32");
		case IType::KIND_UINT32:
			if (name.count)
				return str_tmpf("ZayUint32 {}", name);
			else
				return str_tmpf("ZayUint32");
		case IType::KIND_INT64:
			if (name.count)
				return str_tmpf("ZayInt64 {}", name);
			else
				return str_tmpf("ZayInt64");
		case IType::KIND_UINT64:
			if (name.count)
				return str_tmpf("ZayUint64 {}", name);
			else
				return str_tmpf("ZayUint64");
		case IType::KIND_FLOAT32:
			if (name.count)
				return str_tmpf("ZayFloat32 {}", name);
			else
				return str_tmpf("ZayFloat32");
		case IType::KIND_FLOAT64:
			if (name.count)
				return str_tmpf("ZayFloat64 {}", name);
			else
				return str_tmpf("ZayFloat64");
		case IType::KIND_STRING:
			if (name.count)
				return str_tmpf("ZayString {}", name);
			else
				return str_tmpf("ZayString");
		case IType::KIND_STRUCT:
			if (name.count)
				return str_tmpf("{} {}", type->aggregate.sym->name, name);
			else
				return str_tmpf("{}", type->aggregate.sym->name);
		default:
			return str_tmpf("<UNDEFINED TYPE>");
		}
	}

	inline static Str
	cgen_write_field(CGen self, Type type, const char* name)
	{
		return cgen_write_field(self, type, str_lit(name));
	}

	//Exprs
	inline static void
	cgen_expr_gen(CGen self, Expr expr);

	inline static void
	cgen_expr_atom(CGen self, Expr expr)
	{
		assert(expr->kind == IExpr::KIND_ATOM);
		vprintf(self->out, "{}", expr->atom.str);
	}

	inline static void
	cgen_expr_binary(CGen self, Expr expr)
	{
		assert(expr->kind == IExpr::KIND_BINARY);
		cgen_expr_gen(self, expr->binary.lhs);
		vprintf(self->out, " {} ", expr->binary.op.str);
		cgen_expr_gen(self, expr->binary.rhs);
	}

	inline static void
	cgen_expr_unary(CGen self, Expr expr)
	{
		assert(expr->kind == IExpr::KIND_UNARY);
		vprintf(self->out, "{}", expr->unary.op.str);
		cgen_expr_gen(self, expr->unary.expr);
	}

	inline static void
	cgen_expr_dot(CGen self, Expr expr)
	{
		assert(expr->kind == IExpr::KIND_DOT);
		cgen_expr_gen(self, expr->dot.base);
		vprintf(self->out, ".{}", expr->dot.member.str);
	}

	inline static void
	cgen_expr_indexed(CGen self, Expr expr)
	{
		assert(expr->kind == IExpr::KIND_INDEXED);
		cgen_expr_gen(self, expr->indexed.base);
		vprintf(self->out, "[");
		cgen_expr_gen(self, expr->indexed.index);
		vprintf(self->out, "]");
	}

	inline static void
	cgen_expr_call(CGen self, Expr expr)
	{
		assert(expr->kind == IExpr::KIND_CALL);
		cgen_expr_gen(self, expr->call.base);
		vprintf(self->out, "(");
		for(size_t i = 0; i < expr->call.args.count; ++i)
		{
			if (i != 0)
				vprintf(self->out, ", ");
			cgen_expr_gen(self, expr->call.args[i]);
		}
		vprintf(self->out, ")");
	}

	inline static void
	cgen_expr_cast(CGen self, Expr expr)
	{
		assert(expr->kind == IExpr::KIND_CAST);
		vprintf(self->out, "({})", cgen_write_field(self, expr->cast.to_type, ""));
		cgen_expr_gen(self, expr->cast.base);
	}

	inline static void
	cgen_expr_paren(CGen self, Expr expr)
	{
		assert(expr->kind == IExpr::KIND_PAREN);
		vprintf(self->out, "(");
		cgen_expr_gen(self, expr->paren);
		vprintf(self->out, ")");
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
			vprintf(self->out, "return ");
			cgen_expr_gen(self, stmt->return_stmt);
		}
		else
		{
			vprintf(self->out, "return");
		}
	}

	inline static void
	cgen_stmt_if(CGen self, Stmt stmt)
	{
		assert(stmt->kind == IStmt::KIND_IF);

		vprintf(self->out, "if (");
		cgen_expr_gen(self, stmt->if_stmt.if_cond);
		vprintf(self->out, ") ");
		cgen_stmt_gen(self, stmt->if_stmt.if_body);

		for(const Else_If& e: stmt->if_stmt.else_ifs)
		{
			vprintf(self->out, " else if (");
			cgen_expr_gen(self, e.cond);
			vprintf(self->out, ") ");
			cgen_stmt_gen(self, e.body);
		}

		if(stmt->if_stmt.else_body)
		{
			vprintf(self->out, " else ");
			cgen_stmt_gen(self, stmt->if_stmt.else_body);
		}
	}

	inline static void
	cgen_stmt_for(CGen self, Stmt stmt)
	{
		assert(stmt->kind == IStmt::KIND_FOR);

		cgen_scope_enter(self, src_scope_of(self->src, stmt));

		vprintf(self->out, "for (");
		if (stmt->for_stmt.init_stmt)
		{
			cgen_stmt_gen(self, stmt->for_stmt.init_stmt);
			vprintf(self->out, "; ");
		}
		else
		{
			vprintf(self->out, ";");
		}

		if (stmt->for_stmt.loop_cond)
		{
			cgen_expr_gen(self, stmt->for_stmt.loop_cond);
			vprintf(self->out, "; ");
		}
		else
		{
			vprintf(self->out, ";");
		}

		if (stmt->for_stmt.post_stmt)
			cgen_stmt_gen(self, stmt->for_stmt.post_stmt);

		vprintf(self->out, ") ");

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
				vprintf(self->out, ";");
				cgen_newline(self);
			}

			Type t = cgen_sym(self, stmt->var_stmt.ids[i].str)->type;
			vprintf(self->out, "{}", cgen_write_field(self, t, stmt->var_stmt.ids[i].str));

			if(i < stmt->var_stmt.exprs.count)
			{
				vprintf(self->out, " = ");
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
				vprintf(self->out, ";");
				cgen_newline(self);
			}

			cgen_expr_gen(self, stmt->assign_stmt.lhs[i]);
			vprintf(self->out, " {} ", stmt->assign_stmt.op.str);
			cgen_expr_gen(self, stmt->assign_stmt.rhs[i]);
		}
	}

	inline static void
	cgen_stmt_anonymous_block_gen(CGen self, Stmt stmt)
	{
		assert(stmt->kind == IStmt::KIND_BLOCK);

		cgen_scope_enter(self, src_scope_of(self->src, stmt));

		vprintf(self->out, "{");

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
				vprintf(self->out, ";");
			}
		}

		self->indent--;
		cgen_newline(self);

		vprintf(self->out, "}");

		cgen_scope_leave(self);
	}

	inline static void
	cgen_stmt_block_gen(CGen self, Stmt stmt)
	{
		assert(stmt->kind == IStmt::KIND_BLOCK);

		vprintf(self->out, "{");

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
				vprintf(self->out, ";");
			}
		}

		self->indent--;
		cgen_newline(self);

		vprintf(self->out, "}");
	}

	inline static void
	cgen_stmt_gen(CGen self, Stmt stmt)
	{
		switch(stmt->kind)
		{
		case IStmt::KIND_BREAK:
			vprintf(self->out, "break");
			break;
		case IStmt::KIND_CONTINUE:
			vprintf(self->out, "continue");
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

	//Decls
	inline static void
	cgen_decl_struct_gen(CGen self, Decl decl)
	{
		assert(decl->kind == IDecl::KIND_STRUCT);
		vprintf(self->out, "typedef struct {} {{", decl->name.str);

		self->indent++;

		Sym sym = cgen_sym(self, decl->name.str);
		for(Field_Sign& f: sym->type->aggregate.fields)
		{
			cgen_newline(self);
			vprintf(self->out, "{}", cgen_write_field(self, f.type, f.name));
			vprintf(self->out, ";");
		}

		self->indent--;
		cgen_newline(self);

		vprintf(self->out, "} {};", decl->name.str);
	}

	inline static void
	cgen_decl_union_gen(CGen self, Decl decl)
	{
		assert(decl->kind == IDecl::KIND_UNION);
		vprintf(self->out, "typedef union {} {{", decl->name.str);

		self->indent++;

		Sym sym = cgen_sym(self, decl->name.str);
		for (Field_Sign& f : sym->type->aggregate.fields)
		{
			cgen_newline(self);
			vprintf(self->out, "{}", cgen_write_field(self, f.type, f.name));
			vprintf(self->out, ";");
		}

		self->indent--;
		cgen_newline(self);

		vprintf(self->out, "} {};", decl->name.str);
	}

	inline static void
	cgen_decl_enum_gen(CGen self, Decl decl)
	{
		assert(decl->kind == IDecl::KIND_ENUM);
		vprintf(self->out, "typedef enum {} {{", decl->name.str);

		self->indent++;
		for(size_t i = 0; i < decl->enum_decl.count; ++i)
		{
			if (i != 0)
				vprintf(self->out, ", ");
			cgen_newline(self);
			vprintf(self->out, "{}", decl->enum_decl[i].id.str);
			if(decl->enum_decl[i].expr)
			{
				vprintf(self->out, " = ");
				cgen_expr_gen(self, decl->enum_decl[i].expr);
			}
		}

		self->indent--;
		cgen_newline(self);
		vprintf(self->out, "} {};", decl->name.str);
	}

	inline static void
	cgen_decl_func_gen(CGen self, Decl decl)
	{
		assert(decl->kind == IDecl::KIND_FUNC);
		
		Sym sym = cgen_sym(self, decl->name.str);
		vprintf(
			self->out,
			"{} {}(",
			cgen_write_field(self, sym->type->func.ret, ""),
			sym->name
		);

		if(sym->type->func.args.count == 0)
		{
			vprintf(self->out, "void");
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
						vprintf(self->out, ", ");
					vprintf(self->out, "{}", cgen_write_field(self, t, id.str));
				}
				i += arg.ids.count;
			}
		}
		vprintf(self->out, ") ");

		cgen_scope_enter(self, src_scope_of(self->src, decl));
		cgen_stmt_block_gen(self, decl->func_decl.body);
		cgen_scope_leave(self);
	}

	inline static void
	cgen_decl_var_gen(CGen self, Decl decl)
	{
		assert(decl->kind == IDecl::KIND_VAR);

		for(size_t i = 0; i < decl->var_decl.ids.count; ++i)
		{
			if(i != 0)
			{
				vprintf(self->out, ";");
				cgen_newline(self);
			}

			Sym s = cgen_sym(self, decl->var_decl.ids[i].str);
			vprintf(self->out, "{}", cgen_write_field(self, s->type, s->name));
			if(i < decl->var_decl.exprs.count)
			{
				vprintf(self->out, " = ");
				cgen_expr_gen(self, decl->var_decl.exprs[i]);
			}
		}

		vprintf(self->out, ";");
	}

	inline static void
	cgen_decl_gen(CGen self, Decl decl)
	{
		switch(decl->kind)
		{
		case IDecl::KIND_STRUCT:
			cgen_decl_struct_gen(self, decl);
			break;
		case IDecl::KIND_UNION:
			cgen_decl_union_gen(self, decl);
			break;
		case IDecl::KIND_ENUM:
			cgen_decl_enum_gen(self, decl);
			break;
		case IDecl::KIND_FUNC:
			cgen_decl_func_gen(self, decl);
			break;
		case IDecl::KIND_VAR:
			cgen_decl_var_gen(self, decl);
			break;
		default:
			assert(false && "unreachable");
			break;
		}
	}


	//symbols
	inline static void
	cgen_sym_var_gen(CGen self, Sym sym)
	{
		assert(sym->kind == ISym::KIND_VAR);
		vprintf(self->out, "{}", cgen_write_field(self, sym->type, sym->name));
		if(sym->var_sym.expr)
		{
			vprintf(self->out, " = ");
			cgen_expr_gen(self, sym->var_sym.expr);
		}
		vprintf(self->out, ";");
	}

	inline static void
	cgen_sym_gen(CGen self, Sym sym)
	{
		switch(sym->kind)
		{
		case ISym::KIND_STRUCT:
			cgen_decl_struct_gen(self, sym->struct_sym);
			break;
		case ISym::KIND_UNION:
			cgen_decl_union_gen(self, sym->union_sym);
			break;
		case ISym::KIND_ENUM:
			cgen_decl_enum_gen(self, sym->enum_sym);
			break;
		case ISym::KIND_FUNC:
			cgen_decl_func_gen(self, sym->func_sym);
			break;
		case ISym::KIND_VAR:
			cgen_sym_var_gen(self, sym);
			break;
		default:
			assert(false && "unreachable");
			break;
		}
	}


	//API
	CGen
	cgen_new(Src src)
	{
		CGen self = alloc<ICGen>();
		self->indent = 0;
		self->src = src;
		self->out = stream_memory_new();
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
			memory::tmp()->free_all();
		}
	}
}