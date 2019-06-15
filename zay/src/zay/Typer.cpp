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

	inline static void
	typer_stmt_resolve(Typer self, Stmt stmt);


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
				for(const Tkn& id: d->var_decl.ids)
					typer_sym(self, sym_var(id));
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

		return type_void;
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
		return type_void;
	}

	inline static Type
	typer_expr_dot_resolve(Typer , Expr )
	{
		//let's leave this for now since we didn't add structs
		//let's compile because i'm bored
		return type_void;
	}

	inline static Type
	typer_expr_indexed_resolve(Typer , Expr )
	{
		return type_void;
	}

	inline static Type
	typer_expr_call_resolve(Typer , Expr )
	{
		return type_void;
	}

	inline static Type
	typer_expr_cast_resolve(Typer , Expr )
	{
		return type_void;
	}

	inline static Type
	typer_expr_paren_resolve(Typer , Expr )
	{
		return type_void;
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
		case IExpr::KIND_CAST: return typer_expr_cast_resolve(self, expr);
		case IExpr::KIND_PAREN: return typer_expr_paren_resolve(self, expr);
		default: assert(false && "unreachable"); return type_void;
		}
	}


	inline static void
	typer_stmt_break_resolve(Typer , Stmt )
	{

	}

	inline static void
	typer_stmt_continue_resolve(Typer , Stmt )
	{

	}

	inline static void
	typer_stmt_return_resolve(Typer , Stmt )
	{

	}

	inline static void
	typer_stmt_if_resolve(Typer , Stmt )
	{

	}

	inline static void
	typer_stmt_for_resolve(Typer , Stmt )
	{

	}

	inline static void
	typer_stmt_var_resolve(Typer , Stmt )
	{

	}

	inline static void
	typer_stmt_assign_resolve(Typer , Stmt )
	{

	}

	inline static void
	typer_stmt_expr_resolve(Typer , Stmt )
	{

	}

	inline static void
	typer_stmt_block_resolve(Typer , Stmt )
	{

	}

	inline static void
	typer_stmt_resolve(Typer , Stmt )
	{
		//type check here
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
		case ISym::KIND_FUNC:
			sym->type = typer_decl_func_resolve(self, sym);
			break;

		default:
			break;
		}
		sym->state = ISym::STATE_RESOLVED;
	}


	//API
	Typer
	typer_new(Src src)
	{
		Typer self = alloc<ITyper>();
		self->src = src;
		self->scope_stack = buf_new<Scope>();
		self->global_scope = src_scope_new(self->src, nullptr, nullptr);

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

		for(Sym sym: self->global_scope->syms)
			typer_sym_resolve(self, sym);
	}
}