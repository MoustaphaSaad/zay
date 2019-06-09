#pragma once

#include "zay/Tkn.h"

namespace zay
{
	typedef struct ISym* Sym;
	struct ISym
	{
		enum STATE
		{
			STATE_UNRESOLVED,
			STATE_RESOLVING,
			STATE_RESOLVED
		};

		Tkn name;
		STATE state;
	};
}