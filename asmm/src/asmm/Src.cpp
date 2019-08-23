#include "asmm/Src.h"

#include <mn/Memory.h>
#include <mn/File.h>
#include <mn/Path.h>
#include <mn/Stream.h>
#include <mn/IO.h>
#include <mn/Defer.h>

namespace asmm
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
				print_to(out, ">> {}\n", str_from_substr(l.begin, l.end, memory::tmp()));
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
}