#include "zay/Typer.h"

#include <mn/Memory.h>
#include <mn/IO.h>

#include <assert.h>

namespace zay
{
	using namespace mn;

	inline static void
	typer_sym_resolve(Typer self, Sym sym);

	inline static Type
	typer_expr_resolve(Typer self, Expr expr);

	inline static Type
	typer_stmt_resolve(Typer self, Stmt stmt);

	inline static Type
	typer_stmt_block_resolve(Typer self, Stmt stmt);

	inline static void
	typer_scope_enter(Typer self, Scope scope)
	{
		buf_push(self->scope_stack, scope);
	}

	inline static void
	typer_scope_leave(Typer self)
	{
		buf_pop(self->scope_stack);
	}

	inline static Scope
	typer_scope(Typer self)
	{
		return buf_top(self->scope_stack);
	}

	inline static void
	typer_sym(Typer self, Sym sym)
	{
		Scope scope = typer_scope(self);
		if(Sym old = scope_has(scope, sym->name))
		{
			Tkn new_tkn = sym_tkn(sym);
			Tkn old_tkn = sym_tkn(old);
			Str msg = strf(
				"'{}' symbol redefinition, it was first defined {}:{}",
				new_tkn.str,
				old_tkn.pos.line,
				old_tkn.pos.col
			);
			src_err(self->src, err_tkn(new_tkn, msg));
			sym_free(sym);
		}

		scope_add(scope, sym);
	}

	inline static void
	typer_shallow_walk(Typer self)
	{
		for(Decl d: self->src->ast->decls)
		{
			switch(d->kind)
			{
			case IDecl::KIND_STRUCT:
				typer_sym(self, sym_struct(d));
				break;
			case IDecl::KIND_UNION:
				typer_sym(self, sym_union(d));
				break;
			case IDecl::KIND_ENUM:
				typer_sym(self, sym_enum(d));
				break;
			case IDecl::KIND_VAR:
				for (size_t i = 0; i < d->var_decl.ids.count; ++i)
				{
					Expr e = nullptr;
					if (i < d->var_decl.exprs.count)
						e = d->var_decl.exprs[i];
					typer_sym(self, sym_var(d->var_decl.ids[i], d, d->var_decl.type, e));
				}
				break;
			case IDecl::KIND_FUNC:
				typer_sym(self, sym_func(d));
				break;
			default: assert(false && "unreachable"); break;
			}
		}
	}

	inline static Sym
	typer_sym_by_name(Typer self, const char* name)
	{
		return scope_find(typer_scope(self), name);
	}

	inline static Type
	token_to_type(const Tkn& tkn)
	{
		switch(tkn.kind)
		{
		case Tkn::KIND_KEYWORD_BOOL: return type_bool;
		case Tkn::KIND_KEYWORD_INT: return type_int;
		case Tkn::KIND_KEYWORD_UINT: return type_uint;
		case Tkn::KIND_KEYWORD_INT8: return type_int8;
		case Tkn::KIND_KEYWORD_UINT8: return type_uint8;
		case Tkn::KIND_KEYWORD_INT16: return type_int16;
		case Tkn::KIND_KEYWORD_UINT16: return type_uint16;
		case Tkn::KIND_KEYWORD_INT32: return type_int32;
		case Tkn::KIND_KEYWORD_UINT32: return type_uint32;
		case Tkn::KIND_KEYWORD_INT64: return type_int64;
		case Tkn::KIND_KEYWORD_UINT64: return type_uint64;
		case Tkn::KIND_KEYWORD_FLOAT32: return type_float32;
		case Tkn::KIND_KEYWORD_FLOAT64: return type_float64;
		case Tkn::KIND_KEYWORD_STRING: return type_string;
		default: return type_void;
		}
	}

	inline static Type
	typer_type_sign_resolve(Typer self, const Type_Sign& sign)
	{
		Type res = type_void;

		for(size_t i = 1; i <= sign.count; ++i)
		{
			const Type_Atom& atom = sign[sign.count - i];
			switch(atom.kind)
			{
			case Type_Atom::KIND_NAMED:
				//this maybe a basic type of the builtin types
				res = token_to_type(atom.named);
				//if we discovered that it's not, then this is user defined type
				//and we should find it in the symbol table
				if(res == type_void)
				{
					if(Sym sym = typer_sym_by_name(self, atom.named.str))
					{
						//so we make sure we did resolve this symbol
						typer_sym_resolve(self, sym);
						//and whatever the type of this symbol is the type of this named atom
						res = sym->type;
					}
					else
					{
						src_err(
							self->src,
							err_tkn(atom.named, strf("'{}' undefined symbol", atom.named.str))
						);
					}
				}
				break;
			case Type_Atom::KIND_PTR:
				res = type_intern_ptr(self->src->type_table, res);
				break;
			case Type_Atom::KIND_ARRAY:
				res = type_intern_array(self->src->type_table, Array_Sign{res, atom.count});
				break;
			default:
				assert(false && "unreachable");
				break;
			}
		}

		return res;
	}

	inline static void
	typer_type_complete(Typer self, Sym sym)
	{
		Type type = sym->type;
		if(type->kind == IType::KIND_COMPLETING)
		{
			src_err(self->src, err_tkn(sym_tkn(sym), strf("'{}' recursive type", sym->name)));
			return;
		}
		else if(type->kind != IType::KIND_INCOMPLETE)
		{
			return;
		}

		type->kind = IType::KIND_COMPLETING;
		if(sym->kind == ISym::KIND_STRUCT)
		{
			Decl decl = sym->struct_sym;
			Buf<Field_Sign> fields = buf_new<Field_Sign>();
			for(size_t i = 0; i < decl->struct_decl.count; ++i)
			{
				Type field_type = typer_type_sign_resolve(self, decl->struct_decl[i].type);
				if (field_type->kind == IType::KIND_COMPLETING ||
					field_type->kind == IType::KIND_INCOMPLETE)
				{
					typer_type_complete(self, field_type->aggregate.sym);
				}

				for(size_t j = 0; j < decl->struct_decl[i].ids.count; ++j)
				{
					buf_push(fields, Field_Sign{
						decl->struct_decl[i].ids[j].str,
						field_type,
						0 //set the offset here to be 0 for now
					});
				}
			}
			type_struct_complete(type, fields);
		}
		else if(sym->kind == ISym::KIND_UNION)
		{
			Decl decl = sym->union_sym;
			Buf<Field_Sign> fields = buf_new<Field_Sign>();
			for (size_t i = 0; i < decl->union_decl.count; ++i)
			{
				Type field_type = typer_type_sign_resolve(self, decl->union_decl[i].type);
				if (field_type->kind == IType::KIND_COMPLETING ||
					field_type->kind == IType::KIND_INCOMPLETE)
				{
					typer_type_complete(self, field_type->aggregate.sym);
				}

				for (size_t j = 0; j < decl->union_decl[i].ids.count; ++j)
				{
					buf_push(fields, Field_Sign{
						decl->union_decl[i].ids[j].str,
						field_type,
						0 //set the offset here to be 0 for now
						});
				}
			}
			type_union_complete(type, fields);
		}
		else if(sym->kind == ISym::KIND_ENUM)
		{
			Decl decl = sym->enum_sym;
			Buf<Enum_Value> values = buf_new<Enum_Value>();
			for(size_t i = 0; i < decl->enum_decl.count; ++i)
			{
				Enum_Value v{ decl->enum_decl[i].id, decl->enum_decl[i].expr };
				if (v.value)
				{
					Type value_type = typer_expr_resolve(self, v.value);
					if(value_type != type_int)
					{
						src_err(
							self->src,
							err_expr(v.value, strf("enums should have int values but found '{}'", value_type))
						);
					}
				}
				buf_push(values, v);
			}
			type_enum_complete(type, values);
		}
		else 
		{
			assert(false && "unreachable");
		}
	}


	//expressions
	inline static Type
	typer_expr_atom_resolve(Typer self, Expr expr)
	{
		assert(expr->kind == IExpr::KIND_ATOM);
		switch(expr->atom.kind)
		{
		case Tkn::KIND_INTEGER: return type_int;
		case Tkn::KIND_FLOAT: return type_float64;
		case Tkn::KIND_KEYWORD_STRING: return type_string;
		case Tkn::KIND_KEYWORD_FALSE:
		case Tkn::KIND_KEYWORD_TRUE:
			return type_bool;
		case Tkn::KIND_ID:
			if(Sym sym = typer_sym_by_name(self, expr->atom.str))
			{
				typer_sym_resolve(self, sym);
				return sym->type;
			}
			src_err(self->src, err_expr(expr, strf("'{}' undefined symbol", expr->atom.str)));
			return type_void;
		default: assert(false && "unreachable"); return type_void;
		}
	}

	inline static Type
	typer_expr_binary_resolve(Typer self, Expr expr)
	{
		assert(expr->kind == IExpr::KIND_BINARY);

		Type lhs_type = typer_expr_resolve(self, expr->binary.lhs);
		Type rhs_type = typer_expr_resolve(self, expr->binary.rhs);

		if(lhs_type != rhs_type)
			src_err(self->src, err_expr(expr, strf("type mismatch in binary expression")));

		if(expr->binary.op.kind == Tkn::KIND_LOGIC_AND || expr->binary.op.kind == Tkn::KIND_LOGIC_OR)
		{
			if(lhs_type != type_bool)
			{
				src_err(
					self->src,
					err_expr(expr->binary.lhs, strf("logical operator only work on boolean types"))
				);
			}

			if(rhs_type != type_bool)
			{
				src_err(
					self->src,
					err_expr(expr->binary.rhs, strf("logical operator only work on boolean types"))
				);
			}
		}

		if (expr->binary.op.kind == Tkn::KIND_LESS ||
			expr->binary.op.kind == Tkn::KIND_LESS_EQUAL ||
			expr->binary.op.kind == Tkn::KIND_GREATER ||
			expr->binary.op.kind == Tkn::KIND_GREATER_EQUAL ||
			expr->binary.op.kind == Tkn::KIND_EQUAL_EQUAL ||
			expr->binary.op.kind == Tkn::KIND_NOT_EQUAL)
		{
			return type_bool;
		}

		return lhs_type;
	}

	inline static bool
	type_is_numeric(Type t)
	{
		return (
			t == type_int ||
			t == type_uint ||
			t == type_int8 ||
			t == type_uint8 ||
			t == type_int16 ||
			t == type_uint16 ||
			t == type_int32 ||
			t == type_uint32 ||
			t == type_int64 ||
			t == type_uint64 ||
			t == type_float32 ||
			t == type_float64
		);
	}

	inline static bool
	type_is_integer(Type t)
	{
		return (
			t == type_int ||
			t == type_uint ||
			t == type_int8 ||
			t == type_uint8 ||
			t == type_int16 ||
			t == type_uint16 ||
			t == type_int32 ||
			t == type_uint32 ||
			t == type_int64 ||
			t == type_uint64
		);
	}

	inline static Type
	typer_expr_unary_resolve(Typer self, Expr expr)
	{
		assert(expr->kind == IExpr::KIND_UNARY);

		Type type = typer_expr_resolve(self, expr->unary.expr);

		//works with numbers
		if (expr->unary.op.kind == Tkn::KIND_INC ||
			expr->unary.op.kind == Tkn::KIND_DEC ||
			expr->unary.op.kind == Tkn::KIND_PLUS ||
			expr->unary.op.kind == Tkn::KIND_MINUS)
		{
			if(type_is_numeric(type) == false)
			{
				src_err(
					self->src,
					err_expr(expr->unary.expr, strf("'{}' is only allowed for numeric types", expr->unary.op.str))
				);
			}
		}
		else if(expr->unary.op.kind == Tkn::KIND_LOGIC_NOT)
		{
			if(type != type_bool)
			{
				src_err(
					self->src,
					err_expr(expr, strf("logical not operator is only allowed for boolean types"))
				);
			}
		}
		else if(expr->unary.op.kind == Tkn::KIND_BIT_AND)
		{
			return type_intern_ptr(self->src->type_table, type);
		}
		else if(expr->unary.op.kind == Tkn::KIND_STAR)
		{
			if(type->kind != IType::KIND_PTR)
			{
				src_err(
					self->src,
					err_expr(expr->unary.expr, strf("cannot dereference non pointer type"))
				);
			}
			else
			{
				return type->ptr.base;
			}
			
		}
		return type;
	}

	inline static Type
	type_unqualify(Type t)
	{
		if (t->kind == IType::KIND_PTR)
			return t->ptr.base;
		return t;
	}

	inline static Type
	typer_expr_dot_resolve(Typer self, Expr expr)
	{
		assert(expr->kind == IExpr::KIND_DOT);
		Type type = typer_expr_resolve(self, expr->dot.base);
		Type unqualified_type = type_unqualify(type);
		Type res = type_void;
		if(unqualified_type->kind == IType::KIND_STRUCT)
		{
			for(const Field_Sign& f: unqualified_type->aggregate.fields)
			{
				if(f.name == expr->dot.member.str)
				{
					res = f.type;
					break;
				}
			}
			if(res == type_void)
			{
				src_err(self->src, err_tkn(expr->dot.member, strf("undefined struct field")));
			}
		}
		return res;
	}

	inline static Type
	typer_expr_indexed_resolve(Typer self, Expr expr)
	{
		assert(expr->kind == IExpr::KIND_INDEXED);
		Type type = typer_expr_resolve(self, expr->indexed.base);
		if(type->kind != IType::KIND_ARRAY)
		{
			src_err(
				self->src,
				err_expr(expr->indexed.base, strf("expression type '{}' is not an array", type))
			);
			return type_void;
		}

		if(type_is_integer(type) == false)
		{
			src_err(
				self->src,
				err_expr(expr->indexed.index, strf("index expression type '{}' is not an integer", type))
			);
		}
		return type->array.base;
	}

	inline static Type
	typer_expr_call_resolve(Typer self, Expr expr)
	{
		assert(expr->kind == IExpr::KIND_CALL);
		Type res = typer_expr_resolve(self, expr->call.base);
		if(res->kind != IType::KIND_FUNC)
		{
			src_err(self->src, err_expr(expr->call.base, strf("invalid call, expression is not a function")));
			return type_void;
		}

		if(expr->call.args.count != res->func.args.count)
		{
			Str msg = strf(
				"function expected {} arguments but {} were provided",
				res->func.args.count,
				expr->call.args.count
			);
			src_err(self->src, err_expr(expr, msg));
			return type_void;
		}

		for(size_t i = 0; i < expr->call.args.count; ++i)
		{
			Type type = typer_expr_resolve(self, expr->call.args[i]);
			if(type != res->func.args[i])
			{
				Str msg = strf("function argument {} type mismatch", i);
				src_err(self->src, err_expr(expr->call.args[i], msg));
			}
		}
		return res->func.ret;
	}

	inline static bool
	type_is_number(Type t)
	{
		return (
			t->kind == IType::KIND_INT ||
			t->kind == IType::KIND_UINT ||
			t->kind == IType::KIND_INT8 ||
			t->kind == IType::KIND_UINT8 ||
			t->kind == IType::KIND_INT16 ||
			t->kind == IType::KIND_UINT16 ||
			t->kind == IType::KIND_INT32 ||
			t->kind == IType::KIND_UINT32 ||
			t->kind == IType::KIND_INT64 ||
			t->kind == IType::KIND_UINT64 ||
			t->kind == IType::KIND_FLOAT32 ||
			t->kind == IType::KIND_FLOAT64
		);
	}

	inline static Type
	typer_expr_cast_resolve(Typer self, Expr expr)
	{
		assert(expr->kind == IExpr::KIND_CAST);
		Type from_type = typer_expr_resolve(self, expr->cast.base);
		Type to_type = typer_type_sign_resolve(self, expr->cast.type);

		if(type_is_number(from_type) && type_is_number(to_type))
			return to_type;
		else if(from_type->kind == IType::KIND_PTR && to_type->kind == IType::KIND_PTR)
			return to_type;

		src_err(
			self->src,
			err_expr(expr, strf("can't cast '{}' to '{}'", from_type, to_type))
		);
		return type_void;
	}

	inline static Type
	typer_expr_paren_resolve(Typer self, Expr expr)
	{
		assert(expr->kind == IExpr::KIND_PAREN);
		return typer_expr_resolve(self, expr->paren);
	}

	inline static Type
	typer_expr_resolve(Typer self, Expr expr)
	{
		switch(expr->kind)
		{
		case IExpr::KIND_ATOM: return typer_expr_atom_resolve(self, expr);
		case IExpr::KIND_BINARY: return typer_expr_binary_resolve(self, expr);
		case IExpr::KIND_UNARY: return typer_expr_unary_resolve(self, expr);
		case IExpr::KIND_DOT: return typer_expr_dot_resolve(self, expr);
		case IExpr::KIND_INDEXED: return typer_expr_indexed_resolve(self, expr);
		case IExpr::KIND_CALL: return typer_expr_call_resolve(self, expr);
		case IExpr::KIND_CAST:
			expr->cast.to_type = typer_expr_cast_resolve(self, expr);
			return expr->cast.to_type;
		case IExpr::KIND_PAREN: return typer_expr_paren_resolve(self, expr);
		default: assert(false && "unreachable"); return type_void;
		}
	}


	inline static Type
	typer_stmt_break_resolve(Typer self, Stmt stmt)
	{
		assert(stmt->kind == IStmt::KIND_BREAK);
		if (scope_inside_loop(typer_scope(self)) == false)
			src_err(self->src, err_stmt(stmt, strf("unexpected break statement")));
		return type_void;
	}

	inline static Type
	typer_stmt_continue_resolve(Typer self, Stmt stmt)
	{
		assert(stmt->kind == IStmt::KIND_CONTINUE);
		if (scope_inside_loop(typer_scope(self)) == false)
			src_err(self->src, err_stmt(stmt, strf("unexpected continue statement")));
		return type_void;
	}

	inline static Type
	typer_stmt_return_resolve(Typer self, Stmt stmt)
	{
		assert(stmt->kind == IStmt::KIND_RETURN);
		Type ret = typer_expr_resolve(self, stmt->return_stmt);

		Scope scope = typer_scope(self);
		Type expected = scope_ret(scope);
		if(expected == nullptr)
		{
			src_err(self->src, err_stmt(stmt, strf("unexpected return statement")));
			return ret;
		}

		if (expected != ret)
		{
			src_err(
				self->src,
				err_expr(stmt->return_stmt, strf("wrong return type '{}' expected '{}'", ret, expected))
			);
		}
		return ret;
	}

	inline static Type
	typer_stmt_if_resolve(Typer self, Stmt stmt)
	{
		assert(stmt->kind == IStmt::KIND_IF);
		Type type = typer_expr_resolve(self, stmt->if_stmt.if_cond);
		if(type != type_bool)
		{
			src_err(
				self->src,
				err_expr(stmt->if_stmt.if_cond, strf("if conditions type '{}' is not a boolean", type))
			);
		}
		typer_stmt_resolve(self, stmt->if_stmt.if_body);

		for(const Else_If& e: stmt->if_stmt.else_ifs)
		{
			Type cond_type = typer_expr_resolve(self, e.cond);
			if(cond_type != type_bool)
			{
				src_err(
					self->src,
					err_expr(e.cond, strf("if conditions type '{}' is not a boolean", cond_type))
				);
			}
			typer_stmt_resolve(self, e.body);
		}

		if(stmt->if_stmt.else_body)
			typer_stmt_resolve(self, stmt->if_stmt.else_body);
		return type_void;
	}

	inline static Type
	typer_stmt_for_resolve(Typer self, Stmt stmt)
	{
		assert(stmt->kind == IStmt::KIND_FOR);

		Scope scope = src_scope_new(self->src, stmt, typer_scope(self), true, nullptr);
		typer_scope_enter(self, scope);

		if(stmt->for_stmt.init_stmt)
			typer_stmt_resolve(self, stmt->for_stmt.init_stmt);
		if(stmt->for_stmt.loop_cond)
		{
			Type cond_type = typer_expr_resolve(self, stmt->for_stmt.loop_cond);
			if(cond_type != type_bool)
			{
				src_err(
					self->src,
					err_expr(stmt->for_stmt.loop_cond, strf("for loop condition type '{}' is not a boolean", cond_type))
				);
			}
		}
		if(stmt->for_stmt.post_stmt)
			typer_stmt_resolve(self, stmt->for_stmt.post_stmt);

		for (Stmt s : stmt->for_stmt.loop_body->block_stmt)
			typer_stmt_resolve(self, s);
		typer_scope_leave(self);

		return type_void;
	}

	inline static Type
	typer_stmt_var_resolve(Typer self, Stmt stmt)
	{
		assert(stmt->kind == IStmt::KIND_VAR);

		bool infer = stmt->var_stmt.type.count == 0;

		Type type = type_void;
		if(infer == false)
			type = typer_type_sign_resolve(self, stmt->var_stmt.type);

		for(size_t i = 0; i < stmt->var_stmt.ids.count; ++i)
		{
			Expr e = nullptr;
			if (i < stmt->var_stmt.exprs.count)
				e = stmt->var_stmt.exprs[i];

			Sym s = sym_var(stmt->var_stmt.ids[i], nullptr, stmt->var_stmt.type, e);

			if(infer)
			{
				if(e)
				{
					s->type = typer_expr_resolve(self, e);
				}
				else
				{
					src_err(
						self->src,
						err_tkn(stmt->var_stmt.ids[i], strf("no expression to infer type from"))
					);
				}
			}
			else
			{
				if(e)
				{
					Type expr_type = typer_expr_resolve(self, e);
					if(expr_type != type)
					{
						src_err(
							self->src,
							err_expr(e, strf("type mismatch expected '{}' but found '{}'", type, expr_type))
						);
					}
				}
				s->type = type;
			}

			typer_type_complete(self, s);
			s->state = ISym::STATE_RESOLVED;
			typer_sym(self, s);
		}
		return type_void;
	}

	inline static Type
	typer_stmt_assign_resolve(Typer self, Stmt stmt)
	{
		assert(stmt->kind == IStmt::KIND_ASSIGN);
		for(size_t i = 0; i < stmt->assign_stmt.lhs.count; ++i)
		{
			Type lhs_type = typer_expr_resolve(self, stmt->assign_stmt.lhs[i]);
			if(lhs_type == type_void)
			{
				Str msg = strf("can't assign into a void type");
				src_err(self->src, err_expr(stmt->assign_stmt.lhs[i], msg));
			}

			Type rhs_type = typer_expr_resolve(self, stmt->assign_stmt.rhs[i]);
			if (rhs_type == type_void)
			{
				Str msg = strf("can't assign into a void type");
				src_err(self->src, err_expr(stmt->assign_stmt.rhs[i], msg));
			}

			if(lhs_type != rhs_type)
			{
				Str msg = strf(
					"type mismatch in assignment statement, expected '{}' but found '{}'",
					lhs_type,
					rhs_type
				);
				src_err(self->src, err_expr(stmt->assign_stmt.rhs[i], msg));
			}
		}
		return type_void;
	}

	inline static Type
	typer_stmt_expr_resolve(Typer self, Stmt stmt)
	{
		assert(stmt->kind == IStmt::KIND_EXPR);
		return typer_expr_resolve(self, stmt->expr_stmt);
	}

	inline static Type
	typer_stmt_block_resolve(Typer self, Stmt stmt)
	{
		assert(stmt->kind == IStmt::KIND_BLOCK);
		for(Stmt s: stmt->block_stmt)
			typer_stmt_resolve(self, s);
		return type_void;
	}

	inline static Type
	typer_anonymous_block_resolve(Typer self, Stmt stmt)
	{
		assert(stmt->kind == IStmt::KIND_BLOCK);

		Scope scope = src_scope_new(self->src, stmt, typer_scope(self), false, nullptr);

		typer_scope_enter(self, scope);
		for (Stmt s : stmt->block_stmt)
			typer_stmt_resolve(self, s);
		typer_scope_leave(self);

		return type_void;
	}

	inline static Type
	typer_stmt_resolve(Typer self, Stmt stmt)
	{
		switch(stmt->kind)
		{
		case IStmt::KIND_BREAK: return typer_stmt_break_resolve(self, stmt);
		case IStmt::KIND_CONTINUE: return typer_stmt_continue_resolve(self, stmt);
		case IStmt::KIND_RETURN: return typer_stmt_return_resolve(self, stmt);
		case IStmt::KIND_IF: return typer_stmt_if_resolve(self, stmt);
		case IStmt::KIND_FOR: return typer_stmt_for_resolve(self, stmt);
		case IStmt::KIND_VAR: return typer_stmt_var_resolve(self, stmt);
		case IStmt::KIND_ASSIGN: return typer_stmt_assign_resolve(self, stmt);
		case IStmt::KIND_EXPR: return typer_stmt_expr_resolve(self, stmt);
		case IStmt::KIND_BLOCK: return typer_anonymous_block_resolve(self, stmt);
		default: assert(false && "unreachable"); return type_void;
		}
	}


	inline static Type
	typer_decl_func_resolve(Typer self, Sym sym)
	{
		assert(sym->kind == ISym::KIND_FUNC);
		Decl decl = sym->func_sym;

		Func_Sign sign = func_sign_new();

		for(const Arg& arg: decl->func_decl.args)
		{
			Type type = typer_type_sign_resolve(self, arg.type);
			buf_pushn(sign.args, arg.ids.count, type);
		}

		sign.ret = typer_type_sign_resolve(self, decl->func_decl.ret_type);
		return type_intern_func(self->src->type_table, sign);
	}

	inline static void
	typer_body_func_resolve(Typer self, Sym sym)
	{
		assert(sym->kind == ISym::KIND_FUNC);
		Decl decl = sym->func_sym;

		Scope scope = src_scope_new(self->src, decl, typer_scope(self), false, sym->type->func.ret);

		typer_scope_enter(self, scope);

		//push function args to scope
		size_t i = 0;
		for(const Arg& arg: decl->func_decl.args)
		{
			Type type = sym->type->func.args[i];
			for(const Tkn& id: arg.ids)
			{
				Sym v = sym_var(id, nullptr, arg.type, nullptr);
				v->type = type;
				v->state = ISym::STATE_RESOLVED;
				typer_sym(self, v);
			}
			i += arg.ids.count;
		}

		typer_stmt_block_resolve(self, decl->func_decl.body);

		typer_scope_leave(self);
	}

	inline static Type
	typer_decl_var_resolve(Typer self, Sym sym)
	{
		assert(sym->kind == ISym::KIND_VAR);
		bool infer = sym->var_sym.type.count == 0;

		Type type = type_void;
		if(infer == false)
			type = typer_type_sign_resolve(self, sym->var_sym.type);

		Expr e = sym->var_sym.expr;

		if(infer)
		{
			if(e)
			{
				sym->type = typer_expr_resolve(self, e);
			}
			else
			{
				src_err(
					self->src,
					err_tkn(sym->var_sym.id, strf("no expression to infer type from"))
				);
			}
		}
		else
		{
			if(e)
			{
				Type expr_type = typer_expr_resolve(self, e);
				if(expr_type != type)
				{
					src_err(
						self->src,
						err_expr(e, strf("type mismatch expected '{}' but found '{}'", type, expr_type))
					);
				}
			}
			sym->type = type;
		}

		typer_type_complete(self, sym);
		sym->state = ISym::STATE_RESOLVED;
		return sym->type;
	}

	inline static void
	typer_sym_resolve(Typer self, Sym sym)
	{
		if(sym->state == ISym::STATE_RESOLVED)
		{
			return;
		}
		else if(sym->state == ISym::STATE_RESOLVING)
		{
			Tkn id = sym_tkn(sym);
			src_err(self->src, err_tkn(id, strf("'{}' symbol cyclic dependency", id.str)));
			return;
		}

		assert(sym->state == ISym::STATE_UNRESOLVED);
		sym->state = ISym::STATE_RESOLVING;
		switch(sym->kind)
		{
		case ISym::KIND_STRUCT:
			sym->type = type_intern_incomplete(self->src->type_table, type_incomplete_aggregate(sym));
			break;
		case ISym::KIND_UNION:
			sym->type = type_intern_incomplete(self->src->type_table, type_incomplete_aggregate(sym));
			break;
		case ISym::KIND_ENUM:
			sym->type = type_intern_incomplete(self->src->type_table, type_incomplete_enum(sym));
			break;
		case ISym::KIND_VAR:
			typer_decl_var_resolve(self, sym);
			break;
		case ISym::KIND_FUNC:
			sym->type = typer_decl_func_resolve(self, sym);
			break;

		default:
			break;
		}
		sym->state = ISym::STATE_RESOLVED;

		switch(sym->kind)
		{
		case ISym::KIND_STRUCT:
		case ISym::KIND_UNION:
		case ISym::KIND_ENUM:
			typer_type_complete(self, sym);
			break;
		case ISym::KIND_FUNC:
			typer_body_func_resolve(self, sym);
			break;
		case ISym::KIND_VAR:
			//do nothing
			break;
		default:
			assert(false && "unreachable");
			break;
		}
		buf_push(self->src->reachable_syms, sym);
	}


	//API
	Typer
	typer_new(Src src, ITyper::MODE mode)
	{
		Typer self = alloc<ITyper>();
		self->mode = mode;
		self->src = src;
		self->scope_stack = buf_new<Scope>();
		self->global_scope = src_scope_new(self->src, nullptr, nullptr, false, nullptr);

		typer_scope_enter(self, self->global_scope);
		return self;
	}

	void
	typer_free(Typer self)
	{
		buf_free(self->scope_stack);
		free(self);
	}

	void
	typer_check(Typer self)
	{
		typer_shallow_walk(self);

		if (self->mode == ITyper::MODE_EXE)
		{
			const char* main = str_intern(self->src->str_table, "main");
			Sym main_sym = nullptr;
			for (Sym sym : self->global_scope->syms)
			{
				if (sym->name == main)
				{
					main_sym = sym;
					break;
				}
			}

			if(main_sym == nullptr)
			{
				src_err(self->src, err_str(strf("program doesn't have a main function")));
				return;
			}

			typer_sym_resolve(self, main_sym);
		}
		else
		{
			for (Sym sym : self->global_scope->syms)
				typer_sym_resolve(self, sym);
		}
	}
}