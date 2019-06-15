#pragma once

#include "zay/Exports.h"
#include "zay/Tkn.h"
#include "zay/AST.h"

#include <assert.h>

namespace zay
{
	typedef struct IType* Type;
	typedef struct ISym* Sym;
	struct ISym
	{
		enum KIND
		{
			KIND_NONE,
			KIND_STRUCT,
			KIND_UNION,
			KIND_ENUM,
			KIND_VAR,
			KIND_FUNC
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
		Type type;
		union
		{
			Decl struct_sym;
			Decl union_sym;
			Decl enum_sym;
			Decl func_sym;
			Tkn var_sym;
		};
	};


	ZAY_EXPORT Sym
	sym_struct(Decl d);

	ZAY_EXPORT Sym
	sym_union(Decl d);

	ZAY_EXPORT Sym
	sym_enum(Decl d);

	ZAY_EXPORT Sym
	sym_var(const Tkn& id);

	ZAY_EXPORT Sym
	sym_func(Decl d);

	ZAY_EXPORT void
	sym_free(Sym self);

	inline static void
	destruct(Sym self)
	{
		sym_free(self);
	}

	inline static Tkn
	sym_tkn(Sym self)
	{
		switch(self->kind)
		{
		case ISym::KIND_STRUCT: return self->struct_sym->name;
		case ISym::KIND_UNION: return self->union_sym->name;
		case ISym::KIND_ENUM: return self->enum_sym->name;
		case ISym::KIND_VAR: return self->var_sym;
		case ISym::KIND_FUNC: return self->func_sym->name;
		default: assert(false && "unreachable"); return Tkn{};
		}
	}
}