#include "zay/Src.h"
#include "zay/parse/AST_Lisp.h"

#include <mn/Memory.h>
#include <mn/File.h>
#include <mn/Path.h>
#include <mn/Stream.h>
#include <mn/IO.h>
#include <mn/Defer.h>

namespace zay
{
	Src*
	src_from_file(const char* path)
	{
		auto self = mn::alloc<Src>();
		self->path = mn::str_from_c(path);
		self->content = mn::file_content_str(path);
		self->lines = mn::buf_new<Line>();
		self->str_table = mn::str_intern_new();
		self->errs = mn::buf_new<Err>();
		self->tkns = mn::buf_new<Tkn>();
		self->ast = ast_new();
		self->scopes = mn::buf_new<Scope>();
		self->scope_table = mn::map_new<void*, Scope>();
		self->type_table = type_intern_new();
		self->reachable_syms = mn::buf_new<Sym>();
		return self;
	}

	Src*
	src_from_str(const char* code)
	{
		auto self = mn::alloc<Src>();
		self->path = mn::str_from_c("<STRING>");
		self->content = mn::str_from_c(code);
		self->lines = mn::buf_new<Line>();
		self->str_table = mn::str_intern_new();
		self->errs = mn::buf_new<Err>();
		self->tkns = mn::buf_new<Tkn>();
		self->ast = ast_new();
		self->scopes = mn::buf_new<Scope>();
		self->scope_table = mn::map_new<void*, Scope>();
		self->type_table = type_intern_new();
		self->reachable_syms = mn::buf_new<Sym>();
		return self;
	}

	void
	src_free(Src *self)
	{
		mn::str_free(self->path);
		mn::str_free(self->content);
		mn::buf_free(self->lines);
		mn::str_intern_free(self->str_table);
		destruct(self->errs);
		mn::buf_free(self->tkns);
		ast_free(self->ast);
		destruct(self->scopes);
		mn::map_free(self->scope_table);
		type_intern_free(self->type_table);
		mn::buf_free(self->reachable_syms);
		mn::free(self);
	}

	mn::Str
	src_errs_dump(Src *self, mn::Allocator allocator)
	{
		auto out = mn::memory_stream_new(allocator);
		mn_defer(mn::memory_stream_free(out));
		for(const Err& e: self->errs)
		{
			Line l = self->lines[e.pos.line - 1];
			//we need to put ^^^ under the word the compiler means by the error
			if(e.rng.begin && e.rng.end)
			{
				mn::print_to(out, ">> {}\n", mn::str_from_substr(l.begin, l.end, mn::memory::tmp()));
				mn::print_to(out, ">> ");
				for(const char* it = l.begin; it != l.end; it = mn::rune_next(it))
				{
					auto c = mn::rune_read(it);
					if(it >= e.rng.begin && it < e.rng.end)
					{
						mn::print_to(out, "^");
					}
					else if(c == '\t')
					{
						mn::print_to(out, "\t");
					}
					else
					{
						mn::print_to(out, " ");
					}
				}
				mn::print_to(out, "\n");
			}
			mn::print_to(out, "Error[{}:{}:{}]: {}\n\n", self->path, e.pos.line, e.pos.col, e.msg);
		}
		return mn::memory_stream_str(out);
	}

	mn::Str
	src_tkns_dump(Src *self, mn::Allocator allocator)
	{
		//this is a tmp stream you can use to construct strings into
		auto out = mn::memory_stream_new(allocator);
		mn_defer(mn::memory_stream_free(out));
		for(const Tkn& t: self->tkns)
		{
			mn::print_to(
				out,
				"line: {}, col: {}, kind: \"{}\" str: \"{}\"\n",
				t.pos.line,
				t.pos.col,
				Tkn::NAMES[t.kind],
				t.str
			);
		}
		return mn::memory_stream_str(out);
	}

	mn::Str
	src_ast_dump(Src *self, mn::Allocator allocator)
	{
		auto out = mn::memory_stream_new(allocator);
		AST_Lisp writer = ast_lisp_new(out);
		for(size_t i = 0; i < self->ast->decls.count; ++i)
		{
			ast_lisp_decl(writer, self->ast->decls[i]);
			mn::print_to(out, "\n");
		}
		return mn::memory_stream_str(out);
	}
}