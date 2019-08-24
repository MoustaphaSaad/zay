#pragma once

#include "asmm/scan/Tkn.h"

namespace asmm
{
	struct Ins
	{
		Tkn op;
		union
		{
			//a mode 3 is "op register1(dest) register2(op1) register3(op2)"
			struct
			{
				Tkn dst, op1, op2;
			} mode3;
		};
	};
}