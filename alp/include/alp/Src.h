#pragma once

#include "alp/Exports.h"
#include "alp/Pos.h"
#include "alp/Rng.h"
#include "alp/Err.h"
#include "alp/Tkn.h"
#include "alp/AST.h"

#include <mn/Buf.h>
#include <mn/Str.h>
#include <mn/Str_Intern.h>

namespace alp
{
	typedef Rng Line;

	typedef struct ISrc* Src;
	struct ISrc
	{
		mn::Str path;
		mn::Str content;
		mn::Buf<Line> lines;
		mn::Str_Intern str_table;
		mn::Buf<Err> errs;
		mn::Buf<Tkn> tkns;
		AST ast;
	};

	ALP_EXPORT Src
	src_from_file(const char* path);

	ALP_EXPORT Src
	src_from_str(const char* code);

	ALP_EXPORT void
	src_free(Src self);

	inline static void
	destruct(Src self)
	{
		src_free(self);
	}

	inline static void
	src_line_begin(Src self, const char* begin)
	{
		mn::buf_push(self->lines, Line{begin, begin});
	}

	inline static void
	src_line_end(Src self, const char* end)
	{
		mn::buf_top(self->lines).end = end;
	}

	inline static void
	src_err(Src self, const Err& e)
	{
		mn::buf_push(self->errs, e);
	}

	inline static bool
	src_has_err(Src self)
	{
		return self->errs.count != 0;
	}

	inline static void
	src_tkn(Src self, const Tkn& t)
	{
		mn::buf_push(self->tkns, t);
	}

	ALP_EXPORT mn::Str
	src_errs_dump(Src self, mn::Allocator allocator = mn::allocator_top());

	ALP_EXPORT mn::Str
	src_tkns_dump(Src self, mn::Allocator allocator = mn::allocator_top());

	ALP_EXPORT mn::Str
	src_ast_dump(Src self, mn::Allocator allocator = mn::allocator_top());
}