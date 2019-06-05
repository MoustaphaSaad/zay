#pragma once

#include "zay/Pos.h"
#include "zay/Rng.h"
#include "zay/Token_Listing.h"

namespace zay
{
	// Tkn is a zay source code token
	struct Tkn
	{
		enum KIND
		{
			#define TOKEN(k, s) KIND_##k
				TOKEN_LISTING
			#undef TOKEN
		};

		static const char* const NAMES[];

		const char* str;
		Rng rng;
		Pos pos;
	};
}

#undef TOKEN_LISTING
