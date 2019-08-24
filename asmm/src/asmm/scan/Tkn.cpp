#include "asmm/scan/Tkn.h"
#include "asmm/scan/Token_Listing.h"

namespace asmm
{
	const char* const Tkn::NAMES[] = {
		#define TOKEN(k, s) s
			TOKEN_LISTING
		#undef TOKEN
	};
}
