#pragma once

#include "zay/Exports.h"

#include <mn/Buf.h>

#include <stddef.h>

namespace zay::ir
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

	ZAY_EXPORT Type
	type_ptr(Type base);

	ZAY_EXPORT Type
	type_array(Type base, size_t count);

	ZAY_EXPORT Type
	type_func(Type ret, const mn::Buf<Type>& args);

	ZAY_EXPORT void
	type_free(Type self);

	inline static void
	destruct(Type self)
	{
		ir::type_free(self);
	}
}