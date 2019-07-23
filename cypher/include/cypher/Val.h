#pragma once

#include <stddef.h>

namespace cypher
{
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

	inline static Val
	val_arg(Func func, size_t ix)
	{
		Val self{};
		self.kind = Val::KIND_ARG;
		self.arg = { func, ix };
		return self;
	}

	inline static Val
	val_ins(Func func, Ins_ID id)
	{
		Val self{};
		self.kind = Val::KIND_INS;
		self.ins = { func, id };
		return self;
	}
}
