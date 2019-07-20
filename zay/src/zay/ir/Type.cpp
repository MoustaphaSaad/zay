#include "zay/ir/Type.h"

#include <mn/Memory.h>

#include <assert.h>

namespace zay::ir
{
	using namespace mn;

	inline static IType
	builtin(IType::KIND k)
	{
		IType self{};
		self.kind = k;
		return self;
	}

	static IType _type_void = builtin(IType::KIND_VOID);
	Type type_void = &_type_void;

	static IType _type_bool = builtin(IType::KIND_BOOL);
	Type type_bool = &_type_bool;

	static IType _type_int = builtin(IType::KIND_INT);
	Type type_int = &_type_int;

	static IType _type_uint = builtin(IType::KIND_UINT);
	Type type_uint = &_type_uint;

	static IType _type_int8 = builtin(IType::KIND_INT8);
	Type type_int8 = &_type_int8;

	static IType _type_uint8 = builtin(IType::KIND_UINT8);
	Type type_uint8 = &_type_uint8;

	static IType _type_int16 = builtin(IType::KIND_INT16);
	Type type_int16 = &_type_int16;

	static IType _type_uint16 = builtin(IType::KIND_UINT16);
	Type type_uint16 = &_type_uint16;

	static IType _type_int32 = builtin(IType::KIND_INT32);
	Type type_int32 = &_type_int32;

	static IType _type_uint32 = builtin(IType::KIND_UINT32);
	Type type_uint32 = &_type_uint32;

	static IType _type_int64 = builtin(IType::KIND_INT64);
	Type type_int64 = &_type_int64;

	static IType _type_uint64 = builtin(IType::KIND_UINT64);
	Type type_uint64 = &_type_uint64;

	static IType _type_float32 = builtin(IType::KIND_FLOAT32);
	Type type_float32 = &_type_float32;

	static IType _type_float64 = builtin(IType::KIND_FLOAT64);
	Type type_float64 = &_type_float64;

	static IType _type_string = builtin(IType::KIND_STRING);
	Type type_string = &_type_string;


	//API
	Type
	type_ptr(Type base)
	{
		Type self = alloc<IType>();
		self->kind = IType::KIND_PTR;
		self->ptr = base;
		return self;
	}

	Type
	type_array(Type base, size_t count)
	{
		Type self = alloc<IType>();
		self->kind = IType::KIND_ARRAY;
		self->array.base = base;
		self->array.count = count;
		return self;
	}

	Type
	type_func(Type ret, const Buf<Type>& args)
	{
		Type self = alloc<IType>();
		self->kind = IType::KIND_FUNC;
		self->func.ret = ret;
		self->func.args = args;
		return self;
	}

	void
	type_free(Type self)
	{
		switch(self->kind)
		{
		case IType::KIND_VOID:
		case IType::KIND_BOOL:
		case IType::KIND_INT:
		case IType::KIND_UINT:
		case IType::KIND_INT8:
		case IType::KIND_UINT8:
		case IType::KIND_INT16:
		case IType::KIND_UINT16:
		case IType::KIND_INT32:
		case IType::KIND_UINT32:
		case IType::KIND_INT64:
		case IType::KIND_UINT64:
		case IType::KIND_FLOAT32:
		case IType::KIND_FLOAT64:
		case IType::KIND_STRING:
			return;

		case IType::KIND_PTR:
			type_free(self->ptr);
			break;
		case IType::KIND_ARRAY:
			type_free(self->array.base);
			break;
		case IType::KIND_FUNC:
			destruct(self->func.args);
			type_free(self->func.ret);
			break;

		case IType::KIND_NONE:
		default:
			assert(false && "unreachable");
			break;
		}
		free(self);
	}
}
