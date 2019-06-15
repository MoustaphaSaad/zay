#pragma once 

#include "zay/Exports.h"

#include <mn/Buf.h>
#include <mn/Map.h>

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
			KIND_FUNC
		};

		KIND kind;
		union
		{
			struct { Type base; } ptr;
			Array_Sign array;
			Func_Sign func;
		};
		
	};

	ZAY_EXPORT Type
	type_ptr(Type base);

	ZAY_EXPORT Type
	type_array(const Array_Sign& sign);

	ZAY_EXPORT Type
	type_func(const Func_Sign& sign);

	ZAY_EXPORT void
	type_free(Type self);

	inline static void
	destruct(Type self)
	{
		type_free(self);
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
}