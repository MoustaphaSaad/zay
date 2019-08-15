#pragma once

#include "vm/Exports.h"
#include "vm/Register.h"

#include <mn/Buf.h>

namespace vm
{
	struct State
	{
		mn::Buf<uint8_t> code;
		mn::Buf<uint8_t> stack;
		Register r[int(REGISTER::R_COUNT)];
		bool last_error;
	};

	VM_EXPORT State
	state_new(const mn::Buf<uint8_t> &program);

	VM_EXPORT void
	state_free(State& self);

	inline static void
	destruct(State& self)
	{
		state_free(self);
	}

	VM_EXPORT bool
	state_exec(State& self);
}
