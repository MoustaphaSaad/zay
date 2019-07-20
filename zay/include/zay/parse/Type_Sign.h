#pragma once

#include "zay/Exports.h"
#include "zay/scan/Tkn.h"

#include <mn/Buf.h>

namespace zay
{
	struct Field;
	struct Enum_Field;

	struct Type_Atom;
	typedef mn::Buf<Type_Atom> Type_Sign;

	//Types
	struct Type_Atom
	{
		enum KIND
		{
			KIND_NONE,
			KIND_NAMED,
			KIND_PTR,
			KIND_ARRAY,
			KIND_STRUCT,
			KIND_UNION,
			KIND_ENUM,
			KIND_FUNC
		};

		KIND kind;
		union
		{
			Tkn named;
			Tkn count;
			mn::Buf<Field> struct_fields;
			mn::Buf<Field> union_fields;
			mn::Buf<Enum_Field> enum_fields;
			struct {
				mn::Buf<Type_Sign> args;
				Type_Sign ret;
			} func;
		};
	};

	ZAY_EXPORT Type_Atom
	type_atom_named(const Tkn& t);

	ZAY_EXPORT Type_Atom
	type_atom_ptr();

	ZAY_EXPORT Type_Atom
	type_atom_array(const Tkn& t);

	ZAY_EXPORT Type_Atom
	type_atom_struct(const mn::Buf<Field>& fields);

	ZAY_EXPORT Type_Atom
	type_atom_union(const mn::Buf<Field>& fields);

	ZAY_EXPORT Type_Atom
	type_atom_enum(const mn::Buf<Enum_Field>& fields);

	ZAY_EXPORT Type_Atom
	type_atom_func(const mn::Buf<Type_Sign>& args, const Type_Sign& ret);

	ZAY_EXPORT void
	type_atom_free(Type_Atom& self);

	inline static void
	destruct(Type_Atom& self)
	{
		type_atom_free(self);
	}

	ZAY_EXPORT Type_Atom
	clone(const Type_Atom& self);



	inline static Type_Sign
	type_sign_new()
	{
		return mn::buf_new<Type_Atom>();
	}

	inline static void
	type_sign_free(Type_Sign& self)
	{
		mn::destruct(self);
	}


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
	typedef struct IExpr* Expr;
	struct Enum_Field
	{
		Tkn id;
		Expr expr;
	};
}
