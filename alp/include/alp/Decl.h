#pragma once

#include "alp/Exports.h"
#include "alp/Tkn.h"
#include "alp/Pos.h"
#include "alp/Rng.h"

#include <mn/Str.h>

namespace alp
{
	typedef struct IDecl *Decl;
	struct IDecl
	{
		enum KIND
		{
			KIND_NONE,
			KIND_TOKEN,
		};

		KIND kind;
		Tkn name;
		Rng rng;
		Pos pos;
		union
		{
			mn::Str regex;
		};
	};

	ALP_EXPORT Decl
	decl_token(const Tkn& name, const mn::Str& regex);

	ALP_EXPORT void
	decl_free(Decl self);

	inline static void
	destruct(Decl self)
	{
		decl_free(self);
	}
}