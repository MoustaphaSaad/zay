#pragma once

#include "alp/Exports.h"
#include "alp/Src.h"

#include <mn/Str.h>

namespace alp
{
	struct Code
	{
		mn::Str header, cpp;
	};

	ALP_EXPORT Code
	gen_std(Src src, mn::Allocator allocator);
}
