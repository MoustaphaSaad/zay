#pragma once

#include "cypher/Exports.h"
#include "cypher/Val.h"
#include "cypher/Pkg.h"
#include "cypher/VM_Value.h"

#include <mn/Str.h>
#include <mn/Buf.h>

#include <stdint.h>

namespace cypher
{
	typedef struct IVM* VM;

	CYPHER_EXPORT VM
	vm_new();

	CYPHER_EXPORT void
	vm_free(VM self);

	inline static void
	destruct(VM self)
	{
		vm_free(self);
	}

	CYPHER_EXPORT VM_Value
	vm_func_run(VM self, Pkg pkg, const mn::Str& func_name, const mn::Buf<VM_Value>& args);

	inline static VM_Value
	vm_func_run(VM self, Pkg pkg, const char* func_name, const mn::Buf<VM_Value>& args)
	{
		return vm_func_run(self, pkg, mn::str_lit(func_name), args);
	}
}