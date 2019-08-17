#pragma once

#include "alp/Pos.h"
#include "alp/Rng.h"
#include "alp/Tkn.h"

#include <mn/Str.h>

namespace alp
{
	// An Error is just position in the source code and a string error msg
	struct Err
	{
		Pos pos;
		Rng rng;
		mn::Str msg;
	};

	inline static Err
	err_str(const mn::Str& msg)
	{
		Err self{};
		self.msg = msg;
		return self;
	}

	inline static Err
	err_tkn(const Tkn& t, const mn::Str& m)
	{
		Err self{};
		self.pos = t.pos;
		self.rng = t.rng;
		self.msg = m;
		return self;
	}

	inline static void
	err_free(Err& self)
	{
		mn::str_free(self.msg);
	}

	inline static void
	destruct(Err& self)
	{
		err_free(self);
	}
}