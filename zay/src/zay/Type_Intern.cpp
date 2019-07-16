#include "zay/Type_Intern.h"

#include <mn/Memory.h>

#include <assert.h>

namespace zay
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

	static IType _type_lit_int = builtin(IType::KIND_INT);
	Type type_lit_int = &_type_lit_int;

	static IType _type_lit_float64 = builtin(IType::KIND_FLOAT64);
	Type type_lit_float64 = &_type_lit_float64;

	//API
	Type
	type_func(const Func_Sign& sign)
	{
		Type self = alloc<IType>();
		self->kind = IType::KIND_FUNC;
		self->sym = nullptr;
		self->func = sign;
		return self;
	}

	Type
	type_ptr(Type base)
	{
		Type self = alloc<IType>();
		self->kind = IType::KIND_PTR;
		self->sym = nullptr;
		self->ptr.base = base;
		return self;
	}

	Type
	type_array(const Array_Sign& sign)
	{
		Type self = alloc<IType>();
		self->kind = IType::KIND_ARRAY;
		self->sym = nullptr;
		self->array = sign;
		return self;
	}

	Type
	type_incomplete(Sym sym)
	{
		Type self = alloc<IType>();
		self->kind = IType::KIND_INCOMPLETE;
		self->sym = sym;
		return self;
	}

	void
	type_alias_complete(Type self, Type alias)
	{
		assert(self->kind == IType::KIND_COMPLETING);
		self->kind = IType::KIND_ALIAS;
		self->alias = alias;
	}

	void
	type_struct_complete(Type self, const mn::Buf<Field_Sign>& fields)
	{
		assert(self->kind == IType::KIND_COMPLETING);
		self->kind = IType::KIND_STRUCT;
		self->fields = fields;
	}

	void
	type_union_complete(Type self, const mn::Buf<Field_Sign>& fields)
	{
		assert(self->kind == IType::KIND_COMPLETING);
		self->kind = IType::KIND_UNION;
		self->fields = fields;
	}

	void
	type_enum_complete(Type self, const mn::Buf<Enum_Value>& values)
	{
		assert(self->kind == IType::KIND_COMPLETING);
		self->kind = IType::KIND_ENUM;
		self->enum_values = values;
	}

	void
	type_free(Type self)
	{
		switch(self->kind)
		{
		case IType::KIND_INCOMPLETE:
		case IType::KIND_COMPLETING:
			break;
			//we don't free the builtin types
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
		case IType::KIND_ARRAY:
		case IType::KIND_ALIAS:
			break;
		case IType::KIND_FUNC:
			func_sign_free(self->func);
			break;
		case IType::KIND_STRUCT:
		case IType::KIND_UNION:
			buf_free(self->fields);
			break;
		case IType::KIND_ENUM:
			buf_free(self->enum_values);
			break;
		default: assert(false && "unreachable"); break;
		}
		free(self);
	}


	Type_Intern
	type_intern_new()
	{
		Type_Intern self = alloc<IType_Intern>();
		self->types = buf_new<Type>();
		self->ptr_table = map_new<Type, Type>();
		self->array_table = map_new<Array_Sign, Type, Array_Sign_Hasher>();
		self->func_table = map_new<Func_Sign, Type, Func_Sign_Hasher>();
		return self;
	}

	void
	type_intern_free(Type_Intern self)
	{
		destruct(self->types);
		map_free(self->ptr_table);
		map_free(self->array_table);
		map_free(self->func_table);
		free(self);
	}

	Type
	type_intern_ptr(Type_Intern self, Type base)
	{
		if(auto it = map_lookup(self->ptr_table, base))
			return it->value;

		Type new_type = type_ptr(base);
		buf_push(self->types, new_type);
		map_insert(self->ptr_table, base, new_type);
		return new_type;
	}

	Type
	type_intern_array(Type_Intern self, const Array_Sign& sign)
	{
		if(auto it = map_lookup(self->array_table, sign))
			return it->value;

		Type new_type = type_array(sign);
		buf_push(self->types, new_type);
		map_insert(self->array_table, sign, new_type);
		return new_type;
	}

	Type
	type_intern_func(Type_Intern self, Func_Sign& func)
	{
		if(auto it = map_lookup(self->func_table, func))
		{
			func_sign_free(func);
			return it->value;
		}

		Type new_type = type_func(func);
		buf_push(self->types, new_type);
		map_insert(self->func_table, func, new_type);
		return new_type;
	}

	Type
	type_intern_incomplete(Type_Intern self, Type type)
	{
		return *buf_push(self->types, type);
	}
}