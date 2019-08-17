#include "alp/Tkn.h"
#include "alp/Token_Listing.h"

namespace alp
{
	const char* const Tkn::NAMES[] = {
		#define TOKEN(k, s) s
			TOKEN_LISTING
		#undef TOKEN
	};
}
