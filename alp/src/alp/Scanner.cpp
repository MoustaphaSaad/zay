#include "alp/Scanner.h"
#include "alp/Tkn.h"

#include <mn/Memory.h>
#include <mn/IO.h>

#include <string.h>

namespace alp
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
			prev = self->c;
			scanner_eat(self);
		}

		end_it = self->it;
		scanner_eat(self); //for the "
		return str_intern(self->src->str_table, begin_it, end_it);
	}

	inline static const char*
	scanner_set(Scanner self)
	{
		const char* begin_it = self->it;
		const char* end_it = self->it;

		Rune prev = self->c;
		//eat all runes even those escaped by \ like \]
		while(self->c != ']' || prev == '\\')
		{
			scanner_eat(self);
			prev = self->c;
		}

		end_it = self->it;
		scanner_eat(self); //for the ]
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
		else
		{
			//now for operators
			Rune c = self->c;
			Pos begin_pos = self->pos;
			scanner_eat(self);
			bool no_intern = false;

			switch(c)
			{
			case '(':
				tkn.kind = Tkn::KIND_OPEN_PAREN;
				tkn.str = "(";
				no_intern = true;
				break;
			case ')':
				tkn.kind = Tkn::KIND_CLOSE_PAREN;
				tkn.str = ")";
				no_intern = true;
				break;
			case ';':
				tkn.kind = Tkn::KIND_SEMICOLON;
				tkn.str = ";";
				no_intern = true;
				break;
			case '=':
				tkn.kind = Tkn::KIND_EQUAL;
				tkn.str = "=";
				no_intern = true;
				break;
			case '.':
				tkn.kind = Tkn::KIND_DOT;
				tkn.str = ".";
				no_intern = true;
				break;
			case '+':
				tkn.kind = Tkn::KIND_PLUS;
				tkn.str = "+";
				no_intern = true;
				break;
			case '*':
				tkn.kind = Tkn::KIND_STAR;
				tkn.str = "*";
				no_intern = true;
				break;
			case '|':
				tkn.kind = Tkn::KIND_OR;
				tkn.str = "|";
				no_intern = true;
				break;
			case '?':
				tkn.kind = Tkn::KIND_OPTIONAL;
				tkn.str = "?";
				no_intern = true;
				break;
			case '[':
				tkn.kind = Tkn::KIND_SET;
				if(self->c == '^')
				{
					tkn.kind = Tkn::KIND_NSET;
					scanner_eat(self);
				}
				scanner_set(self);
				break;
			case '"':
				tkn.kind = Tkn::KIND_STRING;
				tkn.str = scanner_string(self);
				no_intern = true;
				break;
			case '/':
				if(self->c == '/')
				{
					tkn.kind = Tkn::KIND_COMMENT;
					scanner_eat(self); //for the second /
					tkn.str = scanner_comment(self);
					no_intern = true;
				}
				else
				{
					src_err(self->src, Err{
						begin_pos,
						Rng{},
						strf("illegal rune {:c}", c)
					});
				}
				break;
			default:
				src_err(self->src, Err{
					begin_pos,
					Rng{},
					strf("illegal rune {:c}", c)
				});
				break;
			}

			if(no_intern == false)
				tkn.str = str_intern(self->src->str_table, tkn.rng.begin, self->it);
		}

		tkn.rng.end = self->it;
		return tkn;
	}
}