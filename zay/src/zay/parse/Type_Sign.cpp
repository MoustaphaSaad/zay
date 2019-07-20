#include "zay/parse/Type_Sign.h"
#include "zay/parse/Decl.h"

namespace zay
{
	using namespace mn;

	Type_Atom
	type_atom_named(const Tkn& t)
	{
		Type_Atom self{};
		self.kind = Type_Atom::KIND_NAMED;
		self.named = t;
		return self;
	}

	Type_Atom
	type_atom_ptr()
	{
		Type_Atom self{};
		self.kind = Type_Atom::KIND_PTR;
		return self;
	}

	Type_Atom
	type_atom_array(const Tkn& t)
	{
		Type_Atom self{};
		self.kind = Type_Atom::KIND_ARRAY;
		self.count = t;
		return self;
	}

	Type_Atom
	type_atom_struct(const Buf<Field>& fields)
	{
		Type_Atom self{};
		self.kind = Type_Atom::KIND_STRUCT;
		self.struct_fields = fields;
		return self;
	}

	Type_Atom
	type_atom_union(const Buf<Field>& fields)
	{
		Type_Atom self{};
		self.kind = Type_Atom::KIND_UNION;
		self.union_fields = fields;
		return self;
	}

	Type_Atom
	type_atom_enum(const Buf<Enum_Field>& fields)
	{
		Type_Atom self{};
		self.kind = Type_Atom::KIND_ENUM;
		self.enum_fields = fields;
		return self;
	}

	Type_Atom
	type_atom_func(const Buf<Type_Sign>& args, const Type_Sign& ret)
	{
		Type_Atom self{};
		self.kind = Type_Atom::KIND_FUNC;
		self.func.args = args;
		self.func.ret = ret;
		return self;
	}

	void
	type_atom_free(Type_Atom& self)
	{
		if (self.kind == Type_Atom::KIND_STRUCT)
		{
			destruct(self.struct_fields);
		}
		else if (self.kind == Type_Atom::KIND_UNION)
		{
			destruct(self.union_fields);
		}
		else if (self.kind == Type_Atom::KIND_ENUM)
		{
			for (const Enum_Field& f : self.enum_fields)
				if(f.expr)
					expr_free(f.expr);
			buf_free(self.enum_fields);
		}
		else if(self.kind == Type_Atom::KIND_FUNC)
		{
			destruct(self.func.args);
			destruct(self.func.ret);
		}
	}

	Type_Atom
	clone(const Type_Atom& self)
	{
		switch(self.kind)
		{
		case Type_Atom::KIND_NAMED:
			return type_atom_named(self.named);
		case Type_Atom::KIND_PTR:
			return type_atom_ptr();
		case Type_Atom::KIND_ARRAY:
			return type_atom_array(self.count);
		case Type_Atom::KIND_STRUCT:
			return type_atom_struct(clone(self.struct_fields));
		case Type_Atom::KIND_UNION:
			return type_atom_union(clone(self.union_fields));
		case Type_Atom::KIND_ENUM:
			return type_atom_enum(clone(self.enum_fields));
		default:
			assert(false && "unreachable");
			return Type_Atom{};
		}
	}
}
