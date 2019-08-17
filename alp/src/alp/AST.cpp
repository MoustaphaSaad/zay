#include "alp/AST.h"

#include <mn/Memory.h>

namespace alp
{
	using namespace mn;

	AST
	ast_new()
	{
		AST self = alloc_zerod<IAST>();
		self->package = Tkn{};
		self->decls = buf_new<Decl>();
		return self;
	}

	void
	ast_free(AST self)
	{
		destruct(self->decls);
		free(self);
	}
}