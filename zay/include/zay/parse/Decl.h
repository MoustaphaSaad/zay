#pragma once

#include "zay/Exports.h"
#include "zay/scan/Tkn.h"
#include "zay/parse/Type_Sign.h"
#include "zay/parse/Stmt.h"
#include "zay/parse/Expr.h"

#include <mn/Buf.h>

namespace zay
{
	//Function Arguments
	struct Arg
	{
		mn::Buf<Tkn> ids;
		Type_Sign type;
	};

	inline static Arg
	arg_new()
	{
		return Arg{
			mn::buf_new<Tkn>(),
			type_sign_new()
		};
	}

	inline static void
	arg_free(Arg& self)
	{
		mn::buf_free(self.ids);
		type_sign_free(self.type);
	}

	inline static void
	destruct(Arg& self)
	{
		arg_free(self);
	}

	struct Decl
	{
		enum KIND
		{
			KIND_NONE,
			KIND_VAR,
			KIND_FUNC,
			KIND_TYPE
		};

		KIND kind;
		//all declarations have names
		Tkn name;
		Rng rng;
		Pos pos;
		union
		{
			Var var_decl;

			struct
			{
				mn::Buf<Arg> args;
				Type_Sign ret_type;
				Stmt* body;
			} func_decl;

			Type_Sign type_decl;
		};
	};

	ZAY_EXPORT Decl*
	decl_var(const Var& v);

	ZAY_EXPORT Decl*
	decl_func(const Tkn& name, const mn::Buf<Arg>& args, const Type_Sign& ret_type, Stmt* body);

	ZAY_EXPORT Decl*
	decl_type(const Tkn& name, const Type_Sign& type);

	ZAY_EXPORT void
	decl_free(Decl* self);

	inline static void
	destruct(Decl* self)
	{
		decl_free(self);
	}
}
