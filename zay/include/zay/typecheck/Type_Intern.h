#pragma once 

#include "zay/Exports.h"
#include "zay/typecheck/Sym.h"

#include <mn/Buf.h>
#include <mn/Map.h>
#include <mn/IO.h>

namespace zay
{
	typedef struct IType* Type;

	//now we'll need to represent function types since this is what we're currently doing
	//function types is the aggregate of thier argument types and return type
	struct Func_Sign
	{
		mn::Buf<Type> args;
		Type ret;

		inline bool
		operator==(const Func_Sign& other) const
		{
			if(args.count != other.args.count)
				return false;

			if(ret != other.ret)
				return false;

			for(size_t i = 0; i < args.count; ++i)
				if(args[i] != other.args[i])
					return false;

			return true;
		}

		inline bool
		operator!=(const Func_Sign& other) const
		{
			return !operator==(other);
		}
	};

	inline static Func_Sign
	func_sign_new()
	{
		return Func_Sign { mn::buf_new<Type>(), nullptr };
	}

	inline static void
	func_sign_free(Func_Sign& self)
	{
		mn::buf_free(self.args);
	}

	inline static void
	destruct(Func_Sign& self)
	{
		func_sign_free(self);
	}

	struct Func_Sign_Hasher
	{
		inline size_t
		operator()(const Func_Sign& a) const
		{
			return mn::hash_mix(mn::murmur_hash(block_from(a.args)), mn::Hash<Type>()(a.ret));
		}
	};

	struct Array_Sign
	{
		Type base;
		size_t count;

		inline bool
		operator==(const Array_Sign& other) const
		{
			return base == other.base && count == other.count;
		}

		inline bool
		operator!=(const Array_Sign& other) const
		{
			return !operator==(other);
		}
	};

	struct Array_Sign_Hasher
	{
		inline size_t
		operator()(const Array_Sign& a) const
		{
			return mn::hash_mix(mn::Hash<Type>()(a.base), mn::Hash<size_t>()(a.count));
		}
	};


	struct Field_Sign
	{
		const char* name;
		Type type;
		size_t offset;
	};

	struct Enum_Value
	{
		Tkn id;
		Expr value;
	};


	struct IType
	{
		enum KIND
		{
			KIND_NONE,
			KIND_INCOMPLETE,
			KIND_COMPLETING,

			KIND_VOID,
			KIND_BOOL,
			KIND_INT,
			KIND_UINT,
			KIND_INT8,
			KIND_UINT8,
			KIND_INT16,
			KIND_UINT16,
			KIND_INT32,
			KIND_UINT32,
			KIND_INT64,
			KIND_UINT64,
			KIND_FLOAT32,
			KIND_FLOAT64,
			KIND_STRING,
			KIND_PTR,
			KIND_ARRAY,
			KIND_FUNC,
			KIND_STRUCT,
			KIND_UNION,
			KIND_ENUM,
			KIND_ALIAS
		};

		KIND kind;
		Sym sym;
		union
		{
			struct { Type base; } ptr;
			Array_Sign array;
			Func_Sign func;
			mn::Buf<Field_Sign> fields;
			mn::Buf<Enum_Value> enum_values;
			Type alias;
		};
		
	};

	ZAY_EXPORT Type
	type_ptr(Type base);

	ZAY_EXPORT Type
	type_array(const Array_Sign& sign);

	ZAY_EXPORT Type
	type_func(const Func_Sign& sign);

	ZAY_EXPORT Type
	type_incomplete(Sym sym);

	ZAY_EXPORT void
	type_alias_complete(Type self, Type alias);

	ZAY_EXPORT void
	type_struct_complete(Type self, const mn::Buf<Field_Sign>& fields);

	ZAY_EXPORT void
	type_union_complete(Type self, const mn::Buf<Field_Sign>& fields);

	ZAY_EXPORT void
	type_enum_complete(Type self, const mn::Buf<Enum_Value>& values);

	ZAY_EXPORT void
	type_free(Type self);

	inline static void
	destruct(Type self)
	{
		type_free(self);
	}

	inline static size_t
	print_str(mn::Stream stream, mn::Print_Format&, Type type)
	{
		switch(type->kind)
		{
		case IType::KIND_VOID: return mn::vprintf(stream, "void");
		case IType::KIND_BOOL: return mn::vprintf(stream, "bool");
		case IType::KIND_INT: return mn::vprintf(stream, "int");
		case IType::KIND_UINT: return mn::vprintf(stream, "uint");
		case IType::KIND_INT8: return mn::vprintf(stream, "int8");
		case IType::KIND_UINT8: return mn::vprintf(stream, "uint8");
		case IType::KIND_INT16: return mn::vprintf(stream, "int16");
		case IType::KIND_UINT16: return mn::vprintf(stream, "uint16");
		case IType::KIND_INT32: return mn::vprintf(stream, "int32");
		case IType::KIND_UINT32: return mn::vprintf(stream, "uint32");
		case IType::KIND_INT64: return mn::vprintf(stream, "int64");
		case IType::KIND_UINT64: return mn::vprintf(stream, "uint64");
		case IType::KIND_FLOAT32: return mn::vprintf(stream, "float32");
		case IType::KIND_FLOAT64: return mn::vprintf(stream, "float64");
		case IType::KIND_STRING: return mn::vprintf(stream, "string");
		case IType::KIND_PTR:
			return mn::vprintf(stream, "*{}", type->ptr.base);
		case IType::KIND_ARRAY:
			return mn::vprintf(stream, "[{}]{}", type->array.count, type->array.base);
		case IType::KIND_FUNC:
		{
			size_t res = mn::vprintf(stream, "func(");
			for(size_t i = 0; i < type->func.args.count; ++i)
			{
				if(i != 0)
					res += mn::vprintf(stream, ", ");
				res += mn::vprintf(stream, ":{}", type->func.args[i]);
			}
			res += mn::vprintf(stream, "): {}", type->func.ret);
			return res;
		}
		case IType::KIND_STRUCT:
		case IType::KIND_UNION:
		case IType::KIND_ENUM:
		case IType::KIND_ALIAS:
			return mn::vprintf(stream, "{}", type->sym->name);
		default:
			return mn::vprintf(stream, "<UNKNOWN_TYPE>");
		}
	}


	ZAY_EXPORT extern Type type_void;
	ZAY_EXPORT extern Type type_bool;
	ZAY_EXPORT extern Type type_int;
	ZAY_EXPORT extern Type type_uint;
	ZAY_EXPORT extern Type type_int8;
	ZAY_EXPORT extern Type type_uint8;
	ZAY_EXPORT extern Type type_int16;
	ZAY_EXPORT extern Type type_uint16;
	ZAY_EXPORT extern Type type_int32;
	ZAY_EXPORT extern Type type_uint32;
	ZAY_EXPORT extern Type type_int64;
	ZAY_EXPORT extern Type type_uint64;
	ZAY_EXPORT extern Type type_float32;
	ZAY_EXPORT extern Type type_float64;
	ZAY_EXPORT extern Type type_string;
	ZAY_EXPORT extern Type type_lit_int;
	ZAY_EXPORT extern Type type_lit_float64;

	inline static Type
	type_unwrap(Type type)
	{
		if (type->kind == IType::KIND_ALIAS)
			return type->alias;
		return type;
	}

	inline static bool
	type_is_numeric(Type t)
	{
		return (
			t == type_int ||
			t == type_uint ||
			t == type_int8 ||
			t == type_uint8 ||
			t == type_int16 ||
			t == type_uint16 ||
			t == type_int32 ||
			t == type_uint32 ||
			t == type_int64 ||
			t == type_uint64 ||
			t == type_float32 ||
			t == type_float64 ||
			t == type_lit_int ||
			t == type_lit_float64
		);
	}

	inline static bool
	type_is_integer(Type t)
	{
		return (
			t == type_int ||
			t == type_uint ||
			t == type_int8 ||
			t == type_uint8 ||
			t == type_int16 ||
			t == type_uint16 ||
			t == type_int32 ||
			t == type_uint32 ||
			t == type_int64 ||
			t == type_uint64 ||
			t == type_lit_int
		);
	}

	inline static bool
	type_is_float(Type t)
	{
		return t == type_float32 || t == type_float64 || t == type_lit_float64;
	}

	inline static bool
	type_is_lit(Type t)
	{
		return t == type_lit_float64 || t == type_lit_int;
	}

	inline static bool
	type_is_same(Type lhs, Type rhs)
	{
		if (lhs == rhs)
		{
			return true;
		}
		else if ((lhs == type_lit_float64 && type_is_float(rhs)) || (rhs == type_lit_float64 && type_is_float(lhs)))
		{
			return true;
		}
		else if((lhs == type_lit_int && type_is_integer(rhs)) || (rhs == type_lit_int && type_is_integer(lhs)))
		{
			return true;
		}
		return false;
	}

	typedef struct IType_Intern* Type_Intern;
	struct IType_Intern
	{
		mn::Buf<Type> types;
		mn::Map<Type, Type> ptr_table;
		mn::Map<Array_Sign, Type, Array_Sign_Hasher> array_table;
		mn::Map<Func_Sign, Type, Func_Sign_Hasher> func_table;
	};

	ZAY_EXPORT Type_Intern
	type_intern_new();

	ZAY_EXPORT void
	type_intern_free(Type_Intern self);

	inline static void
	destruct(Type_Intern self)
	{
		type_intern_free(self);
	}

	ZAY_EXPORT Type
	type_intern_ptr(Type_Intern self, Type base);

	ZAY_EXPORT Type
	type_intern_array(Type_Intern self, const Array_Sign& sign);

	ZAY_EXPORT Type
	type_intern_func(Type_Intern self, Func_Sign& func);

	ZAY_EXPORT Type
	type_intern_incomplete(Type_Intern self, Type type);
}