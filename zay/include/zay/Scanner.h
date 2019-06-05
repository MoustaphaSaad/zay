#pragma once

#include "zay/Exports.h"
#include "zay/Pos.h"
#include "zay/Src.h"

#include <mn/Rune.h>

namespace zay
{
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
	typedef IScanner* Scanner;

	ZAY_EXPORT Scanner
	scanner_new(Src src);

	ZAY_EXPORT void
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

	ZAY_EXPORT Tkn
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