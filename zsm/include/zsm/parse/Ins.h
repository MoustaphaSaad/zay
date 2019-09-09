#pragma once

#include "zsm/scan/Tkn.h"

namespace zsm
{
	struct Ins
	{
		Tkn op;
		union
		{
			//mode 3: op register1(dst) register2(op1) register3(op2)
			struct
			{
				Tkn dst, op1, op2;
			} mode3;
			
			//mode 2: op register1(dst) register2(src)
			struct
			{
				Tkn dst, src;
			} mode2;

			//const : op register1(dst) immediate value
			struct
			{
				Tkn dst, value;
			} constant;

			//conditional jump: op register1(cond) label
			struct
			{
				Tkn cond, label;
			} cond_jump;
			
			struct
			{
				Tkn label;
			} jump;
		};
	};
}