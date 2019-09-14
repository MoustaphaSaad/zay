#pragma once

#include "cypher/Exports.h"
#include "cypher/Func.h"

#include <mn/Str.h>
#include <mn/Buf.h>

namespace cypher
{
	typedef struct IPkg* Pkg;
	struct IPkg
	{
		mn::Str name;
		mn::Buf<Func> funcs;
	};

	CYPHER_EXPORT Pkg
	pkg_new(const mn::Str& name);

	inline static Pkg
	pkg_new(const char* name)
	{
		return pkg_new(mn::str_from_c(name));
	}

	CYPHER_EXPORT void
	pkg_free(Pkg self);

	inline static void
	destruct(Pkg self)
	{
		pkg_free(self);
	}

	inline static Func
	pkg_func_new(Pkg self, const mn::Str& name, Type type)
	{
		Func f = func_new(name, type);
		buf_push(self->funcs, f);
		return f;
	}

	inline static Func
	pkg_func_new(Pkg self, const char* name, Type type)
	{
		return pkg_func_new(self, mn::str_from_c(name), type);
	}

	inline static Func
	pkg_func_get(Pkg self, const mn::Str& name)
	{
		for(auto f: self->funcs)
			if(f->name == name)
				return f;
		return nullptr;
	}
}
