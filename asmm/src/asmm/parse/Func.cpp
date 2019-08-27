#include "asmm/parse/Func.h"

namespace asmm
{
	//API
	Func
	func_new(const Tkn& name)
	{
		Func self{};
		self.name = name;
		self.ins = mn::buf_new<Ins>();
		return self;
	}

	void
	func_free(Func& self)
	{
		mn::buf_free(self.ins);
	}
}