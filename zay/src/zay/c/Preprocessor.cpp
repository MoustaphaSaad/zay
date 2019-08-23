#include "zay/c/Preprocessor.h"

#include <mn/Memory.h>
#include <mn/Memory_Stream.h>
#include <mn/IO.h>

namespace zay::c
{
	using namespace mn;

	struct Preprocessor
	{
		Memory_Stream out;
		Str code;
		const char* it;
		Rune c;
	};

	inline static Preprocessor
	preprocessor_new(const Str& code)
	{
		Preprocessor self{};
		self.out = memory_stream_new();
		self.code = code;
		self.it = self.code.ptr;
		self.c = *self.it;
		return self;
	}

	inline static void
	preprocessor_free(Preprocessor& self)
	{
		memory_stream_free(self.out);
	}

	inline static void
	destruct(Preprocessor& self)
	{
		preprocessor_free(self);
	}


	inline static bool
	is_whitespace(Rune c)
	{
		return (
			c == ' ' ||
			c == '\n' ||
			c == '\r' ||
			c == '\t' ||
			c == '\v'
		);
	}

	inline static bool
	is_letter(Rune c)
	{
		return (rune_is_letter(c) || c == '_');
	}

	inline static bool
	is_digit(Rune c)
	{
		return (c >= '0' && c <= '9');
	}

	inline static bool
	eof(Preprocessor& self)
	{
		return self.it >= end(self.code);
	}

	inline static bool
	eat(Preprocessor& self)
	{
		if(eof(self))
			return false;

		// const char* prev_it = self.it;
		// Rune prev_c = rune_read(self.it);

		self.it = rune_next(self.it);
		self.c = rune_read(self.it);

		//ignore line positions for now
		// self->pos.col++;

		// if(prev_c == '\n')
		// {
		// 	self->pos.col = 1;
		// 	self->pos.line++;
		// 	src_line_end(self->src, prev_it);
		// 	src_line_begin(self->src, self->it);
		// }
		return true;
	}

	inline static void
	skip_ws(Preprocessor& self)
	{
		while(is_whitespace(self.c))
			if(eat(self) == false)
				break;
	}


	//API
	Str
	preprocess(const Str& code, Allocator allocator)
	{
		Preprocessor self = preprocessor_new(code);

		const char* prev_it = self.it;
		for(;eof(self) == false;)
		{
			if(self.c == '#')
			{
				memory_stream_write(self.out, Block{ (void*)prev_it, size_t(self.it - prev_it) });

				eat(self); //eat the #

				//ignore the preprocessor directive
				bool ignore = false;
				while(self.c != '\n' || ignore)
				{
					if(self.c == '\\')
					{
						ignore = true;
					}
					else if(self.c == '\r' && ignore)
					{
						if (eat(self) == false)
							break;
						if (self.c == '\n')
							if (eat(self) == false)
								break;
						continue;
					}
					else
					{
						ignore = false;
					}
					if(eat(self) == false)
						break;
				}
				eat(self); //eat the \n

				prev_it = self.it;
				continue;
			}

			eat(self);
		}
		memory_stream_write(self.out, Block{ (void*)prev_it, size_t(self.it - prev_it) });

		Str res = str_from_c(memory_stream_ptr(self.out), allocator);
		preprocessor_free(self);
		return res;
	}
}