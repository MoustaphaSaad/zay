#pragma once

#include <assert.h>
#include <stdint.h>

namespace cypher
{
	//VM Register
	struct VM_Value
	{
		enum KIND
		{
			KIND_VOID,
			KIND_INT8,
			KIND_UINT8,
			KIND_INT16,
			KIND_UINT16,
			KIND_INT32,
			KIND_UINT32,
			KIND_INT64,
			KIND_UINT64,
			KIND_FLOAT32,
			KIND_FLOAT64
		};

		KIND kind;
		union
		{
			int8_t i8;
			uint8_t u8;
			int16_t i16;
			uint16_t u16;
			int32_t i32;
			uint32_t u32;
			int64_t i64;
			uint64_t u64;
			float f32;
			double f64;
		};
	};

	inline static VM_Value
	vm_int8(int8_t v)
	{
		VM_Value r{};
		r.kind = VM_Value::KIND_INT8;
		r.i8 = v;
		return r;
	}

	inline static VM_Value
	vm_uint8(uint8_t v)
	{
		VM_Value r{};
		r.kind = VM_Value::KIND_UINT8;
		r.u8 = v;
		return r;
	}

	inline static VM_Value
	vm_int16(int16_t v)
	{
		VM_Value r{};
		r.kind = VM_Value::KIND_INT16;
		r.i16 = v;
		return r;
	}

	inline static VM_Value
	vm_uint16(uint16_t v)
	{
		VM_Value r{};
		r.kind = VM_Value::KIND_UINT16;
		r.u16 = v;
		return r;
	}

	inline static VM_Value
	vm_int32(int32_t v)
	{
		VM_Value r{};
		r.kind = VM_Value::KIND_INT32;
		r.i32 = v;
		return r;
	}

	inline static VM_Value
	vm_uint32(uint32_t v)
	{
		VM_Value r{};
		r.kind = VM_Value::KIND_UINT32;
		r.u32 = v;
		return r;
	}

	inline static VM_Value
	vm_int64(int64_t v)
	{
		VM_Value r{};
		r.kind = VM_Value::KIND_INT64;
		r.i64 = v;
		return r;
	}

	inline static VM_Value
	vm_uint64(uint64_t v)
	{
		VM_Value r{};
		r.kind = VM_Value::KIND_UINT64;
		r.u64 = v;
		return r;
	}

	inline static VM_Value
	vm_float32(float v)
	{
		VM_Value r{};
		r.kind = VM_Value::KIND_FLOAT32;
		r.f32 = v;
		return r;
	}

	inline static VM_Value
	vm_float64(double v)
	{
		VM_Value r{};
		r.kind = VM_Value::KIND_FLOAT64;
		r.f64 = v;
		return r;
	}

	inline static VM_Value
	vm_value_add(VM_Value a, VM_Value b)
	{
		assert(a.kind == b.kind);
		switch(a.kind)
		{
		case VM_Value::KIND_VOID: return VM_Value{};
		case VM_Value::KIND_INT8: return vm_int8(a.i8 + b.i8);
		case VM_Value::KIND_UINT8: return vm_uint8(a.u8 + b.u8);
		case VM_Value::KIND_INT16: return vm_int16(a.i16 + b.i16);
		case VM_Value::KIND_UINT16: return vm_uint16(a.u16 + b.u16);
		case VM_Value::KIND_INT32: return vm_int32(a.i32 + b.i32);
		case VM_Value::KIND_UINT32: return vm_uint32(a.u32 + b.u32);
		case VM_Value::KIND_INT64: return vm_int64(a.i64 + b.i64);
		case VM_Value::KIND_UINT64: return vm_uint64(a.u64 + b.u64);
		case VM_Value::KIND_FLOAT32: return vm_float32(a.f32 + b.f32);
		case VM_Value::KIND_FLOAT64: return vm_float64(a.f64 + b.f64);
		default: assert(false && "unimplemented"); return VM_Value{};
		}
	}
}