#pragma once

#include "alp/Exports.h"
#include "alp/Pos.h"
#include "alp/Src.h"

#include <mn/Rune.h>

namespace alp
{
	typedef struct IScanner* Scanner;
	struct IScanner
	{
		Src src;
		// iterator on the source code
		const char* it;
		// Rune is a utf-8 codepoint (A character)
		mn::Rune c;
		// position of the current character
		Pos pos;
	};

	ALP_EXPORT Scanner
	scanner_new(Src src);

	ALP_EXPORT void
	scanner_free(Scanner self);

	inline static void
	destruct(Scanner self)
	{
		scanner_free(self);
	}

	inline static bool
	scanner_eof(Scanner self)
	{
		return self->it >= end(self->src->content);
	}

	ALP_EXPORT Tkn
	scanner_tkn(Scanner self);

	inline static bool
	src_scan(Src src)
	{
		Scanner self = scanner_new(src);
		while(true)
		{
			if(Tkn tkn = scanner_tkn(self))
				src_tkn(src, tkn);
			else
				break;
		}
		scanner_free(self);
		return src_has_err(src) == false;
	}
}