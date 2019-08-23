#include "asmm/scan/Scanner.h"

#include <mn/Memory.h>
#include <mn/IO.h>

#include <string.h>

namespace asmm
{
	using namespace mn;

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
	scanner_eat(Scanner self)
	{
		if(scanner_eof(self))
			return false;

		const char* prev_it = self->it;
		Rune prev_c = rune_read(self->it);

		self->it = rune_next(self->it);
		self->c = rune_read(self->it);

		self->pos.col++;

		if(prev_c == '\n')
		{
			self->pos.col = 1;
			self->pos.line++;
			src_line_end(self->src, prev_it);
			src_line_begin(self->src, self->it);
		}
		return true;
	}

	inline static void
	scanner_skip_whitespaces(Scanner self)
	{
		while(is_whitespace(self->c))
			if(scanner_eat(self) == false)
				break;
	}

	inline static const char*
	scanner_id(Scanner self)
	{
		const char* begin_it = self->it;
		while(is_letter(self->c) || is_digit(self->c))
			if(scanner_eat(self) == false)
				break;
		return str_intern(self->src->str_table, begin_it, self->it);
	}

	inline static int
	digit_value(Rune c)
	{
		if (is_digit(c))
			return c - '0';
		else if (c >= 'a' && c <= 'f')
			return c - 'a' + 10;
		else if (c >= 'A' && c <= 'F')
			return c - 'A' + 10;
		return 16;
	}

	inline static bool
	scanner_digits(Scanner self, int base)
	{
		bool found = false;
		while(digit_value(self->c) < base)
		{
			found = true;
			if(scanner_eat(self) == false)
				break;
		}
		return found;
	}

	inline static void
	scanner_num(Scanner self, Tkn& tkn)
	{
		const char* begin_it = self->it;
		Pos begin_pos = self->pos;
		tkn.kind = Tkn::KIND_INTEGER;

		if(self->c == '0')
		{
			const char* backup_it = self->it;
			Pos backup_pos = self->pos;
			scanner_eat(self); //for the 0

			int base = 0;
			switch(self->c)
			{
			case 'b': case 'B': base = 2; break;
			case 'o': case 'O': base = 8; break;
			case 'd': case 'D': base = 10; break;
			case 'x': case 'X': base = 16; break;
			default: break;
			}

			if(base != 0)
			{
				if(scanner_digits(self, base) == false)
				{
					src_err(self->src, Err{
						begin_pos,
						Rng{begin_it, self->it},
						strf("illegal int literal {:c}", self->c)
					});
				}
				tkn.str = str_intern(self->src->str_table, begin_it, self->it);
				return;
			}

			//this is not a 0x number
			self->it = backup_it;
			self->c = rune_read(self->it);
			self->pos = backup_pos;
		}

		//since this is not a 0x number
		//it might be an integer or float so parse a decimal number anyway
		if(scanner_digits(self, 10) == false)
		{
			src_err(self->src, Err{
				begin_pos,
				Rng{begin_it, self->it},
				strf("illegal int literal {:c}", self->c)
			});
		}

		//float part
		if(self->c == '.')
		{
			tkn.kind = Tkn::KIND_FLOAT;
			scanner_eat(self); //for the .
			//parse the after . part
			if(scanner_digits(self, 10) == false)
			{
				src_err(self->src, Err{
					begin_pos,
					Rng{begin_it, self->it},
					strf("illegal float literal {:c}", self->c)
				});
			}
		}

		//scientific notation part
		if(self->c == 'e' || self->c == 'E')
		{
			tkn.kind = Tkn::KIND_FLOAT;
			scanner_eat(self); //for the e
			if(self->c == '-' || self->c == '+')
				scanner_eat(self);
			if(scanner_digits(self, 10) == false)
			{
				src_err(self->src, Err{
					begin_pos,
					Rng{begin_it, self->it},
					strf("illegal float literal {:c}", self->c)
				});
			}
		}

		//finished the parsing of the number whether it's a float or int
		tkn.str = str_intern(self->src->str_table, begin_it, self->it);
	}

	inline static const char*
	scanner_comment(Scanner self)
	{
		const char* begin_it = self->it;
		const char* end_it = self->it;

		while(self->c != '\n')
		{
			end_it = self->it;
			//windows style line ending \r\n
			if(self->c == '\r')
			{
				if(scanner_eat(self) == false)
					break;
				if(self->c == '\n')
					break;
			}

			if(scanner_eat(self) == false)
				break;
		}

		scanner_eat(self); //for the \n
		return str_intern(self->src->str_table, begin_it, end_it);
	}

	inline static const char*
	scanner_string(Scanner self)
	{
		const char* begin_it = self->it;
		const char* end_it = self->it;

		Rune prev = self->c;
		//eat all runes even those escaped by \ like \"
		while(self->c != '"' || prev == '\\')
		{
			scanner_eat(self);
			prev = self->c;
		}

		end_it = self->it;
		scanner_eat(self); //for the "
		return str_intern(self->src->str_table, begin_it, end_it);
	}


	//API
	Scanner
	scanner_new(Src src)
	{
		Scanner self = mn::alloc<IScanner>();
		self->src = src;
		self->it = begin(self->src->content);
		self->c = rune_read(self->it);
		self->pos = Pos{1, 0};

		src_line_begin(self->src, self->it);
		return self;
	}

	void
	scanner_free(Scanner self)
	{
		free(self);
	}

	Tkn
	scanner_tkn(Scanner self)
	{
		scanner_skip_whitespaces(self);

		if(scanner_eof(self))
			return Tkn{};

		Tkn tkn{};
		tkn.pos = self->pos;
		tkn.rng.begin = self->it;

		if(is_letter(self->c))
		{
			tkn.kind = Tkn::KIND_ID;
			tkn.str = scanner_id(self);

			//let's loop over all the keywords and check them
			for(size_t i = size_t(Tkn::KIND_KEYWORDS__BEGIN + 1);
				i < size_t(Tkn::KIND_KEYWORDS__END);
				++i)
			{
				if(::strcmp(tkn.str, Tkn::NAMES[i]) == 0)
				{
					tkn.kind = Tkn::KIND(i);
					break;
				}
			}
		}
		else if(is_digit(self->c))
		{
			scanner_num(self, tkn);
		}
		else
		{
			// this is an illegal rune
			scanner_eat(self);
			src_err(self->src, Err{
				self->pos,
				Rng{},
				strf("illegal rune {:c}", self->c)
			});
		}

		tkn.rng.end = self->it;
		return tkn;
	}
}