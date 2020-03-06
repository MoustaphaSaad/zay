#include "zay/parse/AST.h"
#include "zay/typecheck/Type_Intern.h"

#include <mn/Memory.h>

#include <assert.h>

namespace zay
{
	AST
	ast_new()
	{
		AST self = mn::alloc<IAST>();
		self->package = Tkn{};
		self->decls = mn::buf_new<Decl>();
		return self;
	}

	void
	ast_free(AST self)
	{
		destruct(self->decls);
		mn::free(self);
	}
}