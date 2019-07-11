#pragma once

#include "zay/Exports.h"
#include "zay/Tkn.h"
#include "zay/Type_Sign.h"
#include "zay/Stmt.h"
#include "zay/Expr.h"

#include <mn/Buf.h>

namespace zay
{
	//Declarations
	typedef struct IDecl* Decl;

	//Struct, Union Fields
	struct Field
	{
		mn::Buf<Tkn> ids;
		Type_Sign type;
	};

	inline static Field
	field_new()
	{
		Field self{};
		self.ids = mn::buf_new<Tkn>();
		self.type = type_sign_new();
		return self;
	}

	inline static void
	field_free(Field& self)
	{
		mn::buf_free(self.ids);
		type_sign_free(self.type);
	}

	inline static void
	destruct(Field& self)
	{
		field_free(self);
	}

	inline static Field
	clone(const Field& self)
	{
		return Field{ clone(self.ids), clone(self.type) };
	}

	//Enum Field
	struct Enum_Field
	{
		Tkn id;
		Expr expr;
	};

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

	struct IDecl
	{
		enum KIND
		{
			KIND_NONE,
			KIND_STRUCT,
			KIND_UNION,
			KIND_ENUM,
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
			mn::Buf<Field> struct_decl;

			//let's not care about unions right now
			mn::Buf<Field> union_decl;

			mn::Buf<Enum_Field> enum_decl;

			Var var_decl;

			struct
			{
				mn::Buf<Arg> args;
				Type_Sign ret_type;
				Stmt body;
			} func_decl;

			Type_Sign type_decl;
		};
	};

	ZAY_EXPORT Decl
	decl_struct(const Tkn& name, const mn::Buf<Field>& fields);

	ZAY_EXPORT Decl
	decl_union(const Tkn& name, const mn::Buf<Field>& fields);

	ZAY_EXPORT Decl
	decl_enum(const Tkn& name, const mn::Buf<Enum_Field>& fields);

	ZAY_EXPORT Decl
	decl_var(const Var& v);

	ZAY_EXPORT Decl
	decl_func(const Tkn& name, const mn::Buf<Arg>& args, const Type_Sign& ret_type, Stmt body);

	ZAY_EXPORT Decl
	decl_type(const Tkn& name, const Type_Sign& type);

	ZAY_EXPORT void
	decl_free(Decl self);

	inline static void
	destruct(Decl self)
	{
		decl_free(self);
	}
}
