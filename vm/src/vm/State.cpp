#include "vm/State.h"
#include "vm/ISA.h"

#include <assert.h>

namespace vm
{
	using namespace mn;

	inline static Register&
	load_r(State& self, REGISTER r)
	{
		return self.r[int(r)];
	}

	inline static uint64_t&
	load_ip(State& self)
	{
		return load_r(self, REGISTER::IP).u64;
	}

	inline static uint8_t
	pop_byte(State& self)
	{
		if (load_ip(self) + sizeof(uint8_t) >= self.code.count)
			self.last_error = true;
		uint8_t res = *(uint8_t*)(self.code.ptr + load_ip(self));
		load_ip(self) += sizeof(res);
		return res;
	}

	inline static uint16_t
	pop_word(State& self)
	{
		if (load_ip(self) + sizeof(uint16_t) >= self.code.count)
			self.last_error = true;
		uint16_t res = *(uint16_t*)(self.code.ptr + load_ip(self));
		load_ip(self) += sizeof(res);
		return res;
	}

	inline static uint32_t
	pop_dword(State& self)
	{
		if (load_ip(self) + sizeof(uint32_t) >= self.code.count)
			self.last_error = true;
		uint32_t res = *(uint32_t*)(self.code.ptr + load_ip(self));
		load_ip(self) += sizeof(res);
		return res;
	}

	inline static uint64_t
	pop_qword(State& self)
	{
		if (load_ip(self) + sizeof(uint64_t) >= self.code.count)
			self.last_error = true;
		uint64_t res = *(uint64_t*)(self.code.ptr + load_ip(self));
		load_ip(self) += sizeof(res);
		return res;
	}

	inline static ISA
	pop_opcode(State& self)
	{
		return ISA(pop_byte(self));
	}

	inline static REGISTER
	pop_r(State& self)
	{
		return REGISTER(pop_byte(self));
	}


	#define BINARY_OP(STATE, PART, OP) do {\
		REGISTER res = pop_r((STATE));\
		REGISTER op1 = pop_r((STATE));\
		REGISTER op2 = pop_r((STATE));\
		load_r((STATE), res).PART = load_r((STATE), op1).PART OP load_r((STATE), op2).PART;\
	} while(0)


	//API
	State
	state_new(const Buf<uint8_t> &program)
	{
		State self{};
		self.code = program;
		self.stack = buf_with_count<uint8_t>(1024 * 1024 * 8);
		return self;
	}

	void
	state_free(State& self)
	{
		buf_free(self.code);
		buf_free(self.stack);
	}

	bool
	exec(State& self)
	{
		if (load_ip(self) >= self.code.count)
		{
			self.last_error = true;
			return false;
		}

		ISA opcode = pop_opcode(self);
		switch(opcode)
		{
		case ISA::I8_ADD:  BINARY_OP(self,  i8, +); break;
		case ISA::I16_ADD: BINARY_OP(self, i16, +); break;
		case ISA::I32_ADD: BINARY_OP(self, i32, +); break;
		case ISA::I64_ADD: BINARY_OP(self, i64, +); break;
		case ISA::U8_ADD:  BINARY_OP(self,  u8, +); break;
		case ISA::U16_ADD: BINARY_OP(self, u16, +); break;
		case ISA::U32_ADD: BINARY_OP(self, u32, +); break;
		case ISA::U64_ADD: BINARY_OP(self, u64, +); break;

		case ISA::I8_SUB:  BINARY_OP(self,  i8, -); break;
		case ISA::I16_SUB: BINARY_OP(self, i16, -); break;
		case ISA::I32_SUB: BINARY_OP(self, i32, -); break;
		case ISA::I64_SUB: BINARY_OP(self, i64, -); break;
		case ISA::U8_SUB:  BINARY_OP(self,  u8, -); break;
		case ISA::U16_SUB: BINARY_OP(self, u16, -); break;
		case ISA::U32_SUB: BINARY_OP(self, u32, -); break;
		case ISA::U64_SUB: BINARY_OP(self, u64, -); break;

		case ISA::I8_MUL:  BINARY_OP(self,  i8, *); break;
		case ISA::I16_MUL: BINARY_OP(self, i16, *); break;
		case ISA::I32_MUL: BINARY_OP(self, i32, *); break;
		case ISA::I64_MUL: BINARY_OP(self, i64, *); break;
		case ISA::U8_MUL:  BINARY_OP(self,  u8, *); break;
		case ISA::U16_MUL: BINARY_OP(self, u16, *); break;
		case ISA::U32_MUL: BINARY_OP(self, u32, *); break;
		case ISA::U64_MUL: BINARY_OP(self, u64, *); break;

		case ISA::I8_DIV:  BINARY_OP(self,  i8, /); break;
		case ISA::I16_DIV: BINARY_OP(self, i16, /); break;
		case ISA::I32_DIV: BINARY_OP(self, i32, /); break;
		case ISA::I64_DIV: BINARY_OP(self, i64, /); break;
		case ISA::U8_DIV:  BINARY_OP(self,  u8, /); break;
		case ISA::U16_DIV: BINARY_OP(self, u16, /); break;
		case ISA::U32_DIV: BINARY_OP(self, u32, /); break;
		case ISA::U64_DIV: BINARY_OP(self, u64, /); break;

		case ISA::I8_MOD:  BINARY_OP(self,  i8, %); break;
		case ISA::I16_MOD: BINARY_OP(self, i16, %); break;
		case ISA::I32_MOD: BINARY_OP(self, i32, %); break;
		case ISA::I64_MOD: BINARY_OP(self, i64, %); break;
		case ISA::U8_MOD:  BINARY_OP(self,  u8, %); break;
		case ISA::U16_MOD: BINARY_OP(self, u16, %); break;
		case ISA::U32_MOD: BINARY_OP(self, u32, %); break;
		case ISA::U64_MOD: BINARY_OP(self, u64, %); break;

		case ISA::U8_EQ:  BINARY_OP(self,  u8, ==); break;
		case ISA::U16_EQ: BINARY_OP(self, u16, ==); break;
		case ISA::U32_EQ: BINARY_OP(self, u32, ==); break;
		case ISA::U64_EQ: BINARY_OP(self, u64, ==); break;

		case ISA::I8_LT:  BINARY_OP(self,  i8, <); break;
		case ISA::I16_LT: BINARY_OP(self, i16, <); break;
		case ISA::I32_LT: BINARY_OP(self, i32, <); break;
		case ISA::I64_LT: BINARY_OP(self, i64, <); break;
		case ISA::U8_LT:  BINARY_OP(self,  u8, <); break;
		case ISA::U16_LT: BINARY_OP(self, u16, <); break;
		case ISA::U32_LT: BINARY_OP(self, u32, <); break;
		case ISA::U64_LT: BINARY_OP(self, u64, <); break;

		case ISA::I8_GT:  BINARY_OP(self,  i8, >); break;
		case ISA::I16_GT: BINARY_OP(self, i16, >); break;
		case ISA::I32_GT: BINARY_OP(self, i32, >); break;
		case ISA::I64_GT: BINARY_OP(self, i64, >); break;
		case ISA::U8_GT:  BINARY_OP(self,  u8, >); break;
		case ISA::U16_GT: BINARY_OP(self, u16, >); break;
		case ISA::U32_GT: BINARY_OP(self, u32, >); break;
		case ISA::U64_GT: BINARY_OP(self, u64, >); break;

		case ISA::I8_LE:  BINARY_OP(self,  i8, <=); break;
		case ISA::I16_LE: BINARY_OP(self, i16, <=); break;
		case ISA::I32_LE: BINARY_OP(self, i32, <=); break;
		case ISA::I64_LE: BINARY_OP(self, i64, <=); break;
		case ISA::U8_LE:  BINARY_OP(self,  u8, <=); break;
		case ISA::U16_LE: BINARY_OP(self, u16, <=); break;
		case ISA::U32_LE: BINARY_OP(self, u32, <=); break;
		case ISA::U64_LE: BINARY_OP(self, u64, <=); break;

		case ISA::I8_GE:  BINARY_OP(self,  i8, >=); break;
		case ISA::I16_GE: BINARY_OP(self, i16, >=); break;
		case ISA::I32_GE: BINARY_OP(self, i32, >=); break;
		case ISA::I64_GE: BINARY_OP(self, i64, >=); break;
		case ISA::U8_GE:  BINARY_OP(self,  u8, >=); break;
		case ISA::U16_GE: BINARY_OP(self, u16, >=); break;
		case ISA::U32_GE: BINARY_OP(self, u32, >=); break;
		case ISA::U64_GE: BINARY_OP(self, u64, >=); break;

		case ISA::U8_CONST:
		{
			REGISTER r = pop_r(self);
			load_r(self, r).u8 = pop_byte(self);
			break;
		}

		case ISA::U16_CONST:
		{
			REGISTER r = pop_r(self);
			load_r(self, r).u16 = pop_word(self);
			break;
		}

		case ISA::U32_CONST:
		{
			REGISTER r = pop_r(self);
			load_r(self, r).u32 = pop_dword(self);
			break;
		}

		case ISA::U64_CONST:
		{
			REGISTER r = pop_r(self);
			load_r(self, r).u64 = pop_qword(self);
			break;
		}


		//sign extension
		case ISA::I16_X8:
		{
			REGISTER dst = pop_r(self);
			REGISTER src = pop_r(self);
			load_r(self, dst).i16 = int16_t(load_r(self, src).i8);
			break;
		}

		case ISA::I32_X8:
		{
			REGISTER dst = pop_r(self);
			REGISTER src = pop_r(self);
			load_r(self, dst).i32 = int32_t(load_r(self, src).i8);
			break;
		}

		case ISA::I32_X16:
		{
			REGISTER dst = pop_r(self);
			REGISTER src = pop_r(self);
			load_r(self, dst).i32 = int32_t(load_r(self, src).i16);
			break;
		}

		case ISA::I64_X8:
		{
			REGISTER dst = pop_r(self);
			REGISTER src = pop_r(self);
			load_r(self, dst).i64 = int64_t(load_r(self, src).i8);
			break;
		}

		case ISA::I64_X16:
		{
			REGISTER dst = pop_r(self);
			REGISTER src = pop_r(self);
			load_r(self, dst).i64 = int64_t(load_r(self, src).i16);
			break;
		}

		case ISA::I64_X32:
		{
			REGISTER dst = pop_r(self);
			REGISTER src = pop_r(self);
			load_r(self, dst).i64 = int64_t(load_r(self, src).i32);
			break;
		}

		//zero extension
		case ISA::U16_X8:
		{
			REGISTER dst = pop_r(self);
			REGISTER src = pop_r(self);
			load_r(self, dst).u16 = int16_t(load_r(self, src).u8);
			break;
		}

		case ISA::U32_X8:
		{
			REGISTER dst = pop_r(self);
			REGISTER src = pop_r(self);
			load_r(self, dst).u32 = int32_t(load_r(self, src).u8);
			break;
		}

		case ISA::U32_X16:
		{
			REGISTER dst = pop_r(self);
			REGISTER src = pop_r(self);
			load_r(self, dst).u32 = int32_t(load_r(self, src).u16);
			break;
		}

		case ISA::U64_X8:
		{
			REGISTER dst = pop_r(self);
			REGISTER src = pop_r(self);
			load_r(self, dst).u64 = int64_t(load_r(self, src).u8);
			break;
		}

		case ISA::U64_X16:
		{
			REGISTER dst = pop_r(self);
			REGISTER src = pop_r(self);
			load_r(self, dst).u64 = int64_t(load_r(self, src).u16);
			break;
		}

		case ISA::U64_X32:
		{
			REGISTER dst = pop_r(self);
			REGISTER src = pop_r(self);
			load_r(self, dst).u64 = int64_t(load_r(self, src).u32);
			break;
		}

		//conditional jump
		case ISA::JT:
		{
			REGISTER cond = pop_r(self);
			uint64_t add = pop_qword(self);
			if(load_r(self, cond).u8 != 0)
				load_ip(self) = add;
			break;
		}

		case ISA::JF:
		{
			REGISTER cond = pop_r(self);
			uint64_t add = pop_qword(self);
			if(load_r(self, cond).u8 == 0)
				load_ip(self) = add;
			break;
		}


		//unconditional jump
		case ISA::JUMP:
		{
			load_ip(self) = pop_qword(self);
			break;
		}

		case ISA::HALT:
		{
			self.halted = true;
			break;
		}

		case ISA::IGL:
		default:
		{
			self.last_error = true;
			assert(false && "unreachable");
			break;
		}

		}
		return self.last_error == false;
	}

	bool
	run(State& self)
	{
		load_ip(self) = 0;
		self.halted = false;
		while (self.halted == false)
			if (exec(self) == false)
				break;
		return self.last_error == false;
	}
}
