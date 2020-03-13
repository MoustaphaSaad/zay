#pragma once

#include "zay/Exports.h"
#include "zay/scan/Tkn.h"
#include "zay/parse/AST.h"

#include <assert.h>

namespace zay
{
	struct Type;
	struct Sym
	{
		enum KIND
		{
			KIND_NONE,
			KIND_STRUCT,
			KIND_UNION,
			KIND_ENUM,
			KIND_VAR,
			KIND_FUNC,
			KIND_TYPE
		};

		enum STATE
		{
			STATE_UNRESOLVED,
			STATE_RESOLVING,
			STATE_RESOLVED
		};

		KIND kind;
		STATE state;
		const char* name;
		Type* type;
		union
		{
			Decl* struct_sym;
			Decl* union_sym;
			Decl* enum_sym;
			Decl* func_sym;

			struct
			{
				Tkn id;
				Decl* decl;
				Type_Sign type;
				Expr* expr;
			} var_sym;
			Decl* type_sym;
		};
	};


	ZAY_EXPORT Sym*
	sym_struct(Decl* d);

	ZAY_EXPORT Sym*
	sym_union(Decl* d);

	ZAY_EXPORT Sym*
	sym_enum(Decl* d);

	ZAY_EXPORT Sym*
	sym_var(const Tkn& id, Decl* decl, const Type_Sign& type, Expr* expr);

	ZAY_EXPORT Sym*
	sym_func(Decl* d);

	ZAY_EXPORT Sym*
	sym_type(Decl* d);

	ZAY_EXPORT void
	sym_free(Sym* self);

	inline static void
	destruct(Sym* self)
	{
		sym_free(self);
	}

	inline static Tkn
	sym_tkn(Sym* self)
	{
		switch(self->kind)
		{
		case Sym::KIND_STRUCT: return self->struct_sym->name;
		case Sym::KIND_UNION: return self->union_sym->name;
		case Sym::KIND_ENUM: return self->enum_sym->name;
		case Sym::KIND_VAR: return self->var_sym.id;
		case Sym::KIND_FUNC: return self->func_sym->name;
		case Sym::KIND_TYPE: return self->type_sym->name;
		default: assert(false && "unreachable"); return Tkn{};
		}
	}

	inline static Decl*
	sym_decl(Sym* self)
	{
		switch(self->kind)
		{
		case Sym::KIND_STRUCT: return self->struct_sym;
		case Sym::KIND_UNION: return self->union_sym;
		case Sym::KIND_ENUM: return self->enum_sym;
		case Sym::KIND_VAR: return self->var_sym.decl;
		case Sym::KIND_FUNC: return self->func_sym;
		case Sym::KIND_TYPE: return self->type_sym;
		default: assert(false && "unreachable"); return nullptr;
		}
	}
}