#pragma once

#include "vm/Exports.h"

#include <mn/Map.h>
#include <mn/Buf.h>
#include <mn/Str.h>

namespace vm
{
	//fixup is a patch work where the loader will subtitute the address at the offset
	//in the specified function with the address of the target symbol
	struct Fixup
	{
		mn::Str func;
		mn::Str target;
		uint32_t offset;
	};

	VM_EXPORT Fixup
	fixup_new(const mn::Str& func, const mn::Str& target, uint32_t offset);

	VM_EXPORT void
	fixup_free(Fixup& self);

	inline static void
	destruct(Fixup& self)
	{
		fixup_free(self);
	}

	//program is collection of functions and fixup requests
	struct Program
	{
		mn::Map<mn::Str, mn::Buf<uint8_t>> funcs;
		mn::Buf<Fixup> fixups;
	};

	VM_EXPORT Program
	program_new();

	VM_EXPORT void
	program_free(Program& self);

	inline static void
	destruct(Program& self)
	{
		program_free(self);
	}

	VM_EXPORT void
	program_func_add(Program& self, const mn::Str& name, const mn::Buf<uint8_t> &bytes);

	VM_EXPORT void
	program_fixup(Program& self, const Fixup& fixup);

	VM_EXPORT Program
	program_save(const mn::Str& filename);
}