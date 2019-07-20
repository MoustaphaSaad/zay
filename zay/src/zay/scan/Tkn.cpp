#include "zay/scan/Tkn.h"
#include "zay/scan/Token_Listing.h"

namespace zay
{
	const char* const Tkn::NAMES[] = {
		#define TOKEN(k, s) s
			TOKEN_LISTING
		#undef TOKEN
	};
}
