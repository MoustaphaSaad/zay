#pragma once

#include "zay/ir/ID.h"

#include <stdint.h>

namespace zay::ir
{
	struct Val
	{
		enum KIND
		{
			KIND_NONE,
			KIND_ARG,
			KIND_IMM,
			KIND_INS
		};

		KIND kind;
		union
		{
			struct
			{
				Func_ID func_id;
				size_t ix;
			} arg;
			
			union
			{
				int32_t i32;
			} imm;

			struct
			{
				Func_ID func_id;
				Ins_ID ins_id;
			} ins;
		};
	};
}
