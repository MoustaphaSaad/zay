#include "zay/AST_Lisp.h"

#include <mn/IO.h>

#include <assert.h>

namespace zay
{
	using namespace mn;

	inline static void
	ast_lisp_indent(AST_Lisp& self)
	{
		for(size_t i = 0; i < self.level; ++i)
			vprintf(self.out, "\t");
	}

	inline static void
	ast_lisp_type(AST_Lisp& self, const Type_Sign& type)
	{
		vprintf(self.out, "(type ");
		for(size_t i = 0; i < type.count; ++i)
		{
			switch(type[i].kind)
			{
			case Type_Atom::KIND_NAMED:
				vprintf(self.out, " {}", type[i].named.str);
				break;

			case Type_Atom::KIND_PTR:
				vprintf(self.out, "*");
				break;

			case Type_Atom::KIND_ARRAY:
				vprintf(self.out, "[{}]", type[i].count.str);
				break;

			default:
				assert(false && "unreachable");
				break;
			}
		}
		vprintf(self.out, ")");
	}

	inline static void
	ast_lisp_field(AST_Lisp& self, const Field& field)
	{
		ast_lisp_indent(self);
		vprintf(self.out, "(field ");

		for(size_t i = 0; i < field.ids.count; ++i)
		{
			if(i != 0)
				vprintf(self.out, ", ");
			vprintf(self.out, "{}", field.ids[i].str);
		}

		vprintf(self.out, ": ");

		ast_lisp_type(self, field.type);

		vprintf(self.out, ")");
	}

	inline static void
	ast_lisp_aggregate(AST_Lisp& self, Decl decl)
	{
		ast_lisp_indent(self);

		const char* type;
		switch(decl->kind)
		{
		case IDecl::KIND_STRUCT: type = "struct"; break;
		case IDecl::KIND_UNION: type = "union"; break;
		default: assert(false && "unreachable"); type = "<UNIDENTIFIED>"; break;
		}

		vprintf(self.out, "({}", type);

		const char* name = "<UNNAMED>";
		if(decl->name)
			name = decl->name.str;
		vprintf(self.out, " {}\n", name);

		self.level++;

		for(size_t i = 0; i < decl->struct_decl.count; ++i)
		{
			ast_lisp_field(self, decl->struct_decl[i]);
			vprintf(self.out, "\n");
		}

		self.level--;

		ast_lisp_indent(self);
		vprintf(self.out, ")");
	}


	//API
	void
	ast_lisp_decl(AST_Lisp& self, Decl decl)
	{
		switch(decl->kind)
		{
		case IDecl::KIND_STRUCT:
		case IDecl::KIND_UNION:
			ast_lisp_aggregate(self, decl);
			break;

		default: assert(false && "unreachable"); break;
		}
	}
}