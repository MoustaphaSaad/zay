#include "zay/Typer.h"

#include <mn/Memory.h>
#include <mn/IO.h>

#include <assert.h>

namespace zay
{
	using namespace mn;

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
	}
}