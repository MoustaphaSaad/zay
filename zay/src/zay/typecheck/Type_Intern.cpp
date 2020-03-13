#include "zay/typecheck/Type_Intern.h"

#include <mn/Memory.h>

#include <assert.h>

namespace zay
{
	inline static Type
	builtin(Type::KIND k)
	{
		Type self{};
		self.kind = k;
		return self;
	}

	static Type _type_void = builtin(Type::KIND_VOID);
	Type* type_void = &_type_void;

	static Type _type_bool = builtin(Type::KIND_BOOL);
	Type* type_bool = &_type_bool;

	static Type _type_int = builtin(Type::KIND_INT);
	Type* type_int = &_type_int;

	static Type _type_uint = builtin(Type::KIND_UINT);
	Type* type_uint = &_type_uint;

	static Type _type_int8 = builtin(Type::KIND_INT8);
	Type* type_int8 = &_type_int8;

	static Type _type_uint8 = builtin(Type::KIND_UINT8);
	Type* type_uint8 = &_type_uint8;

	static Type _type_int16 = builtin(Type::KIND_INT16);
	Type* type_int16 = &_type_int16;

	static Type _type_uint16 = builtin(Type::KIND_UINT16);
	Type* type_uint16 = &_type_uint16;

	static Type _type_int32 = builtin(Type::KIND_INT32);
	Type* type_int32 = &_type_int32;

	static Type _type_uint32 = builtin(Type::KIND_UINT32);
	Type* type_uint32 = &_type_uint32;

	static Type _type_int64 = builtin(Type::KIND_INT64);
	Type* type_int64 = &_type_int64;

	static Type _type_uint64 = builtin(Type::KIND_UINT64);
	Type* type_uint64 = &_type_uint64;

	static Type _type_float32 = builtin(Type::KIND_FLOAT32);
	Type* type_float32 = &_type_float32;

	static Type _type_float64 = builtin(Type::KIND_FLOAT64);
	Type* type_float64 = &_type_float64;

	static Type _type_string = builtin(Type::KIND_STRING);
	Type* type_string = &_type_string;

	static Type _type_lit_int = builtin(Type::KIND_INT);
	Type* type_lit_int = &_type_lit_int;

	static Type _type_lit_float64 = builtin(Type::KIND_FLOAT64);
	Type* type_lit_float64 = &_type_lit_float64;

	//API
	Type*
	type_func(const Func_Sign& sign)
	{
		auto self = mn::alloc<Type>();
		self->kind = Type::KIND_FUNC;
		self->sym = nullptr;
		self->func = sign;
		return self;
	}

	Type*
	type_ptr(Type* base)
	{
		auto self = mn::alloc<Type>();
		self->kind = Type::KIND_PTR;
		self->sym = nullptr;
		self->ptr.base = base;
		return self;
	}

	Type*
	type_array(const Array_Sign& sign)
	{
		auto self = mn::alloc<Type>();
		self->kind = Type::KIND_ARRAY;
		self->sym = nullptr;
		self->array = sign;
		return self;
	}

	Type*
	type_incomplete(Sym sym)
	{
		auto self = mn::alloc<Type>();
		self->kind = Type::KIND_INCOMPLETE;
		self->sym = sym;
		return self;
	}

	void
	type_alias_complete(Type* self, Type* alias)
	{
		assert(self->kind == Type::KIND_COMPLETING);
		self->kind = Type::KIND_ALIAS;
		self->alias = alias;
	}

	void
	type_struct_complete(Type* self, const mn::Buf<Field_Sign>& fields)
	{
		assert(self->kind == Type::KIND_COMPLETING);
		self->kind = Type::KIND_STRUCT;
		self->fields = fields;
	}

	void
	type_union_complete(Type* self, const mn::Buf<Field_Sign>& fields)
	{
		assert(self->kind == Type::KIND_COMPLETING);
		self->kind = Type::KIND_UNION;
		self->fields = fields;
	}

	void
	type_enum_complete(Type* self, const mn::Buf<Enum_Value>& values)
	{
		assert(self->kind == Type::KIND_COMPLETING);
		self->kind = Type::KIND_ENUM;
		self->enum_values = values;
	}

	void
	type_free(Type* self)
	{
		switch(self->kind)
		{
		case Type::KIND_INCOMPLETE:
		case Type::KIND_COMPLETING:
			break;
			//we don't free the builtin types
		case Type::KIND_VOID:
		case Type::KIND_BOOL:
		case Type::KIND_INT:
		case Type::KIND_UINT:
		case Type::KIND_INT8:
		case Type::KIND_UINT8:
		case Type::KIND_INT16:
		case Type::KIND_UINT16:
		case Type::KIND_INT32:
		case Type::KIND_UINT32:
		case Type::KIND_INT64:
		case Type::KIND_UINT64:
		case Type::KIND_FLOAT32:
		case Type::KIND_FLOAT64:
		case Type::KIND_STRING:
			return;
		case Type::KIND_PTR:
		case Type::KIND_ARRAY:
		case Type::KIND_ALIAS:
			break;
		case Type::KIND_FUNC:
			func_sign_free(self->func);
			break;
		case Type::KIND_STRUCT:
		case Type::KIND_UNION:
			mn::buf_free(self->fields);
			break;
		case Type::KIND_ENUM:
			mn::buf_free(self->enum_values);
			break;
		default: assert(false && "unreachable"); break;
		}
		mn::free(self);
	}


	Type_Intern
	type_intern_new()
	{
		Type_Intern self = mn::alloc<IType_Intern>();
		self->types = mn::buf_new<Type*>();
		self->ptr_table = mn::map_new<Type*, Type*>();
		self->array_table = mn::map_new<Array_Sign, Type*, Array_Sign_Hasher>();
		self->func_table = mn::map_new<Func_Sign, Type*, Func_Sign_Hasher>();
		return self;
	}

	void
	type_intern_free(Type_Intern self)
	{
		destruct(self->types);
		mn::map_free(self->ptr_table);
		mn::map_free(self->array_table);
		mn::map_free(self->func_table);
		mn::free(self);
	}

	Type*
	type_intern_ptr(Type_Intern self, Type* base)
	{
		if(auto it = mn::map_lookup(self->ptr_table, base))
			return it->value;

		Type* new_type = type_ptr(base);
		mn::buf_push(self->types, new_type);
		mn::map_insert(self->ptr_table, base, new_type);
		return new_type;
	}

	Type*
	type_intern_array(Type_Intern self, const Array_Sign& sign)
	{
		if(auto it = mn::map_lookup(self->array_table, sign))
			return it->value;

		Type* new_type = type_array(sign);
		mn::buf_push(self->types, new_type);
		mn::map_insert(self->array_table, sign, new_type);
		return new_type;
	}

	Type*
	type_intern_func(Type_Intern self, Func_Sign& func)
	{
		if(auto it = mn::map_lookup(self->func_table, func))
		{
			func_sign_free(func);
			return it->value;
		}

		Type* new_type = type_func(func);
		mn::buf_push(self->types, new_type);
		mn::map_insert(self->func_table, func, new_type);
		return new_type;
	}

	Type*
	type_intern_incomplete(Type_Intern self, Type* type)
	{
		return *mn::buf_push(self->types, type);
	}
}