#include "asmm/Gen.h"

#include <vm/Program.h>
#include <vm/ISA.h>
#include <vm/Register.h>

#include <mn/Str.h>
#include <mn/Map.h>
#include <mn/IO.h>
#include <mn/Defer.h>

#include <assert.h>

namespace asmm
{
	struct Patch
	{
		Tkn label;
		size_t offset;
	};

	struct Gen
	{
		Src src;

		mn::Buf<uint8_t> bytes;
		mn::Buf<Patch> patches;
		mn::Map<const char*, uint64_t> labels;
	};

	inline static Gen
	gen_new(Src src)
	{
		Gen self{};
		self.src = src;
		self.bytes = mn::buf_new<uint8_t>();
		self.patches = mn::buf_new<Patch>();
		self.labels = mn::map_new<const char*, size_t>();
		return self;
	}

	inline static void
	gen_free(Gen& self)
	{
		mn::buf_free(self.bytes);
		mn::buf_free(self.patches);
		mn::map_free(self.labels);
	}

	inline static void
	destruct(Gen& self)
	{
		gen_free(self);
	}

	inline static void
	gen_register_func(Gen& self, const Func& f)
	{
		vm::program_func_add(self.src->program, mn::str_from_c(f.name.str), self.bytes);
		self.bytes = mn::buf_new<uint8_t>();
		mn::buf_clear(self.patches);
		mn::map_clear(self.labels);
	}

	inline static void
	push8(Gen& self, uint8_t v)
	{
		mn::buf_push(self.bytes, v);
	}

	inline static void
	push16(Gen& self, uint16_t v)
	{
		mn::buf_push(self.bytes, uint8_t(v));
		mn::buf_push(self.bytes, uint8_t(v >> 8));
	}

	inline static void
	push32(Gen& self, uint32_t v)
	{
		mn::buf_push(self.bytes, uint8_t(v));
		mn::buf_push(self.bytes, uint8_t(v >> 8));
		mn::buf_push(self.bytes, uint8_t(v >> 16));
		mn::buf_push(self.bytes, uint8_t(v >> 24));
	}

	inline static void
	push64(Gen& self, uint64_t v)
	{
		mn::buf_push(self.bytes, uint8_t(v));
		mn::buf_push(self.bytes, uint8_t(v >> 8));
		mn::buf_push(self.bytes, uint8_t(v >> 16));
		mn::buf_push(self.bytes, uint8_t(v >> 24));
		mn::buf_push(self.bytes, uint8_t(v >> 32));
		mn::buf_push(self.bytes, uint8_t(v >> 40));
		mn::buf_push(self.bytes, uint8_t(v >> 48));
		mn::buf_push(self.bytes, uint8_t(v >> 56));
	}

	inline static void
	patch64(Gen& self, size_t offset, uint64_t v)
	{
		self.bytes[offset + 0] = uint8_t(v);
		self.bytes[offset + 1] = uint8_t(v >> 8);
		self.bytes[offset + 2] = uint8_t(v >> 16);
		self.bytes[offset + 3] = uint8_t(v >> 24);
		self.bytes[offset + 4] = uint8_t(v >> 32);
		self.bytes[offset + 5] = uint8_t(v >> 40);
		self.bytes[offset + 6] = uint8_t(v >> 48);
		self.bytes[offset + 7] = uint8_t(v >> 56);
	}

	inline static void
	push_jump_patch(Gen& self, const Tkn& t)
	{
		mn::buf_push(self.patches, Patch{t, self.bytes.count});
		push64(self, 0);
	}

	inline static vm::ISA
	tkn_to_isa(const Tkn& op)
	{
		switch(op.kind)
		{
		case Tkn::KIND_KEYWORD_I8_ADD: return vm::ISA::I8_ADD;
		case Tkn::KIND_KEYWORD_I16_ADD: return vm::ISA::I16_ADD;
		case Tkn::KIND_KEYWORD_I32_ADD: return vm::ISA::I32_ADD;
		case Tkn::KIND_KEYWORD_I64_ADD: return vm::ISA::I64_ADD;
		case Tkn::KIND_KEYWORD_U8_ADD: return vm::ISA::U8_ADD;
		case Tkn::KIND_KEYWORD_U16_ADD: return vm::ISA::U16_ADD;
		case Tkn::KIND_KEYWORD_U32_ADD: return vm::ISA::U32_ADD;
		case Tkn::KIND_KEYWORD_U64_ADD: return vm::ISA::U64_ADD;

		case Tkn::KIND_KEYWORD_I8_SUB: return vm::ISA::I8_SUB;
		case Tkn::KIND_KEYWORD_I16_SUB: return vm::ISA::I16_SUB;
		case Tkn::KIND_KEYWORD_I32_SUB: return vm::ISA::I32_SUB;
		case Tkn::KIND_KEYWORD_I64_SUB: return vm::ISA::I64_SUB;
		case Tkn::KIND_KEYWORD_U8_SUB: return vm::ISA::U8_SUB;
		case Tkn::KIND_KEYWORD_U16_SUB: return vm::ISA::U16_SUB;
		case Tkn::KIND_KEYWORD_U32_SUB: return vm::ISA::U32_SUB;
		case Tkn::KIND_KEYWORD_U64_SUB: return vm::ISA::U64_SUB;

		case Tkn::KIND_KEYWORD_I8_MUL: return vm::ISA::I8_MUL;
		case Tkn::KIND_KEYWORD_I16_MUL: return vm::ISA::I16_MUL;
		case Tkn::KIND_KEYWORD_I32_MUL: return vm::ISA::I32_MUL;
		case Tkn::KIND_KEYWORD_I64_MUL: return vm::ISA::I64_MUL;
		case Tkn::KIND_KEYWORD_U8_MUL: return vm::ISA::U8_MUL;
		case Tkn::KIND_KEYWORD_U16_MUL: return vm::ISA::U16_MUL;
		case Tkn::KIND_KEYWORD_U32_MUL: return vm::ISA::U32_MUL;
		case Tkn::KIND_KEYWORD_U64_MUL: return vm::ISA::U64_MUL;

		case Tkn::KIND_KEYWORD_I8_DIV: return vm::ISA::I8_DIV;
		case Tkn::KIND_KEYWORD_I16_DIV: return vm::ISA::I16_DIV;
		case Tkn::KIND_KEYWORD_I32_DIV: return vm::ISA::I32_DIV;
		case Tkn::KIND_KEYWORD_I64_DIV: return vm::ISA::I64_DIV;
		case Tkn::KIND_KEYWORD_U8_DIV: return vm::ISA::U8_DIV;
		case Tkn::KIND_KEYWORD_U16_DIV: return vm::ISA::U16_DIV;
		case Tkn::KIND_KEYWORD_U32_DIV: return vm::ISA::U32_DIV;
		case Tkn::KIND_KEYWORD_U64_DIV: return vm::ISA::U64_DIV;

		case Tkn::KIND_KEYWORD_I8_MOD: return vm::ISA::I8_MOD;
		case Tkn::KIND_KEYWORD_I16_MOD: return vm::ISA::I16_MOD;
		case Tkn::KIND_KEYWORD_I32_MOD: return vm::ISA::I32_MOD;
		case Tkn::KIND_KEYWORD_I64_MOD: return vm::ISA::I64_MOD;
		case Tkn::KIND_KEYWORD_U8_MOD: return vm::ISA::U8_MOD;
		case Tkn::KIND_KEYWORD_U16_MOD: return vm::ISA::U16_MOD;
		case Tkn::KIND_KEYWORD_U32_MOD: return vm::ISA::U32_MOD;
		case Tkn::KIND_KEYWORD_U64_MOD: return vm::ISA::U64_MOD;

		case Tkn::KIND_KEYWORD_I8_EQ: return vm::ISA::U8_EQ;
		case Tkn::KIND_KEYWORD_I16_EQ: return vm::ISA::U16_EQ;
		case Tkn::KIND_KEYWORD_I32_EQ: return vm::ISA::U32_EQ;
		case Tkn::KIND_KEYWORD_I64_EQ: return vm::ISA::U64_EQ;
		case Tkn::KIND_KEYWORD_U8_EQ: return vm::ISA::U8_EQ;
		case Tkn::KIND_KEYWORD_U16_EQ: return vm::ISA::U16_EQ;
		case Tkn::KIND_KEYWORD_U32_EQ: return vm::ISA::U32_EQ;
		case Tkn::KIND_KEYWORD_U64_EQ: return vm::ISA::U64_EQ;

		case Tkn::KIND_KEYWORD_I8_LT: return vm::ISA::I8_LT;
		case Tkn::KIND_KEYWORD_I16_LT: return vm::ISA::I16_LT;
		case Tkn::KIND_KEYWORD_I32_LT: return vm::ISA::I32_LT;
		case Tkn::KIND_KEYWORD_I64_LT: return vm::ISA::I64_LT;
		case Tkn::KIND_KEYWORD_U8_LT: return vm::ISA::U8_LT;
		case Tkn::KIND_KEYWORD_U16_LT: return vm::ISA::U16_LT;
		case Tkn::KIND_KEYWORD_U32_LT: return vm::ISA::U32_LT;
		case Tkn::KIND_KEYWORD_U64_LT: return vm::ISA::U64_LT;

		case Tkn::KIND_KEYWORD_I8_GT: return vm::ISA::I8_GT;
		case Tkn::KIND_KEYWORD_I16_GT: return vm::ISA::I16_GT;
		case Tkn::KIND_KEYWORD_I32_GT: return vm::ISA::I32_GT;
		case Tkn::KIND_KEYWORD_I64_GT: return vm::ISA::I64_GT;
		case Tkn::KIND_KEYWORD_U8_GT: return vm::ISA::U8_GT;
		case Tkn::KIND_KEYWORD_U16_GT: return vm::ISA::U16_GT;
		case Tkn::KIND_KEYWORD_U32_GT: return vm::ISA::U32_GT;
		case Tkn::KIND_KEYWORD_U64_GT: return vm::ISA::U64_GT;

		case Tkn::KIND_KEYWORD_I8_LE: return vm::ISA::I8_LE;
		case Tkn::KIND_KEYWORD_I16_LE: return vm::ISA::I16_LE;
		case Tkn::KIND_KEYWORD_I32_LE: return vm::ISA::I32_LE;
		case Tkn::KIND_KEYWORD_I64_LE: return vm::ISA::I64_LE;
		case Tkn::KIND_KEYWORD_U8_LE: return vm::ISA::U8_LE;
		case Tkn::KIND_KEYWORD_U16_LE: return vm::ISA::U16_LE;
		case Tkn::KIND_KEYWORD_U32_LE: return vm::ISA::U32_LE;
		case Tkn::KIND_KEYWORD_U64_LE: return vm::ISA::U64_LE;

		case Tkn::KIND_KEYWORD_I8_GE: return vm::ISA::I8_GE;
		case Tkn::KIND_KEYWORD_I16_GE: return vm::ISA::I16_GE;
		case Tkn::KIND_KEYWORD_I32_GE: return vm::ISA::I32_GE;
		case Tkn::KIND_KEYWORD_I64_GE: return vm::ISA::I64_GE;
		case Tkn::KIND_KEYWORD_U8_GE: return vm::ISA::U8_GE;
		case Tkn::KIND_KEYWORD_U16_GE: return vm::ISA::U16_GE;
		case Tkn::KIND_KEYWORD_U32_GE: return vm::ISA::U32_GE;
		case Tkn::KIND_KEYWORD_U64_GE: return vm::ISA::U64_GE;

		case Tkn::KIND_KEYWORD_I16_X8: return vm::ISA::I16_X8;
		case Tkn::KIND_KEYWORD_I32_X8: return vm::ISA::I32_X8;
		case Tkn::KIND_KEYWORD_I32_X16: return vm::ISA::I32_X16;
		case Tkn::KIND_KEYWORD_I64_X8: return vm::ISA::I64_X8;
		case Tkn::KIND_KEYWORD_I64_X16: return vm::ISA::I64_X16;
		case Tkn::KIND_KEYWORD_I64_X32: return vm::ISA::I64_X32;
		case Tkn::KIND_KEYWORD_U16_X8: return vm::ISA::U16_X8;
		case Tkn::KIND_KEYWORD_U32_X8: return vm::ISA::U32_X8;
		case Tkn::KIND_KEYWORD_U32_X16: return vm::ISA::U32_X16;
		case Tkn::KIND_KEYWORD_U64_X8: return vm::ISA::U64_X8;
		case Tkn::KIND_KEYWORD_U64_X16: return vm::ISA::U64_X16;
		case Tkn::KIND_KEYWORD_U64_X32: return vm::ISA::U64_X32;

		case Tkn::KIND_KEYWORD_I8_CONST: return vm::ISA::U8_CONST;
		case Tkn::KIND_KEYWORD_I16_CONST: return vm::ISA::U16_CONST;
		case Tkn::KIND_KEYWORD_I32_CONST: return vm::ISA::U32_CONST;
		case Tkn::KIND_KEYWORD_I64_CONST: return vm::ISA::U64_CONST;
		case Tkn::KIND_KEYWORD_U8_CONST: return vm::ISA::U8_CONST;
		case Tkn::KIND_KEYWORD_U16_CONST: return vm::ISA::U16_CONST;
		case Tkn::KIND_KEYWORD_U32_CONST: return vm::ISA::U32_CONST;
		case Tkn::KIND_KEYWORD_U64_CONST: return vm::ISA::U64_CONST;
		
		case Tkn::KIND_KEYWORD_JT: return vm::ISA::JT;
		case Tkn::KIND_KEYWORD_JF: return vm::ISA::JF;
		case Tkn::KIND_KEYWORD_JUMP: return vm::ISA::JUMP;
		case Tkn::KIND_KEYWORD_HALT: return vm::ISA::HALT;

		default:
			assert(false && "unreachable");
			return vm::ISA::HALT;
		}
	}

	inline static vm::REGISTER
	tkn_to_reg(const Tkn& t)
	{
		switch(t.kind)
		{
		case Tkn::KIND_KEYWORD_R0: return vm::REGISTER::R0;
		case Tkn::KIND_KEYWORD_R1: return vm::REGISTER::R1;
		case Tkn::KIND_KEYWORD_R2: return vm::REGISTER::R2;
		case Tkn::KIND_KEYWORD_R3: return vm::REGISTER::R3;
		case Tkn::KIND_KEYWORD_R4: return vm::REGISTER::R4;
		case Tkn::KIND_KEYWORD_R5: return vm::REGISTER::R5;
		case Tkn::KIND_KEYWORD_R6: return vm::REGISTER::R6;
		case Tkn::KIND_KEYWORD_R7: return vm::REGISTER::R7;
		case Tkn::KIND_KEYWORD_IP: return vm::REGISTER::IP;
		case Tkn::KIND_KEYWORD_SP: return vm::REGISTER::SP;
		
		default:
			assert(false && "unreachable");
			return vm::REGISTER::R_COUNT;
		}
	}

	inline static void
	gen_ins(Gen& self, const Ins& i)
	{
		auto opcode = tkn_to_isa(i.op);
		if(is_mode3(i.op))
		{
			push8(self, (uint8_t)opcode);
			push8(self, (uint8_t)tkn_to_reg(i.mode3.dst));
			push8(self, (uint8_t)tkn_to_reg(i.mode3.op1));
			push8(self, (uint8_t)tkn_to_reg(i.mode3.op2));
		}
		else if(is_mode2(i.op))
		{
			push8(self, (uint8_t)opcode);
			push8(self, (uint8_t)tkn_to_reg(i.mode2.dst));
			push8(self, (uint8_t)tkn_to_reg(i.mode2.src));
		}
		else if(is_const(i.op))
		{
			push8(self, (uint8_t)opcode);
			push8(self, (uint8_t)tkn_to_reg(i.constant.dst));
			if(i.op.kind == Tkn::KIND_KEYWORD_I8_CONST)
			{
				int8_t v{};
				if(mn::reads(i.constant.value.str, v) == 0)
				{
					src_err(self.src, err_tkn(i.constant.value, mn::strf("'{}' invalid constant value", v)));
				}
				push8(self, uint8_t(v));
			}
			else if(i.op.kind == Tkn::KIND_KEYWORD_U8_CONST)
			{
				uint8_t v{};
				if(mn::reads(i.constant.value.str, v) == 0)
				{
					src_err(self.src, err_tkn(i.constant.value, mn::strf("'{}' invalid constant value", v)));
				}
				push8(self, uint8_t(v));
			}
			else if(i.op.kind == Tkn::KIND_KEYWORD_I16_CONST)
			{
				int16_t v{};
				if(mn::reads(i.constant.value.str, v) == 0)
				{
					src_err(self.src, err_tkn(i.constant.value, mn::strf("'{}' invalid constant value", v)));
				}
				push16(self, uint16_t(v));
			}
			else if(i.op.kind == Tkn::KIND_KEYWORD_U16_CONST)
			{
				uint16_t v{};
				if(mn::reads(i.constant.value.str, v) == 0)
				{
					src_err(self.src, err_tkn(i.constant.value, mn::strf("'{}' invalid constant value", v)));
				}
				push16(self, uint16_t(v));
			}
			else if(i.op.kind == Tkn::KIND_KEYWORD_I32_CONST)
			{
				int32_t v{};
				if(mn::reads(i.constant.value.str, v) == 0)
				{
					src_err(self.src, err_tkn(i.constant.value, mn::strf("'{}' invalid constant value", v)));
				}
				push32(self, uint32_t(v));
			}
			else if(i.op.kind == Tkn::KIND_KEYWORD_U32_CONST)
			{
				uint32_t v{};
				if(mn::reads(i.constant.value.str, v) == 0)
				{
					src_err(self.src, err_tkn(i.constant.value, mn::strf("'{}' invalid constant value", v)));
				}
				push32(self, uint32_t(v));
			}
			else if(i.op.kind == Tkn::KIND_KEYWORD_I64_CONST)
			{
				int64_t v{};
				if(mn::reads(i.constant.value.str, v) == 0)
				{
					src_err(self.src, err_tkn(i.constant.value, mn::strf("'{}' invalid constant value", v)));
				}
				push64(self, uint64_t(v));
			}
			else if(i.op.kind == Tkn::KIND_KEYWORD_U64_CONST)
			{
				uint64_t v{};
				if(mn::reads(i.constant.value.str, v) == 0)
				{
					src_err(self.src, err_tkn(i.constant.value, mn::strf("'{}' invalid constant value", v)));
				}
				push64(self, uint64_t(v));
			}
			else
			{
				assert(false && "unreachable");
			}
		}
		else if(i.op.kind == Tkn::KIND_KEYWORD_JT || i.op.kind == Tkn::KIND_KEYWORD_JF)
		{
			push8(self, (uint8_t)opcode);
			push8(self, (uint8_t)tkn_to_reg(i.cond_jump.cond));
			push_jump_patch(self, i.cond_jump.label);
		}
		else if(i.op.kind == Tkn::KIND_KEYWORD_JUMP)
		{
			push8(self, (uint8_t)opcode);
			push_jump_patch(self, i.cond_jump.label);
		}
		else if(i.op.kind == Tkn::KIND_KEYWORD_HALT)
		{
			push8(self, (uint8_t)opcode);
		}
		else if(i.op.kind == Tkn::KIND_ID)
		{
			mn::map_insert(self.labels, i.op.str, uint64_t(self.bytes.count));
		}
		else
		{
			assert(false && "unreachable");
		}
	}

	inline static void
	gen_func(Gen& self, const Func& f)
	{
		for(const auto& i: f.ins)
			gen_ins(self, i);

		for(const auto& p: self.patches)
		{
			auto it = mn::map_lookup(self.labels, p.label.str);
			if(it == nullptr)
			{
				src_err(self.src, err_tkn(p.label, mn::strf("undefined label '{}'", p.label.str)));
				continue;
			}
			patch64(self, p.offset, it->value);
		}

		gen_register_func(self, f);
	}
	
	//API
	bool
	gen(Src src)
	{
		auto g = gen_new(src);
		mn_defer(gen_free(g));

		for(const auto& f: src->funcs)
		{
			gen_func(g, f);
			if(src_has_err(src))
				return false;
		}

		return true;
	}
}