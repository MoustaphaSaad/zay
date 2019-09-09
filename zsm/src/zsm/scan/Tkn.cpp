#include "zsm/scan/Tkn.h"
#include "zsm/scan/Token_Listing.h"

namespace zsm
{
	const char* const Tkn::NAMES[] = {
		#define TOKEN(k, s) s
			TOKEN_LISTING
		#undef TOKEN
	};
}
