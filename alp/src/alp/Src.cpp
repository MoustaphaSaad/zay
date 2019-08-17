#include "alp/Src.h"

#include <mn/Path.h>
#include <mn/Memory.h>
#include <mn/Memory_Stream.h>
#include <mn/Defer.h>
#include <mn/IO.h>

namespace alp
{
	using namespace mn;

	Src
	src_new(const mn::Str& content, const char* path)
	{
		Src self = alloc<ISrc>();
		self->path = str_from_c(path);
		self->content = content;
		self->lines = buf_new<Line>();
		self->str_table = str_intern_new();
		self->errs = buf_new<Err>();
		self->tkns = buf_new<Tkn>();
		self->ast = ast_new();
		return self;
	}

	//API
	Src
	src_from_file(const char* path)
	{
		return src_new(file_content_str(path), path);
	}

	Src
	src_from_str(const char* code)
	{
		return src_new(str_from_c(code), "<STRING>");
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
		free(self);
	}

	Str
	src_errs_dump(Src self, Allocator allocator)
	{
		Memory_Stream out = memory_stream_new(allocator);
		mn_defer(memory_stream_free(out));
		for(const Err& e: self->errs)
		{
			Line l = self->lines[e.pos.line - 1];
			//we need to put ^^^ under the word the compiler means by the error
			if(e.rng.begin && e.rng.end)
			{
				print_to(out, ">> {:.{}s}\n", l.begin, l.end - l.begin);
				print_to(out, ">> ");
				for(const char* it = l.begin; it != l.end; it = rune_next(it))
				{
					Rune c = rune_read(it);
					if(it >= e.rng.begin && it < e.rng.end)
					{
						print_to(out, "^");
					}
					else if(c == '\t')
					{
						print_to(out, "\t");
					}
					else
					{
						print_to(out, " ");
					}
				}
				print_to(out, "\n");
			}
			print_to(out, "Error[{}:{}:{}]: {}\n\n", self->path, e.pos.line, e.pos.col, e.msg);
		}
		return memory_stream_str(out);
	}

	Str
	src_tkns_dump(Src self, Allocator allocator)
	{
		//this is a tmp stream you can use to construct strings into
		Memory_Stream out = memory_stream_new(allocator);
		mn_defer(memory_stream_free(out));
		for(const Tkn& t: self->tkns)
		{
			print_to(
				out,
				"line: {}, col: {}, kind: \"{}\" str: \"{}\"\n",
				t.pos.line,
				t.pos.col,
				Tkn::NAMES[t.kind],
				t.str
			);
		}
		return memory_stream_str(out);
	}

	Str
	src_ast_dump(Src self, Allocator allocator)
	{
		//this is a tmp stream you can use to construct strings into
		Memory_Stream out = memory_stream_new(allocator);
		mn_defer(memory_stream_free(out));
		for(Decl d: self->ast->decls)
		{
			switch(d->kind)
			{
			case IDecl::KIND_TOKEN:
				print_to(out, "token {}: {};\n", d->name.str, d->regex);
				break;
			default:
				assert(false && "unreachable");
				break;
			}
		}
		return memory_stream_str(out);
	}
}