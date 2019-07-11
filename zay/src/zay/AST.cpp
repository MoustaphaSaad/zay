#include "zay/AST.h"
#include "zay/Type_Intern.h"

#include <mn/Memory.h>

#include <assert.h>

namespace zay
{
	using namespace mn;

	AST
	ast_new()
	{
		AST self = alloc<IAST>();
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