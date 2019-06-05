#include "zay/Tkn.h"
#include "zay/Token_Listing.h"

namespace zay
{
	const char* const Tkn::NAMES[] = {
		#define TOKEN(k, s) s
			TOKEN_LISTING
		#undef TOKEN
	};
}
