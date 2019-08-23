#pragma once

#include "asmm/scan/Pos.h"
#include "asmm/scan/Rng.h"
#include "asmm/scan/Token_Listing.h"

namespace asmm
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
}

#undef TOKEN_LISTING
