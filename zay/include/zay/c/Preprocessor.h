#pragma once

#include "zay/Exports.h"

#include <mn/Str.h>

namespace zay::c
{
	ZAY_EXPORT mn::Str
	preprocess(const mn::Str& code, mn::Allocator allocator = mn::allocator_top());

	inline static mn::Str
	preprocess(const char* code, mn::Allocator allocator = mn::allocator_top())
	{
		return preprocess(mn::str_lit(code), allocator);
	}
}