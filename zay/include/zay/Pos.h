#pragma once

#include <stdint.h>

namespace zay
{
	//Pos is a position in source code text
	struct Pos
	{
		uint32_t line, col;
	};
}