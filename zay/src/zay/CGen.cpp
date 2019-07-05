#include "zay/CGen.h"

#include <mn/Memory.h>

#include <assert.h>

namespace zay
{
	using namespace mn;

	inline static void
	cgen_indent(CGen self)
	{
		for (size_t i = 0; i < self->indent; ++i)
			vprintf(self->out, "\t");
	}

	inline static void
	cgen_newline(CGen self)
	{
		vprintf(self->out, "\n");
		cgen_indent(self);
	}

	inline static Scope
	cgen_scope(CGen self)
	{
		return buf_top(self->scope_stack);
	}

	inline static void
	cgen_scope_enter(CGen self, Scope scope)
	{
		buf_push(self->scope_stack, scope);
	}

	inline static void
	cgen_scope_leave(CGen self)
	{
		buf_pop(self->scope_stack);
	}

	inline static Sym
	cgen_sym(CGen self, const char* name)
	{
		return scope_find(cgen_scope(self), name);
	}

	inline static Str
	cdecl_paren(Str str, Rune c)
	{
		if (c && c != '[')
			return str_tmpf("({})", str);
		return str;
	}

	inline static Str
	cgen_write_field(CGen self, Type type, const Str& name)
	{
		Rune c = 0;
		if (name.count > 0)
			c = rune_read(name.ptr);

		switch(type->kind)
		{
		case IType::KIND_PTR:
			return cgen_write_field(self, type->ptr.base, cdecl_paren(str_tmpf("*{}", name), c));
		case IType::KIND_ARRAY:
			return cgen_write_field(self, type->array.base, cdecl_paren(str_tmpf("{}[{}]", name, type->array.count), c));
		case IType::KIND_FUNC:
		{
			Str res = str_tmpf("(*{})(", name);
			if(type->func.args.count == 0)
			{
				str_push(res, "ZayVoid");
			}
			else
			{
				for(size_t i = 0; i < type->func.args.count; ++i)
				{
					if (i != 0)
						strf(res, ", ");
					strf(res, "{}", cgen_write_field(self, type->func.args[i], str_lit("")));
				}
			}
			res = strf(res, ")");
			return cgen_write_field(self, type->func.ret, res);
		}
		case IType::KIND_VOID:
			if (name.count)
				return str_tmpf("ZayVoid {}", name);
			else
				return str_tmpf("ZayVoid");
		case IType::KIND_BOOL:
			if (name.count)
				return str_tmpf("ZayBool {}", name);
			else
				return str_tmpf("ZayBool");
		case IType::KIND_INT:
			if (name.count)
				return str_tmpf("ZayInt {}", name);
			else
				return str_tmpf("ZayInt");
		case IType::KIND_UINT:
			if (name.count)
				return str_tmpf("ZayUint {}", name);
			else
				return str_tmpf("ZayUint");
		case IType::KIND_INT8:
			if (name.count)
				return str_tmpf("ZayInt8 {}", name);
			else
				return str_tmpf("ZayInt8");
		case IType::KIND_UINT8:
			if (name.count)
				return str_tmpf("ZayUint8 {}", name);
			else
				return str_tmpf("ZayUint8");
		case IType::KIND_INT16:
			if (name.count)
				return str_tmpf("ZayInt16 {}", name);
			else
				return str_tmpf("ZayInt16");
		case IType::KIND_UINT16:
			if (name.count)
				return str_tmpf("ZayUint16 {}", name);
			else
				return str_tmpf("ZayUint16");
		case IType::KIND_INT32:
			if (name.count)
				return str_tmpf("ZayInt32 {}", name);
			else
				return str_tmpf("ZayInt32");
		case IType::KIND_UINT32:
			if (name.count)
				return str_tmpf("ZayUint32 {}", name);
			else
				return str_tmpf("ZayUint32");
		case IType::KIND_INT64:
			if (name.count)
				return str_tmpf("ZayInt64 {}", name);
			else
				return str_tmpf("ZayInt64");
		case IType::KIND_UINT64:
			if (name.count)
				return str_tmpf("ZayUint64 {}", name);
			else
				return str_tmpf("ZayUint64");
		case IType::KIND_FLOAT32:
			if (name.count)
				return str_tmpf("ZayFloat32 {}", name);
			else
				return str_tmpf("ZayFloat32");
		case IType::KIND_FLOAT64:
			if (name.count)
				return str_tmpf("ZayFloat64 {}", name);
			else
				return str_tmpf("ZayFloat64");
		case IType::KIND_STRING:
			if (name.count)
				return str_tmpf("ZayString {}", name);
			else
				return str_tmpf("ZayString");
		case IType::KIND_STRUCT:
			if (name.count)
				return str_tmpf("{} {}", type->aggregate.sym->name, name);
			else
				return str_tmpf("{}", type->aggregate.sym->name);
		default:
			return str_tmpf("<UNDEFINED TYPE>");
		}
	}

	inline static void
	cgen_decl_struct_gen(CGen self, Decl decl)
	{
		assert(decl->kind == IDecl::KIND_STRUCT);
		vprintf(self->out, "typedef struct {} {{", decl->name.str);

		self->indent++;

		Sym sym = cgen_sym(self, decl->name.str);
		for(Field_Sign& f: sym->type->aggregate.fields)
		{
			cgen_newline(self);
			vprintf(self->out, "{}", cgen_write_field(self, f.type, str_lit(f.name)));
			vprintf(self->out, ";");
		}
		memory::tmp()->free_all();

		self->indent--;
		cgen_newline(self);

		vprintf(self->out, "} {};", decl->name.str);
	}

	inline static void
	cgen_decl_gen(CGen self, Decl decl)
	{
		switch(decl->kind)
		{
		case IDecl::KIND_STRUCT:
			cgen_decl_struct_gen(self, decl);
			break;
		default:
			assert(false && "unreachable");
			break;
		}
	}


	//API
	CGen
	cgen_new(Src src)
	{
		CGen self = alloc<ICGen>();
		self->indent = 0;
		self->src = src;
		self->out = stream_memory_new();
		self->scope_stack = buf_new<Scope>();

		buf_push(self->scope_stack, src_scope_of(src, nullptr));
		return self;
	}

	void
	cgen_free(CGen self)
	{
		stream_free(self->out);
		buf_free(self->scope_stack);
		free(self);
	}

	void
	cgen_gen(CGen self)
	{
		for (Decl d : self->src->reachable_decls)
			cgen_decl_gen(self, d);
	}
}