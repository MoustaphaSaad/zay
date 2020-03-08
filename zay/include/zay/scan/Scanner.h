#pragma once

#include "zay/Exports.h"
#include "zay/scan/Pos.h"
#include "zay/Src.h"

#include <mn/Rune.h>

namespace zay
{
	struct Scanner
	{
		Src *src;
		// iterator on the source code
		const char* it;
		// Rune is a utf-8 codepoint (A character)
		mn::Rune c;
		// position of the current character
		Pos pos;
	};

	inline static Scanner
	scanner_new(Src *src)
	{
		Scanner self{};
		self.src = src;
		self.it = begin(self.src->content);
		self.c = mn::rune_read(self.it);
		self.pos = Pos{1, 0};

		src_line_begin(self.src, self.it);
		return self;
	}

	inline static bool
	scanner_eof(Scanner *self)
	{
		return self->it >= end(self->src->content);
	}

	ZAY_EXPORT Tkn
	scanner_tkn(Scanner *self);

	inline static bool
	src_scan(Src *src)
	{
		auto self = scanner_new(src);
		while(true)
		{
			if(Tkn tkn = scanner_tkn(&self))
				src_tkn(src, tkn);
			else
				break;
		}
		return src_has_err(src) == false;
	}
}