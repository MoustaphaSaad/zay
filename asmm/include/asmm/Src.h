#pragma once

#include "asmm/Exports.h"
#include "asmm/Err.h"
#include "asmm/scan/Rng.h"
#include "asmm/scan/Tkn.h"
#include "asmm/parse/Func.h"

#include <vm/Program.h>

#include <mn/Str.h>
#include <mn/Str_Intern.h>
#include <mn/Buf.h>
#include <mn/Map.h>

namespace asmm
{
	// Line is a range of source code
	typedef Rng Line;

	// Src is our compilation unit
	struct ISrc
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
		// the functions in the compilation unit
		mn::Buf<Func> funcs;
		// vm assembled output
		vm::Program program;
	};
	typedef ISrc* Src;

	ASMM_EXPORT Src
	src_from_file(const char* path);

	ASMM_EXPORT Src
	src_from_str(const char* code);

	ASMM_EXPORT void
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

	ASMM_EXPORT mn::Str
	src_errs_dump(Src self, mn::Allocator allocator = mn::allocator_top());

	ASMM_EXPORT mn::Str
	src_tkns_dump(Src self, mn::Allocator allocator = mn::allocator_top());
}