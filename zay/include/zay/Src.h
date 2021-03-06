#pragma once

#include "zay/Exports.h"
#include "zay/scan/Rng.h"
#include "zay/Err.h"
#include "zay/scan/Tkn.h"
#include "zay/parse/AST.h"
#include "zay/typecheck/Scope.h"
#include "zay/typecheck/Type_Intern.h"

#include <mn/Str.h>
#include <mn/Str_Intern.h>
#include <mn/Buf.h>
#include <mn/Map.h>

namespace zay
{
	enum class MODE
	{
		NONE,
		EXE,
		LIB
	};

	// Line is a range of source code
	typedef Rng Line;

	// Src is our compilation unit
	struct Src
	{
		// path of compilation unit on disk "<STRING>" if there's none
		mn::Str path;
		// content of the compilation unit (the code)
		mn::Str content;
		// source code lines
		mn::Buf<Line> lines;
		// string table for fast string compare
		mn::Str_Intern str_table;
		// list of errors in the compilation unit
		mn::Buf<Err> errs;
		// tokens of this compilation unit
		mn::Buf<Tkn> tkns;
		// AST of this compilation unit
		AST ast;
		// All the scopes created for this translation unit
		mn::Buf<Scope*> scopes;
		// Scopes can be attached to AST Entities so here's the attachment table
		mn::Map<void*, Scope*> scope_table;
		// Type Interning table to hold all the types of this complication unit
		Type_Intern type_table;
		// Reachable symbols
		mn::Buf<Sym*> reachable_syms;
	};

	ZAY_EXPORT Src*
	src_from_file(const char* path);

	ZAY_EXPORT Src*
	src_from_str(const char* code);

	ZAY_EXPORT void
	src_free(Src *self);

	inline static void
	destruct(Src *self)
	{
		src_free(self);
	}

	inline static void
	src_line_begin(Src *self, const char* begin)
	{
		mn::buf_push(self->lines, Line{begin, begin});
	}

	inline static void
	src_line_end(Src *self, const char* end)
	{
		mn::buf_top(self->lines).end = end;
	}

	inline static void
	src_err(Src *self, const Err& e)
	{
		mn::buf_push(self->errs, e);
	}

	inline static bool
	src_has_err(Src *self)
	{
		return self->errs.count != 0;
	}

	inline static void
	src_tkn(Src *self, const Tkn& t)
	{
		mn::buf_push(self->tkns, t);
	}

	inline static Scope*
	src_scope_new(Src *self, void* ast_node, Scope* parent, bool inside_loop, Type* ret)
	{
		auto scope = scope_new(parent, inside_loop, ret);
		mn::buf_push(self->scopes, scope);
		mn::map_insert(self->scope_table, ast_node, scope);
		return scope;
	}

	inline static Scope*
	src_scope_of(Src *self, void* ast_node)
	{
		if(auto it = mn::map_lookup(self->scope_table, ast_node))
			return it->value;
		return nullptr;
	}

	ZAY_EXPORT mn::Str
	src_errs_dump(Src *self, mn::Allocator allocator = mn::allocator_top());

	ZAY_EXPORT mn::Str
	src_tkns_dump(Src *self, mn::Allocator allocator = mn::allocator_top());

	ZAY_EXPORT mn::Str
	src_ast_dump(Src *self, mn::Allocator allocator = mn::allocator_top());
}