#include "zay/Src.h"
#include "zay/parse/AST_Lisp.h"

#include <mn/Memory.h>
#include <mn/File.h>
#include <mn/Stream.h>
#include <mn/IO.h>

namespace zay
{
	using namespace mn;

	Src
	src_from_file(const char* path)
	{
		Src self = alloc<ISrc>();
		self->path = str_from_c(path);
		self->content = file_content_str(path);
		self->lines = buf_new<Line>();
		self->str_table = str_intern_new();
		self->errs = buf_new<Err>();
		self->tkns = buf_new<Tkn>();
		self->ast = ast_new();
		self->scopes = buf_new<Scope>();
		self->scope_table = map_new<void*, Scope>();
		self->type_table = type_intern_new();
		self->reachable_syms = buf_new<Sym>();
		return self;
	}

	Src
	src_from_str(const char* code)
	{
		Src self = alloc<ISrc>();
		self->path = str_from_c("<STRING>");
		self->content = str_from_c(code);
		self->lines = buf_new<Line>();
		self->str_table = str_intern_new();
		self->errs = buf_new<Err>();
		self->tkns = buf_new<Tkn>();
		self->ast = ast_new();
		self->scopes = buf_new<Scope>();
		self->scope_table = map_new<void*, Scope>();
		self->type_table = type_intern_new();
		self->reachable_syms = buf_new<Sym>();
		return self;
	}

	void
	src_free(Src self)
	{
		str_free(self->path);
		str_free(self->content);
		buf_free(self->lines);
		str_intern_free(self->str_table);
		destruct(self->errs);
		buf_free(self->tkns);
		ast_free(self->ast);
		destruct(self->scopes);
		map_free(self->scope_table);
		type_intern_free(self->type_table);
		buf_free(self->reachable_syms);
		free(self);
	}

	Str
	src_errs_dump(Src self, Allocator allocator)
	{
		Stream out = stream_tmp();
		for(const Err& e: self->errs)
		{
			Line l = self->lines[e.pos.line - 1];
			//we need to put ^^^ under the word the compiler means by the error
			if(e.rng.begin && e.rng.end)
			{
				vprintf(out, ">> {}\n", str_from_substr(l.begin, l.end, memory::tmp()));
				vprintf(out, ">> ");
				for(const char* it = l.begin; it != l.end; it = rune_next(it))
				{
					Rune c = rune_read(it);
					if(it >= e.rng.begin && it < e.rng.end)
					{
						vprintf(out, "^");
					}
					else if(c == '\t')
					{
						vprintf(out, "\t");
					}
					else
					{
						vprintf(out, " ");
					}
				}
				vprintf(out, "\n");
			}
			vprintf(out, "Error[{}:{}:{}]: {}\n\n", self->path, e.pos.line, e.pos.col, e.msg);
		}
		return str_from_c(stream_str(out), allocator);
	}

	Str
	src_tkns_dump(Src self, Allocator allocator)
	{
		//this is a tmp stream you can use to construct strings into
		Stream out = stream_tmp();
		for(const Tkn& t: self->tkns)
		{
			vprintf(
				out,
				"line: {}, col: {}, kind: \"{}\" str: \"{}\"\n",
				t.pos.line,
				t.pos.col,
				Tkn::NAMES[t.kind],
				t.str
			);
		}
		return str_from_c(stream_str(out), allocator);
	}

	Str
	src_ast_dump(Src self, Allocator allocator)
	{
		Stream out = stream_tmp();
		AST_Lisp writer = ast_lisp_new(out);
		for(size_t i = 0; i < self->ast->decls.count; ++i)
		{
			ast_lisp_decl(writer, self->ast->decls[i]);
			vprintf(out, "\n");
		}
		return str_from_c(stream_str(out), allocator);
	}
}