#pragma once

#include "cypher/Exports.h"

#include <mn/Buf.h>

#include <stddef.h>

namespace cypher
{
	typedef struct IType* Type;

	struct IType
	{
		enum KIND
		{
			KIND_NONE,
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
			Type ptr;
			struct
			{
				Type base;
				size_t count;
			} array;
			struct
			{
				mn::Buf<Type> args;
				Type ret;
			} func;
		};
	};

	CYPHER_EXPORT extern Type type_void;
	CYPHER_EXPORT extern Type type_bool;
	CYPHER_EXPORT extern Type type_int;
	CYPHER_EXPORT extern Type type_uint;
	CYPHER_EXPORT extern Type type_int8;
	CYPHER_EXPORT extern Type type_uint8;
	CYPHER_EXPORT extern Type type_int16;
	CYPHER_EXPORT extern Type type_uint16;
	CYPHER_EXPORT extern Type type_int32;
	CYPHER_EXPORT extern Type type_uint32;
	CYPHER_EXPORT extern Type type_int64;
	CYPHER_EXPORT extern Type type_uint64;
	CYPHER_EXPORT extern Type type_float32;
	CYPHER_EXPORT extern Type type_float64;
	CYPHER_EXPORT extern Type type_string;

	CYPHER_EXPORT Type
	type_ptr(Type base);

	CYPHER_EXPORT Type
	type_array(Type base, size_t count);

	CYPHER_EXPORT Type
	type_func(Type ret, const mn::Buf<Type>& args);

	inline static Type
	type_func(Type ret, const std::initializer_list<Type>& args)
	{
		return type_func(ret, mn::buf_lit(args));
	}

	CYPHER_EXPORT void
	type_free(Type self);

	inline static void
	destruct(Type self)
	{
		type_free(self);
	}
}