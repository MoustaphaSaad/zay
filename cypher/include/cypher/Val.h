#pragma once

#include "cypher/Exports.h"

#include <stddef.h>

namespace cypher
{
	typedef struct IType* Type;
	typedef struct IFunc* Func;
	typedef size_t Ins_ID;

	struct Val
	{
		enum KIND
		{
			KIND_NONE,
			KIND_ARG,
			KIND_INS
		};

		KIND kind;
		Type type;
		union
		{
			struct
			{
				Func func;
				size_t ix;
			} arg;

			struct
			{
				Func func;
				Ins_ID ins_id;
			} ins;
		};
	};

	CYPHER_EXPORT Val
	val_arg(Type t, Func func, size_t ix);

	CYPHER_EXPORT Val
	val_ins(Type t, Func func, Ins_ID id);
}
