#pragma once

#include "zay/Tkn.h"
#include "zay/Type_Sign.h"

#include <mn/Buf.h>

namespace zay
{
	typedef struct IExpr* Expr;

	struct Var
	{
		mn::Buf<Tkn> ids;
		Type_Sign type;
		mn::Buf<Expr> exprs;
	};

	inline static Var
	var_new()
	{
		return Var{
			mn::buf_new<Tkn>(),
			type_sign_new(),
			mn::buf_new<Expr>()
		};
	}

	inline static void
	var_free(Var& self)
	{
		mn::buf_free(self.ids);
		type_sign_free(self.type);
		destruct(self.exprs);
	}

	inline static void
	destruct(Var& self)
	{
		var_free(self);
	}
}
