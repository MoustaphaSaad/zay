#pragma once

#include <stdint.h>

namespace vm
{
	union Register
	{
		int8_t  i8;
		int16_t i16;
		int32_t i32;
		int64_t i64;
		uint8_t  u8;
		uint16_t u16;
		uint32_t u32;
		uint64_t u64;
	};

	enum class REGISTER
	{
		//general purpose registers
		R0,
		R1,
		R2,
		R3,
		R4,
		R5,
		R6,
		R7,
		//instruction pointer
		IP,
		//stack pointer
		SP,
		R_COUNT
	};
}