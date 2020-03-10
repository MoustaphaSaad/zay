#pragma once

#include "zay/scan/Pos.h"
#include "zay/scan/Rng.h"
#include "zay/scan/Tkn.h"
#include "zay/parse/AST.h"

#include <mn/Str.h>

namespace zay
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

	inline static Err
	err_expr(Expr *e, const mn::Str& m)
	{
		Err self{};
		self.pos = e->pos;
		self.rng = e->rng;
		self.msg = m;
		return self;
	}

	inline static Err
	err_stmt(Stmt* s, const mn::Str& m)
	{
		Err self{};
		self.pos = s->pos;
		self.rng = s->rng;
		self.msg = m;
		return self;
	}

	inline static Err
	err_decl(Decl* d, const mn::Str& m)
	{
		Err self{};
		self.pos = d->pos;
		self.rng = d->rng;
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