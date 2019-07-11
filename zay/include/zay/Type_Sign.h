#pragma once

#include "zay/Exports.h"
#include "zay/Tkn.h"

#include <mn/Buf.h>

namespace zay
{
	struct Field;
	struct Enum_Field;

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
			KIND_ENUM
		};

		KIND kind;
		union
		{
			Tkn named;
			Tkn count;
			mn::Buf<Field> struct_fields;
			mn::Buf<Field> union_fields;
			mn::Buf<Enum_Field> enum_fields;
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

	ZAY_EXPORT void
	type_atom_free(Type_Atom& self);

	inline static void
	destruct(Type_Atom& self)
	{
		type_atom_free(self);
	}

	ZAY_EXPORT Type_Atom
	clone(const Type_Atom& self);

	typedef mn::Buf<Type_Atom> Type_Sign;

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
}
