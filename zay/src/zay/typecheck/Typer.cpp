#include "zay/typecheck/Typer.h"

#include <mn/Memory.h>
#include <mn/IO.h>

#include <assert.h>

namespace zay
{
	inline static void
	typer_sym_resolve(Typer& self, Sym* sym);

	inline static Type*
	typer_expr_resolve(Typer& self, Expr* expr);

	inline static Type*
	typer_stmt_resolve(Typer& self, Stmt* stmt);

	inline static Type*
	typer_stmt_block_resolve(Typer& self, Stmt* stmt);

	inline static void
	typer_type_complete(Typer& self, Sym* sym);

	inline static void
	typer_scope_enter(Typer& self, Scope* scope)
	{
		mn::buf_push(self.scope_stack, scope);
	}

	inline static void
	typer_scope_leave(Typer& self)
	{
		mn::buf_pop(self.scope_stack);
	}

	inline static Scope*
	typer_scope(Typer& self)
	{
		return mn::buf_top(self.scope_stack);
	}

	inline static Sym*
	typer_sym(Typer& self, Sym* sym)
	{
		auto scope = typer_scope(self);
		if(auto old = scope_has(scope, sym->name))
		{
			Tkn new_tkn = sym_tkn(sym);
			Tkn old_tkn = sym_tkn(old);
			auto msg = mn::strf(
				"'{}' symbol redefinition, it was first defined {}:{}",
				new_tkn.str,
				old_tkn.pos.line,
				old_tkn.pos.col
			);
			src_err(self.src, err_tkn(new_tkn, msg));
			sym_free(sym);
			return nullptr;
		}

		scope_add(scope, sym);
		return sym;
	}

	inline static void
	typer_shallow_walk(Typer& self)
	{
		for(Decl* d: self.src->ast.decls)
		{
			switch(d->kind)
			{
			case Decl::KIND_VAR:
				for (size_t i = 0; i < d->var_decl.ids.count; ++i)
				{
					Expr* e = nullptr;
					if (i < d->var_decl.exprs.count)
						e = d->var_decl.exprs[i];
					typer_sym(self, sym_var(d->var_decl.ids[i], d, d->var_decl.type, e));
				}
				break;
			case Decl::KIND_TYPE:
				typer_sym(self, sym_type(d));
				break;
			case Decl::KIND_FUNC:
				typer_sym(self, sym_func(d));
				break;
			default: assert(false && "unreachable"); break;
			}
		}
	}

	inline static Sym*
	typer_sym_by_name(Typer& self, const char* name)
	{
		return scope_find(typer_scope(self), name);
	}

	inline static Type*
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

	inline static Type*
	typer_type_sign_resolve(Typer& self, const Type_Sign& sign, Type* incomplete_type)
	{
		Type* res = type_void;

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
				if(type_is_same(res, type_void))
				{
					if(auto sym = typer_sym_by_name(self, atom.named.str))
					{
						//so we make sure we did resolve this symbol
						typer_sym_resolve(self, sym);
						//and whatever the type of this symbol is the type of this named atom
						res = sym->type;
					}
					else
					{
						src_err(
							self.src,
							err_tkn(atom.named, mn::strf("'{}' undefined symbol", atom.named.str))
						);
					}
				}

				if (incomplete_type)
				{
					type_alias_complete(incomplete_type, res);
					res = incomplete_type;
				}

				break;
			case Type_Atom::KIND_PTR:
				res = type_intern_ptr(self.src->type_table, res);
				break;
			case Type_Atom::KIND_ARRAY:
			{
				size_t array_count = 0;
				if (atom.count.kind == Tkn::KIND_INTEGER)
					mn::reads(atom.count.str, array_count);
				res = type_intern_array(self.src->type_table, Array_Sign{ res, array_count });
				break;
			}
			case Type_Atom::KIND_STRUCT:
			{
				//this is not a named type, so generate a name for it and put it in global scope
				if(incomplete_type == nullptr)
				{
					auto name = mn::str_tmpf("__unnamed_struct_{}", self.unnamed_id++);
					Tkn unnamed_id = tkn_anonymous_id(str_intern(self.src->str_table, name));
					Decl* unnamed_decl = decl_type(unnamed_id, clone(sign));
					mn::buf_push(self.src->ast.decls, unnamed_decl);
					auto unnamed_sym = sym_type(unnamed_decl);
					scope_add(self.global_scope, unnamed_sym);
					typer_sym_resolve(self, unnamed_sym);

					incomplete_type = type_intern_incomplete(self.src->type_table, type_incomplete(unnamed_sym));
					incomplete_type->kind = Type::KIND_COMPLETING;
				}
				res = incomplete_type;

				auto fields = mn::buf_new<Field_Sign>();
				for(size_t j = 0; j < atom.struct_fields.count; ++j)
				{
					Type* field_type = typer_type_sign_resolve(self, atom.struct_fields[j].type, nullptr);
					if (field_type->kind == Type::KIND_COMPLETING ||
						field_type->kind == Type::KIND_INCOMPLETE)
					{
						typer_type_complete(self, field_type->sym);
					}

					for(size_t k = 0; k < atom.struct_fields[j].ids.count; ++k)
					{
						mn::buf_push(fields, Field_Sign{
							atom.struct_fields[j].ids[k].str,
							field_type,
							0 //set the offset here to be 0 for now
						});
					}
				}
				type_struct_complete(res, fields);
				break;
			}
			case Type_Atom::KIND_UNION:
			{
				//this is not a named type, so generate a name for it and put it in global scope
				if(incomplete_type == nullptr)
				{
					auto name = mn::str_tmpf("__unnamed_union_{}", self.unnamed_id++);
					Tkn unnamed_id = tkn_anonymous_id(str_intern(self.src->str_table, name));
					Decl* unnamed_decl = decl_type(unnamed_id, clone(sign));
					mn::buf_push(self.src->ast.decls, unnamed_decl);
					auto unnamed_sym = sym_type(unnamed_decl);
					scope_add(self.global_scope, unnamed_sym);
					typer_sym_resolve(self, unnamed_sym);

					incomplete_type = type_intern_incomplete(self.src->type_table, type_incomplete(unnamed_sym));
					incomplete_type->kind = Type::KIND_COMPLETING;
				}
				res = incomplete_type;

				auto fields = mn::buf_new<Field_Sign>();
				for(size_t j = 0; j < atom.union_fields.count; ++j)
				{
					Type* field_type = typer_type_sign_resolve(self, atom.union_fields[j].type, nullptr);
					if (field_type->kind == Type::KIND_COMPLETING ||
						field_type->kind == Type::KIND_INCOMPLETE)
					{
						typer_type_complete(self, field_type->sym);
					}

					for(size_t k = 0; k < atom.union_fields[j].ids.count; ++k)
					{
						mn::buf_push(fields, Field_Sign{
							atom.union_fields[j].ids[k].str,
							field_type,
							0 //set the offset here to be 0 for now
						});
					}
				}
				type_union_complete(res, fields);
				break;
			}
			case Type_Atom::KIND_ENUM:
			{
				//this is not a named type, so generate a name for it and put it in global scope
				if(incomplete_type == nullptr)
				{
					auto name = mn::str_tmpf("__unnamed_enum_{}", self.unnamed_id++);
					Tkn unnamed_id = tkn_anonymous_id(str_intern(self.src->str_table, name));
					Decl* unnamed_decl = decl_type(unnamed_id, clone(sign));
					mn::buf_push(self.src->ast.decls, unnamed_decl);
					auto unnamed_sym = sym_type(unnamed_decl);
					scope_add(self.global_scope, unnamed_sym);
					typer_sym_resolve(self, unnamed_sym);

					incomplete_type = type_intern_incomplete(self.src->type_table, type_incomplete(unnamed_sym));
					incomplete_type->kind = Type::KIND_COMPLETING;
				}
				res = incomplete_type;

				auto values = mn::buf_new<Enum_Value>();
				for(size_t j = 0; j < atom.enum_fields.count; ++j)
				{
					Enum_Value v{ atom.enum_fields[j].id, atom.enum_fields[j].expr };
					if(v.value)
					{
						Type* value_type = typer_expr_resolve(self, v.value);
						if(type_is_integer(value_type) == false)
						{
							src_err(
								self.src,
								err_expr(v.value, mn::strf("enums should have int values but found '{}'", *value_type))
							);
						}
					}
					mn::buf_push(values, v);
				}
				type_enum_complete(res, values);
				break;
			}
			case Type_Atom::KIND_FUNC:
			{
				Func_Sign func_sign = func_sign_new();
				for(const Type_Sign& arg: atom.func.args)
					mn::buf_push(func_sign.args, typer_type_sign_resolve(self, arg, nullptr));
				func_sign.ret = typer_type_sign_resolve(self, atom.func.ret, nullptr);
				res = type_intern_func(self.src->type_table, func_sign);
				break;
			}
			default:
				assert(false && "unreachable");
				break;
			}
		}

		return res;
	}

	inline static void
	typer_type_complete(Typer& self, Sym* sym)
	{
		Type* type = sym->type;
		if(type->kind == Type::KIND_COMPLETING)
		{
			src_err(self.src, err_tkn(sym_tkn(sym), mn::strf("'{}' recursive type", sym->name)));
			return;
		}
		else if(type->kind != Type::KIND_INCOMPLETE)
		{
			return;
		}

		type->kind = Type::KIND_COMPLETING;
		if(sym->kind == Sym::KIND_TYPE)
		{
			Decl* decl = sym->type_sym;
			sym->type = typer_type_sign_resolve(self, decl->type_decl, sym->type);
		}
		else 
		{
			assert(false && "unreachable");
		}
	}


	//expressions
	inline static Type*
	typer_expr_atom_resolve(Typer& self, Expr* expr)
	{
		assert(expr->kind == Expr::KIND_ATOM);
		switch(expr->atom.kind)
		{
		case Tkn::KIND_INTEGER: return type_lit_int;
		case Tkn::KIND_FLOAT: return type_lit_float64;
		case Tkn::KIND_KEYWORD_STRING: return type_string;
		case Tkn::KIND_KEYWORD_FALSE:
		case Tkn::KIND_KEYWORD_TRUE:
			return type_bool;
		case Tkn::KIND_ID:
			if(auto sym = typer_sym_by_name(self, expr->atom.str))
			{
				typer_sym_resolve(self, sym);
				return sym->type;
			}
			src_err(self.src, err_expr(expr, mn::strf("'{}' undefined symbol", expr->atom.str)));
			return type_void;
		default: assert(false && "unreachable"); return type_void;
		}
	}

	inline static Type*
	typer_expr_binary_resolve(Typer& self, Expr* expr)
	{
		assert(expr->kind == Expr::KIND_BINARY);

		Type* lhs_type = typer_expr_resolve(self, expr->binary.lhs);
		Type* rhs_type = typer_expr_resolve(self, expr->binary.rhs);

		if(type_is_same(lhs_type, rhs_type) == false)
			src_err(self.src, err_expr(expr, mn::strf("type mismatch in binary expression")));

		if(expr->binary.op.kind == Tkn::KIND_LOGIC_AND || expr->binary.op.kind == Tkn::KIND_LOGIC_OR)
		{
			if(type_is_same(lhs_type, type_bool) == false)
			{
				src_err(
					self.src,
					err_expr(expr->binary.lhs, mn::strf("logical operator only work on boolean types"))
				);
			}

			if(type_is_same(rhs_type, type_bool) == false)
			{
				src_err(
					self.src,
					err_expr(expr->binary.rhs, mn::strf("logical operator only work on boolean types"))
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

	inline static Type*
	typer_expr_unary_resolve(Typer& self, Expr* expr)
	{
		assert(expr->kind == Expr::KIND_UNARY);

		Type* type = typer_expr_resolve(self, expr->unary.expr);

		//works with numbers
		if (expr->unary.op.kind == Tkn::KIND_INC ||
			expr->unary.op.kind == Tkn::KIND_DEC ||
			expr->unary.op.kind == Tkn::KIND_PLUS ||
			expr->unary.op.kind == Tkn::KIND_MINUS)
		{
			if(type_is_numeric(type) == false)
			{
				src_err(
					self.src,
					err_expr(expr->unary.expr, mn::strf("'{}' is only allowed for numeric types", expr->unary.op.str))
				);
			}
		}
		else if(expr->unary.op.kind == Tkn::KIND_LOGIC_NOT)
		{
			if(type_is_same(type, type_bool) == false)
			{
				src_err(
					self.src,
					err_expr(expr, mn::strf("logical not operator is only allowed for boolean types"))
				);
			}
		}
		else if(expr->unary.op.kind == Tkn::KIND_BIT_AND)
		{
			return type_intern_ptr(self.src->type_table, type);
		}
		else if(expr->unary.op.kind == Tkn::KIND_STAR)
		{
			if(type->kind != Type::KIND_PTR)
			{
				src_err(
					self.src,
					err_expr(expr->unary.expr, mn::strf("cannot dereference non pointer type"))
				);
			}
			else
			{
				return type->ptr.base;
			}
			
		}
		return type;
	}

	inline static Type*
	type_unqualify(Type* t)
	{
		if (t->kind == Type::KIND_PTR)
			return t->ptr.base;
		return t;
	}

	inline static Type*
	typer_expr_dot_resolve(Typer& self, Expr* expr)
	{
		assert(expr->kind == Expr::KIND_DOT);
		Type* type = typer_expr_resolve(self, expr->dot.base);
		Type* unqualified_type = type_unqualify(type);
		Type* res = type_void;
		if(unqualified_type->kind == Type::KIND_STRUCT)
		{
			for(const Field_Sign& f: unqualified_type->fields)
			{
				if(f.name == expr->dot.member.str)
				{
					res = f.type;
					break;
				}
			}
			if(type_is_same(res, type_void))
			{
				src_err(self.src, err_tkn(expr->dot.member, mn::strf("undefined struct field")));
			}
		}
		else if (unqualified_type->kind == Type::KIND_UNION)
		{
			for (const Field_Sign& f : unqualified_type->fields)
			{
				if (f.name == expr->dot.member.str)
				{
					res = f.type;
					break;
				}
			}
			if (type_is_same(res, type_void))
			{
				src_err(self.src, err_tkn(expr->dot.member, mn::strf("undefined union field")));
			}
		}
		else if (unqualified_type->kind == Type::KIND_ENUM)
		{
			for (const Enum_Value& v : unqualified_type->enum_values)
			{
				if (v.id.str == expr->dot.member.str)
				{
					res = unqualified_type;
					break;
				}
			}
			if (type_is_same(res, type_void))
			{
				src_err(self.src, err_tkn(expr->dot.member, mn::strf("undefined enum field")));
			}
		}
		return res;
	}

	inline static Type*
	typer_expr_indexed_resolve(Typer& self, Expr* expr)
	{
		assert(expr->kind == Expr::KIND_INDEXED);
		Type* type = typer_expr_resolve(self, expr->indexed.base);
		if(type->kind != Type::KIND_ARRAY)
		{
			src_err(
				self.src,
				err_expr(expr->indexed.base, mn::strf("expression type '{}' is not an array", *type))
			);
			return type_void;
		}

		if(type_is_integer(type) == false)
		{
			src_err(
				self.src,
				err_expr(expr->indexed.index, mn::strf("index expression type '{}' is not an integer", *type))
			);
		}
		return type->array.base;
	}

	inline static Type*
	typer_expr_call_resolve(Typer& self, Expr* expr)
	{
		assert(expr->kind == Expr::KIND_CALL);
		Type* res = typer_expr_resolve(self, expr->call.base);
		if(res->kind != Type::KIND_FUNC)
		{
			src_err(self.src, err_expr(expr->call.base, mn::strf("invalid call, expression is not a function")));
			return type_void;
		}

		if(expr->call.args.count != res->func.args.count)
		{
			auto msg = mn::strf(
				"function expected {} arguments but {} were provided",
				res->func.args.count,
				expr->call.args.count
			);
			src_err(self.src, err_expr(expr, msg));
			return type_void;
		}

		for(size_t i = 0; i < expr->call.args.count; ++i)
		{
			Type* type = typer_expr_resolve(self, expr->call.args[i]);
			if(type_is_same(type, res->func.args[i]) == false)
			{
				auto msg = mn::strf("function argument {} type mismatch", i);
				src_err(self.src, err_expr(expr->call.args[i], msg));
			}
		}
		return res->func.ret;
	}

	inline static Type*
	typer_expr_cast_resolve(Typer& self, Expr* expr)
	{
		assert(expr->kind == Expr::KIND_CAST);
		Type* from_type = typer_expr_resolve(self, expr->cast.base);
		Type* to_type = typer_type_sign_resolve(self, expr->cast.type, nullptr);

		from_type = type_unwrap(from_type);
		to_type = type_unwrap(to_type);

		if(type_is_numeric(from_type) && type_is_numeric(to_type))
			return to_type;
		else if(from_type->kind == Type::KIND_PTR && to_type->kind == Type::KIND_PTR)
			return to_type;

		src_err(
			self.src,
			err_expr(expr, mn::strf("can't cast '{}' to '{}'", *from_type, *to_type))
		);
		return type_void;
	}

	inline static Type*
	typer_expr_paren_resolve(Typer& self, Expr* expr)
	{
		assert(expr->kind == Expr::KIND_PAREN);
		return typer_expr_resolve(self, expr->paren);
	}

	inline static Type*
	typer_expr_complit_resolve(Typer& self, Expr* expr)
	{
		assert(expr->kind == Expr::KIND_COMPLIT);
		Type* type = typer_type_sign_resolve(self, expr->complit.type, nullptr);
		for(size_t i = 0; i < expr->complit.fields.count; ++i)
		{
			Complit_Field field = expr->complit.fields[i];
			Type* left_type = type_void;
			if(field.kind == Complit_Field::KIND_MEMBER)
			{
				if (type->kind == Type::KIND_STRUCT ||
					type->kind == Type::KIND_UNION)
				{
					for(size_t j = 0; j < type->fields.count; ++j)
					{
						if(field.left->atom.str == type->fields[j].name)
						{
							left_type = type->fields[j].type;
							break;
						}
					}
				}
				if (type_is_same(left_type, type_void))
				{
					src_err(
						self.src,
						err_expr(field.left, mn::strf("'{}' type doesn't have this field", *type))
					);
				}
			}
			else if(field.kind == Complit_Field::KIND_ARRAY)
			{
				if(type->kind == Type::KIND_ARRAY)
				{
					left_type = type->array.base;
				}
				else
				{
					src_err(
						self.src,
						err_expr(field.left, mn::strf("'{}' type is not an array", *type))
					);
				}
			}

			Type* right_type = typer_expr_resolve(self, field.right);

			if(type_is_same(left_type, right_type) == false)
			{
				auto msg = mn::strf("type mismatch, type '{}' expected but type '{}' was provided", *left_type, *right_type);
				src_err(self.src, err_expr(field.right, msg));
			}
		}
		return type;
	}

	inline static Type*
	typer_expr_resolve(Typer& self, Expr* expr)
	{
		switch(expr->kind)
		{
		case Expr::KIND_ATOM:
			expr->type = typer_expr_atom_resolve(self, expr);
			return expr->type;
		case Expr::KIND_BINARY:
			expr->type = typer_expr_binary_resolve(self, expr);
			return expr->type;
		case Expr::KIND_UNARY:
			expr->type = typer_expr_unary_resolve(self, expr);
			return expr->type;
		case Expr::KIND_DOT:
			expr->type = typer_expr_dot_resolve(self, expr);
			return expr->type;
		case Expr::KIND_INDEXED:
			expr->type = typer_expr_indexed_resolve(self, expr);
			return expr->type;
		case Expr::KIND_CALL:
			expr->type = typer_expr_call_resolve(self, expr);
			return expr->type;
		case Expr::KIND_CAST:
			expr->type = typer_expr_cast_resolve(self, expr);
			return expr->type;
		case Expr::KIND_PAREN:
			expr->type = typer_expr_paren_resolve(self, expr);
			return expr->type;
		case Expr::KIND_COMPLIT:
			expr->type = typer_expr_complit_resolve(self, expr);
			return expr->type;
		default: assert(false && "unreachable"); return type_void;
		}
	}


	inline static Type*
	typer_stmt_break_resolve(Typer& self, Stmt* stmt)
	{
		assert(stmt->kind == Stmt::KIND_BREAK);
		if (scope_inside_loop(typer_scope(self)) == false)
			src_err(self.src, err_stmt(stmt, mn::strf("unexpected break statement")));
		return type_void;
	}

	inline static Type*
	typer_stmt_continue_resolve(Typer& self, Stmt* stmt)
	{
		assert(stmt->kind == Stmt::KIND_CONTINUE);
		if (scope_inside_loop(typer_scope(self)) == false)
			src_err(self.src, err_stmt(stmt, mn::strf("unexpected continue statement")));
		return type_void;
	}

	inline static Type*
	typer_stmt_return_resolve(Typer& self, Stmt* stmt)
	{
		assert(stmt->kind == Stmt::KIND_RETURN);
		Type* ret = typer_expr_resolve(self, stmt->return_stmt);

		auto scope = typer_scope(self);
		Type* expected = scope_ret(scope);
		if(expected == nullptr)
		{
			src_err(self.src, err_stmt(stmt, mn::strf("unexpected return statement")));
			return ret;
		}

		if (type_is_same(expected, ret) == false)
		{
			src_err(
				self.src,
				err_expr(stmt->return_stmt, mn::strf("wrong return type '{}' expected '{}'", *ret, *expected))
			);
		}
		return ret;
	}

	inline static Type*
	typer_stmt_if_resolve(Typer& self, Stmt* stmt)
	{
		assert(stmt->kind == Stmt::KIND_IF);
		Type* type = typer_expr_resolve(self, stmt->if_stmt.if_cond);
		if(type_is_same(type, type_bool) == false)
		{
			src_err(
				self.src,
				err_expr(stmt->if_stmt.if_cond, mn::strf("if conditions type '{}' is not a boolean", *type))
			);
		}
		typer_stmt_resolve(self, stmt->if_stmt.if_body);

		for(const Else_If& e: stmt->if_stmt.else_ifs)
		{
			Type* cond_type = typer_expr_resolve(self, e.cond);
			if(type_is_same(cond_type, type_bool) == false)
			{
				src_err(
					self.src,
					err_expr(e.cond, mn::strf("if conditions type '{}' is not a boolean", *cond_type))
				);
			}
			typer_stmt_resolve(self, e.body);
		}

		if(stmt->if_stmt.else_body)
			typer_stmt_resolve(self, stmt->if_stmt.else_body);
		return type_void;
	}

	inline static Type*
	typer_stmt_for_resolve(Typer& self, Stmt* stmt)
	{
		assert(stmt->kind == Stmt::KIND_FOR);

		auto scope = src_scope_new(self.src, stmt, typer_scope(self), true, nullptr);
		typer_scope_enter(self, scope);

		if(stmt->for_stmt.init_stmt)
			typer_stmt_resolve(self, stmt->for_stmt.init_stmt);
		if(stmt->for_stmt.loop_cond)
		{
			Type* cond_type = typer_expr_resolve(self, stmt->for_stmt.loop_cond);
			if(type_is_same(cond_type, type_bool) == false)
			{
				src_err(
					self.src,
					err_expr(stmt->for_stmt.loop_cond, mn::strf("for loop condition type '{}' is not a boolean", *cond_type))
				);
			}
		}
		if(stmt->for_stmt.post_stmt)
			typer_stmt_resolve(self, stmt->for_stmt.post_stmt);

		for (Stmt* s : stmt->for_stmt.loop_body->block_stmt)
			typer_stmt_resolve(self, s);
		typer_scope_leave(self);

		return type_void;
	}

	inline static Type*
	typer_stmt_var_resolve(Typer& self, Stmt* stmt)
	{
		assert(stmt->kind == Stmt::KIND_VAR);

		bool infer = stmt->var_stmt.type.count == 0;

		Type* type = type_void;
		if(infer == false)
			type = typer_type_sign_resolve(self, stmt->var_stmt.type, nullptr);

		for(size_t i = 0; i < stmt->var_stmt.ids.count; ++i)
		{
			Expr* e = nullptr;
			if (i < stmt->var_stmt.exprs.count)
				e = stmt->var_stmt.exprs[i];

			auto s = sym_var(stmt->var_stmt.ids[i], nullptr, stmt->var_stmt.type, e);

			if(infer)
			{
				if(e)
				{
					s->type = typer_expr_resolve(self, e);
				}
				else
				{
					src_err(
						self.src,
						err_tkn(stmt->var_stmt.ids[i], mn::strf("no expression to infer type from"))
					);
				}
			}
			else
			{
				if(e)
				{
					Type* expr_type = typer_expr_resolve(self, e);
					if(type_is_same(type_unwrap(expr_type), type_unwrap(type)) == false)
					{
						src_err(
							self.src,
							err_expr(e, mn::strf("type mismatch expected '{}' but found '{}'", *type, *expr_type))
						);
					}
				}
				s->type = type;
			}

			typer_type_complete(self, s);
			s->state = Sym::STATE_RESOLVED;
			typer_sym(self, s);
		}
		return type_void;
	}

	inline static Type*
	typer_stmt_assign_resolve(Typer& self, Stmt* stmt)
	{
		assert(stmt->kind == Stmt::KIND_ASSIGN);
		for(size_t i = 0; i < stmt->assign_stmt.lhs.count; ++i)
		{
			Type* lhs_type = typer_expr_resolve(self, stmt->assign_stmt.lhs[i]);
			if(type_is_same(lhs_type, type_void))
			{
				auto msg = mn::strf("can't assign into a void type");
				src_err(self.src, err_expr(stmt->assign_stmt.lhs[i], msg));
			}

			Type* rhs_type = typer_expr_resolve(self, stmt->assign_stmt.rhs[i]);
			if (type_is_same(rhs_type, type_void))
			{
				auto msg = mn::strf("can't assign into a void type");
				src_err(self.src, err_expr(stmt->assign_stmt.rhs[i], msg));
			}

			if(type_is_same(lhs_type, rhs_type) == false)
			{
				auto msg = mn::strf(
					"type mismatch in assignment statement, expected '{}' but found '{}'",
					*lhs_type,
					*rhs_type
				);
				src_err(self.src, err_expr(stmt->assign_stmt.rhs[i], msg));
			}
		}
		return type_void;
	}

	inline static Type*
	typer_stmt_expr_resolve(Typer& self, Stmt* stmt)
	{
		assert(stmt->kind == Stmt::KIND_EXPR);
		return typer_expr_resolve(self, stmt->expr_stmt);
	}

	inline static Type*
	typer_stmt_block_resolve(Typer& self, Stmt* stmt)
	{
		assert(stmt->kind == Stmt::KIND_BLOCK);
		for(Stmt* s: stmt->block_stmt)
			typer_stmt_resolve(self, s);
		return type_void;
	}

	inline static Type*
	typer_anonymous_block_resolve(Typer& self, Stmt* stmt)
	{
		assert(stmt->kind == Stmt::KIND_BLOCK);

		auto scope = src_scope_new(self.src, stmt, typer_scope(self), false, nullptr);

		typer_scope_enter(self, scope);
		for (Stmt* s : stmt->block_stmt)
			typer_stmt_resolve(self, s);
		typer_scope_leave(self);

		return type_void;
	}

	inline static Type*
	typer_stmt_resolve(Typer& self, Stmt* stmt)
	{
		switch(stmt->kind)
		{
		case Stmt::KIND_BREAK: return typer_stmt_break_resolve(self, stmt);
		case Stmt::KIND_CONTINUE: return typer_stmt_continue_resolve(self, stmt);
		case Stmt::KIND_RETURN: return typer_stmt_return_resolve(self, stmt);
		case Stmt::KIND_IF: return typer_stmt_if_resolve(self, stmt);
		case Stmt::KIND_FOR: return typer_stmt_for_resolve(self, stmt);
		case Stmt::KIND_VAR: return typer_stmt_var_resolve(self, stmt);
		case Stmt::KIND_ASSIGN: return typer_stmt_assign_resolve(self, stmt);
		case Stmt::KIND_EXPR: return typer_stmt_expr_resolve(self, stmt);
		case Stmt::KIND_BLOCK: return typer_anonymous_block_resolve(self, stmt);
		default: assert(false && "unreachable"); return type_void;
		}
	}


	//terminate functions
	inline static bool
	typer_is_terminating(Typer& self, Stmt* stmt)
	{
		switch(stmt->kind)
		{
		case Stmt::KIND_BLOCK:
			if (stmt->block_stmt.count == 0)
				return false;
			return typer_is_terminating(self, mn::buf_top(stmt->block_stmt));
		case Stmt::KIND_RETURN:
			return true;
		case Stmt::KIND_FOR:
			return typer_is_terminating(self, stmt->for_stmt.loop_body);
		case Stmt::KIND_IF:
		{
			bool res = typer_is_terminating(self, stmt->if_stmt.if_body);
			for (const Else_If& f : stmt->if_stmt.else_ifs)
				res &= typer_is_terminating(self, f.body);
			if (stmt->if_stmt.else_body)
				res &= typer_is_terminating(self, stmt->if_stmt.else_body);
			return res;
		}
		default:
			return false;
		}
	}


	inline static Type*
	typer_decl_func_resolve(Typer& self, Sym* sym)
	{
		assert(sym->kind == Sym::KIND_FUNC);
		Decl* decl = sym->func_sym;

		Func_Sign sign = func_sign_new();

		for(const Arg& arg: decl->func_decl.args)
		{
			Type* type = typer_type_sign_resolve(self, arg.type, nullptr);
			buf_pushn(sign.args, arg.ids.count, type);
		}

		sign.ret = typer_type_sign_resolve(self, decl->func_decl.ret_type, nullptr);
		return type_intern_func(self.src->type_table, sign);
	}

	inline static void
	typer_body_func_resolve(Typer& self, Sym* sym)
	{
		assert(sym->kind == Sym::KIND_FUNC);
		Decl* decl = sym->func_sym;

		auto scope = src_scope_new(self.src, decl, typer_scope(self), false, sym->type->func.ret);

		typer_scope_enter(self, scope);

		//push function args to scope
		size_t i = 0;
		for(const Arg& arg: decl->func_decl.args)
		{
			Type* type = sym->type->func.args[i];
			for(const Tkn& id: arg.ids)
			{
				auto v = sym_var(id, nullptr, arg.type, nullptr);
				v->type = type;
				v->state = Sym::STATE_RESOLVED;
				typer_sym(self, v);
			}
			i += arg.ids.count;
		}

		// typecheck the function body if it exists
		if (decl->func_decl.body)
		{
			typer_stmt_block_resolve(self, decl->func_decl.body);

			if (type_is_same(sym->type->func.ret, type_void) == false &&
				typer_is_terminating(self, decl->func_decl.body) == false)
			{
				src_err(
					self.src,
					err_decl(decl, mn::strf("missing return at the end of the function"))
				);
			}
		}

		typer_scope_leave(self);
	}

	inline static Type*
	typer_decl_var_resolve(Typer& self, Sym* sym)
	{
		assert(sym->kind == Sym::KIND_VAR);
		bool infer = sym->var_sym.type.count == 0;

		Type* type = type_void;
		if(infer == false)
			type = typer_type_sign_resolve(self, sym->var_sym.type, nullptr);

		Expr* e = sym->var_sym.expr;

		if(infer)
		{
			if(e)
			{
				sym->type = typer_expr_resolve(self, e);
			}
			else
			{
				src_err(
					self.src,
					err_tkn(sym->var_sym.id, mn::strf("no expression to infer type from"))
				);
			}
		}
		else
		{
			if(e)
			{
				Type* expr_type = typer_expr_resolve(self, e);
				if(type_is_same(expr_type, type) == false)
				{
					src_err(
						self.src,
						err_expr(e, mn::strf("type mismatch expected '{}' but found '{}'", *type, *expr_type))
					);
				}
			}
			sym->type = type;
		}

		typer_type_complete(self, sym);
		sym->state = Sym::STATE_RESOLVED;
		return sym->type;
	}

	inline static void
	typer_sym_resolve(Typer& self, Sym* sym)
	{
		if(sym->state == Sym::STATE_RESOLVED)
		{
			return;
		}
		else if(sym->state == Sym::STATE_RESOLVING)
		{
			Tkn id = sym_tkn(sym);
			src_err(self.src, err_tkn(id, mn::strf("'{}' symbol cyclic dependency", id.str)));
			return;
		}

		assert(sym->state == Sym::STATE_UNRESOLVED);
		sym->state = Sym::STATE_RESOLVING;
		switch(sym->kind)
		{
		case Sym::KIND_STRUCT:
		case Sym::KIND_UNION:
		case Sym::KIND_ENUM:
		case Sym::KIND_TYPE:
			sym->type = type_intern_incomplete(self.src->type_table, type_incomplete(sym));
			break;

		case Sym::KIND_VAR:
			typer_decl_var_resolve(self, sym);
			break;
		case Sym::KIND_FUNC:
			sym->type = typer_decl_func_resolve(self, sym);
			break;

		default:
			break;
		}
		sym->state = Sym::STATE_RESOLVED;

		switch(sym->kind)
		{
		case Sym::KIND_STRUCT:
		case Sym::KIND_UNION:
		case Sym::KIND_ENUM:
		case Sym::KIND_TYPE:
			typer_type_complete(self, sym);
			break;
		case Sym::KIND_FUNC:
			typer_body_func_resolve(self, sym);
			break;
		case Sym::KIND_VAR:
			//do nothing
			break;
		default:
			assert(false && "unreachable");
			break;
		}
		mn::buf_push(self.src->reachable_syms, sym);
	}


	//API
	Typer
	typer_new(Src *src, Typer::MODE mode)
	{
		Typer self{};
		self.mode = mode;
		self.src = src;
		self.scope_stack = mn::buf_new<Scope*>();
		self.global_scope = src_scope_new(self.src, nullptr, nullptr, false, nullptr);
		self.unnamed_id = 0;

		typer_scope_enter(self, self.global_scope);
		return self;
	}

	void
	typer_free(Typer& self)
	{
		buf_free(self.scope_stack);
	}

	void
	typer_check(Typer& self)
	{
		typer_shallow_walk(self);

		if (self.mode == Typer::MODE_EXE)
		{
			const char* main = str_intern(self.src->str_table, "main");
			Sym* main_sym = nullptr;
			for (size_t i = 0; i < self.global_scope->syms.count; ++i)
			{
				if (self.global_scope->syms[i]->name == main)
				{
					main_sym = self.global_scope->syms[i];
					break;
				}
			}

			if(main_sym == nullptr)
			{
				src_err(self.src, err_str(mn::strf("program doesn't have a main function")));
				return;
			}

			typer_sym_resolve(self, main_sym);
		}
		else
		{
			for (size_t i = 0; i < self.global_scope->syms.count; ++i)
				typer_sym_resolve(self, self.global_scope->syms[i]);
		}

		if(src_has_err(self.src) == false)
		{
			// provide package name for all the reachable symbols
			for(auto sym: self.src->reachable_syms)
				if(self.src->ast.package)
					sym->package_name = mn::strf("{}_{}", self.src->ast.package.str, sym->name);
		}
	}
}