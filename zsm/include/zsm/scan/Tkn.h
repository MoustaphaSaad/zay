#pragma once

#include "zsm/Exports.h"
#include "zsm/scan/Pos.h"
#include "zsm/scan/Rng.h"
#include "zsm/scan/Token_Listing.h"

namespace zsm
{
	struct Tkn
	{
		enum KIND
		{
			#define TOKEN(k, s) KIND_##k
				TOKEN_LISTING
			#undef TOKEN
		};

		static const char* const NAMES[];

		KIND kind;
		const char* str;
		Rng rng;
		Pos pos;

		inline operator bool() const { return kind != KIND_NONE; }
	};

	inline static bool
	is_mode3(const Tkn& t)
	{
		return (
			t.kind == Tkn::KIND_KEYWORD_I8_ADD ||
			t.kind == Tkn::KIND_KEYWORD_I16_ADD ||
			t.kind == Tkn::KIND_KEYWORD_I32_ADD ||
			t.kind == Tkn::KIND_KEYWORD_I64_ADD ||
			t.kind == Tkn::KIND_KEYWORD_U8_ADD ||
			t.kind == Tkn::KIND_KEYWORD_U16_ADD ||
			t.kind == Tkn::KIND_KEYWORD_U32_ADD ||
			t.kind == Tkn::KIND_KEYWORD_U64_ADD ||
			t.kind == Tkn::KIND_KEYWORD_I8_SUB ||
			t.kind == Tkn::KIND_KEYWORD_I16_SUB ||
			t.kind == Tkn::KIND_KEYWORD_I32_SUB ||
			t.kind == Tkn::KIND_KEYWORD_I64_SUB ||
			t.kind == Tkn::KIND_KEYWORD_U8_SUB ||
			t.kind == Tkn::KIND_KEYWORD_U16_SUB ||
			t.kind == Tkn::KIND_KEYWORD_U32_SUB ||
			t.kind == Tkn::KIND_KEYWORD_U64_SUB ||
			t.kind == Tkn::KIND_KEYWORD_I8_MUL ||
			t.kind == Tkn::KIND_KEYWORD_I16_MUL ||
			t.kind == Tkn::KIND_KEYWORD_I32_MUL ||
			t.kind == Tkn::KIND_KEYWORD_I64_MUL ||
			t.kind == Tkn::KIND_KEYWORD_U8_MUL ||
			t.kind == Tkn::KIND_KEYWORD_U16_MUL ||
			t.kind == Tkn::KIND_KEYWORD_U32_MUL ||
			t.kind == Tkn::KIND_KEYWORD_U64_MUL ||
			t.kind == Tkn::KIND_KEYWORD_I8_DIV ||
			t.kind == Tkn::KIND_KEYWORD_I16_DIV ||
			t.kind == Tkn::KIND_KEYWORD_I32_DIV ||
			t.kind == Tkn::KIND_KEYWORD_I64_DIV ||
			t.kind == Tkn::KIND_KEYWORD_U8_DIV ||
			t.kind == Tkn::KIND_KEYWORD_U16_DIV ||
			t.kind == Tkn::KIND_KEYWORD_U32_DIV ||
			t.kind == Tkn::KIND_KEYWORD_U64_DIV ||
			t.kind == Tkn::KIND_KEYWORD_I8_MOD ||
			t.kind == Tkn::KIND_KEYWORD_I16_MOD ||
			t.kind == Tkn::KIND_KEYWORD_I32_MOD ||
			t.kind == Tkn::KIND_KEYWORD_I64_MOD ||
			t.kind == Tkn::KIND_KEYWORD_U8_MOD ||
			t.kind == Tkn::KIND_KEYWORD_U16_MOD ||
			t.kind == Tkn::KIND_KEYWORD_U32_MOD ||
			t.kind == Tkn::KIND_KEYWORD_U64_MOD ||
			t.kind == Tkn::KIND_KEYWORD_I8_EQ ||
			t.kind == Tkn::KIND_KEYWORD_I16_EQ ||
			t.kind == Tkn::KIND_KEYWORD_I32_EQ ||
			t.kind == Tkn::KIND_KEYWORD_I64_EQ ||
			t.kind == Tkn::KIND_KEYWORD_U8_EQ ||
			t.kind == Tkn::KIND_KEYWORD_U16_EQ ||
			t.kind == Tkn::KIND_KEYWORD_U32_EQ ||
			t.kind == Tkn::KIND_KEYWORD_U64_EQ ||
			t.kind == Tkn::KIND_KEYWORD_I8_LT ||
			t.kind == Tkn::KIND_KEYWORD_I16_LT ||
			t.kind == Tkn::KIND_KEYWORD_I32_LT ||
			t.kind == Tkn::KIND_KEYWORD_I64_LT ||
			t.kind == Tkn::KIND_KEYWORD_U8_LT ||
			t.kind == Tkn::KIND_KEYWORD_U16_LT ||
			t.kind == Tkn::KIND_KEYWORD_U32_LT ||
			t.kind == Tkn::KIND_KEYWORD_U64_LT ||
			t.kind == Tkn::KIND_KEYWORD_I8_GT ||
			t.kind == Tkn::KIND_KEYWORD_I16_GT ||
			t.kind == Tkn::KIND_KEYWORD_I32_GT ||
			t.kind == Tkn::KIND_KEYWORD_I64_GT ||
			t.kind == Tkn::KIND_KEYWORD_U8_GT ||
			t.kind == Tkn::KIND_KEYWORD_U16_GT ||
			t.kind == Tkn::KIND_KEYWORD_U32_GT ||
			t.kind == Tkn::KIND_KEYWORD_U64_GT ||
			t.kind == Tkn::KIND_KEYWORD_I8_LE ||
			t.kind == Tkn::KIND_KEYWORD_I16_LE ||
			t.kind == Tkn::KIND_KEYWORD_I32_LE ||
			t.kind == Tkn::KIND_KEYWORD_I64_LE ||
			t.kind == Tkn::KIND_KEYWORD_U8_LE ||
			t.kind == Tkn::KIND_KEYWORD_U16_LE ||
			t.kind == Tkn::KIND_KEYWORD_U32_LE ||
			t.kind == Tkn::KIND_KEYWORD_U64_LE ||
			t.kind == Tkn::KIND_KEYWORD_I8_GE ||
			t.kind == Tkn::KIND_KEYWORD_I16_GE ||
			t.kind == Tkn::KIND_KEYWORD_I32_GE ||
			t.kind == Tkn::KIND_KEYWORD_I64_GE ||
			t.kind == Tkn::KIND_KEYWORD_U8_GE ||
			t.kind == Tkn::KIND_KEYWORD_U16_GE ||
			t.kind == Tkn::KIND_KEYWORD_U32_GE ||
			t.kind == Tkn::KIND_KEYWORD_U64_GE
		);
	}

	inline static bool
	is_mode2(const Tkn& t)
	{
		return (
			t.kind == Tkn::KIND_KEYWORD_I16_X8 ||
			t.kind == Tkn::KIND_KEYWORD_I32_X8 ||
			t.kind == Tkn::KIND_KEYWORD_I32_X16 ||
			t.kind == Tkn::KIND_KEYWORD_I64_X8 ||
			t.kind == Tkn::KIND_KEYWORD_I64_X16 ||
			t.kind == Tkn::KIND_KEYWORD_I64_X32 ||
			t.kind == Tkn::KIND_KEYWORD_U16_X8 ||
			t.kind == Tkn::KIND_KEYWORD_U32_X8 ||
			t.kind == Tkn::KIND_KEYWORD_U32_X16 ||
			t.kind == Tkn::KIND_KEYWORD_U64_X8 ||
			t.kind == Tkn::KIND_KEYWORD_U64_X16 ||
			t.kind == Tkn::KIND_KEYWORD_U64_X32
		);
	}

	inline static bool
	is_const(const Tkn& t)
	{
		return (
			t.kind == Tkn::KIND_KEYWORD_I8_CONST ||
			t.kind == Tkn::KIND_KEYWORD_I16_CONST ||
			t.kind == Tkn::KIND_KEYWORD_I32_CONST ||
			t.kind == Tkn::KIND_KEYWORD_I64_CONST ||
			t.kind == Tkn::KIND_KEYWORD_U8_CONST ||
			t.kind == Tkn::KIND_KEYWORD_U16_CONST ||
			t.kind == Tkn::KIND_KEYWORD_U32_CONST ||
			t.kind == Tkn::KIND_KEYWORD_U64_CONST
		);
	}

	inline static bool
	is_reg(const Tkn& t)
	{
		return (
			t.kind == Tkn::KIND_KEYWORD_R0 ||
			t.kind == Tkn::KIND_KEYWORD_R1 ||
			t.kind == Tkn::KIND_KEYWORD_R2 ||
			t.kind == Tkn::KIND_KEYWORD_R3 ||
			t.kind == Tkn::KIND_KEYWORD_R4 ||
			t.kind == Tkn::KIND_KEYWORD_R5 ||
			t.kind == Tkn::KIND_KEYWORD_R6 ||
			t.kind == Tkn::KIND_KEYWORD_R7 ||
			t.kind == Tkn::KIND_KEYWORD_IP ||
			t.kind == Tkn::KIND_KEYWORD_SP
		);
	}
}

#undef TOKEN_LISTING
