#include "cypher/Val.h"
#include "cypher/Func.h"

namespace cypher
{
	//API
	Val
	val_arg(Type t, Func func, size_t ix)
	{
		Val self{};
		self.kind = Val::KIND_ARG;
		self.type = t;
		self.arg = { func, ix };
		return self;
	}

	Val
	val_ins(Type t, Func func, Ins_ID id)
	{
		Val self{};
		self.kind = Val::KIND_INS;
		self.type = t;
		self.ins = { func, id };
		return self;
	}
}