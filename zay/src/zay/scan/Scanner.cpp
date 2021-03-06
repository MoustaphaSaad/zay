#include "zay/scan/Scanner.h"

#include <mn/Memory.h>
#include <mn/IO.h>

#include <string.h>

namespace zay
{
	inline static bool
	is_whitespace(mn::Rune c)
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
	is_letter(mn::Rune c)
	{
		return (mn::rune_is_letter(c) || c == '_');
	}

	inline static bool
	is_digit(mn::Rune c)
	{
		return (c >= '0' && c <= '9');
	}

	inline static bool
	scanner_eat(Scanner *self)
	{
		if(scanner_eof(self))
			return false;

		const char* prev_it = self->it;
		auto prev_c = mn::rune_read(self->it);

		self->it = mn::rune_next(self->it);
		self->c = mn::rune_read(self->it);

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
	scanner_skip_whitespaces(Scanner *self)
	{
		while(is_whitespace(self->c))
			if(scanner_eat(self) == false)
				break;
	}

	inline static const char*
	scanner_id(Scanner *self)
	{
		const char* begin_it = self->it;
		while(is_letter(self->c) || is_digit(self->c))
			if(scanner_eat(self) == false)
				break;
		return mn::str_intern(self->src->str_table, begin_it, self->it);
	}

	inline static int
	digit_value(mn::Rune c)
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
	scanner_digits(Scanner *self, int base)
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
	scanner_num(Scanner *self, Tkn *tkn)
	{
		const char* begin_it = self->it;
		Pos begin_pos = self->pos;
		tkn->kind = Tkn::KIND_INTEGER;

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
						mn::strf("illegal int literal {:c}", self->c)
					});
				}
				tkn->str = mn::str_intern(self->src->str_table, begin_it, self->it);
				return;
			}

			//this is not a 0x number
			self->it = backup_it;
			self->c = mn::rune_read(self->it);
			self->pos = backup_pos;
		}

		//since this is not a 0x number
		//it might be an integer or float so parse a decimal number anyway
		if(scanner_digits(self, 10) == false)
		{
			src_err(self->src, Err{
				begin_pos,
				Rng{begin_it, self->it},
				mn::strf("illegal int literal {:c}", self->c)
			});
		}

		//float part
		if(self->c == '.')
		{
			tkn->kind = Tkn::KIND_FLOAT;
			scanner_eat(self); //for the .
			//parse the after . part
			if(scanner_digits(self, 10) == false)
			{
				src_err(self->src, Err{
					begin_pos,
					Rng{begin_it, self->it},
					mn::strf("illegal float literal {:c}", self->c)
				});
			}
		}

		//scientific notation part
		if(self->c == 'e' || self->c == 'E')
		{
			tkn->kind = Tkn::KIND_FLOAT;
			scanner_eat(self); //for the e
			if(self->c == '-' || self->c == '+')
				scanner_eat(self);
			if(scanner_digits(self, 10) == false)
			{
				src_err(self->src, Err{
					begin_pos,
					Rng{begin_it, self->it},
					mn::strf("illegal float literal {:c}", self->c)
				});
			}
		}

		//finished the parsing of the number whether it's a float or int
		tkn->str = mn::str_intern(self->src->str_table, begin_it, self->it);
	}

	inline static const char*
	scanner_comment(Scanner *self)
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
		return mn::str_intern(self->src->str_table, begin_it, end_it);
	}

	inline static const char*
	scanner_string(Scanner *self)
	{
		const char* begin_it = self->it;
		const char* end_it = self->it;

		auto prev = self->c;
		//eat all runes even those escaped by \ like \"
		while(self->c != '"' || prev == '\\')
		{
			scanner_eat(self);
			prev = self->c;
		}

		end_it = self->it;
		scanner_eat(self); //for the "
		return mn::str_intern(self->src->str_table, begin_it, end_it);
	}


	//API
	Tkn
	scanner_tkn(Scanner *self)
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
			scanner_num(self, &tkn);
		}
		else
		{
			//now for operators
			auto c = self->c;
			Pos begin_pos = self->pos;
			scanner_eat(self);
			bool no_intern = false;

			switch(c)
			{
			case '(': tkn.kind = Tkn::KIND_OPEN_PAREN; break;
			case ')': tkn.kind = Tkn::KIND_CLOSE_PAREN; break;
			case '{': tkn.kind = Tkn::KIND_OPEN_CURLY; break;
			case '}': tkn.kind = Tkn::KIND_CLOSE_CURLY; break;
			case '[': tkn.kind = Tkn::KIND_OPEN_BRACKET; break;
			case ']': tkn.kind = Tkn::KIND_CLOSE_BRACKET; break;
			case ':': tkn.kind = Tkn::KIND_COLON; break;
			case ';': tkn.kind = Tkn::KIND_SEMICOLON; break;
			case ',': tkn.kind = Tkn::KIND_COMMA; break;
			case '.': tkn.kind = Tkn::KIND_DOT; break;
			case '"':
				tkn.kind = Tkn::KIND_STRING;
				scanner_string(self);
				break;
			case '<':
				tkn.kind = Tkn::KIND_LESS;
				if(self->c == '=')
				{
					tkn.kind = Tkn::KIND_LESS_EQUAL;
					scanner_eat(self);
				}
				if(self->c == '<')
				{
					tkn.kind = Tkn::KIND_LEFT_SHIFT;
					scanner_eat(self);
					if(self->c == '=')
					{
						tkn.kind = Tkn::KIND_LEFT_SHIFT_EQUAL;
						scanner_eat(self);
					}
				}
				break;
			case '>':
				tkn.kind = Tkn::KIND_GREATER;
				if(self->c == '=')
				{
					tkn.kind = Tkn::KIND_GREATER_EQUAL;
					scanner_eat(self);
				}
				else if(self->c == '>')
				{
					tkn.kind = Tkn::KIND_RIGHT_SHIFT;
					scanner_eat(self);
					if(self->c == '=')
					{
						tkn.kind = Tkn::KIND_RIGHT_SHIFT_EQUAL;
						scanner_eat(self);
					}
				}
				break;
			case '=':
				tkn.kind = Tkn::KIND_EQUAL;
				if(self->c == '=')
				{
					tkn.kind = Tkn::KIND_EQUAL_EQUAL;
					scanner_eat(self);
				}
				break;
			case '+':
				tkn.kind = Tkn::KIND_PLUS;
				if(self->c == '=')
				{
					tkn.kind = Tkn::KIND_PLUS_EQUAL;
					scanner_eat(self);
				}
				else if(self->c == '+')
				{
					tkn.kind = Tkn::KIND_INC;
					scanner_eat(self);
				}
				break;
			case '-':
				tkn.kind = Tkn::KIND_MINUS;
				if(self->c == '=')
				{
					tkn.kind = Tkn::KIND_MINUS_EQUAL;
					scanner_eat(self);
				}
				else if(self->c == '-')
				{
					tkn.kind = Tkn::KIND_DEC;
					scanner_eat(self);
				}
				break;
			case '*':
				tkn.kind = Tkn::KIND_STAR;
				if(self->c == '=')
				{
					tkn.kind = Tkn::KIND_STAR_EQUAL;
					scanner_eat(self);
				}
				break;
			case '/':
				tkn.kind = Tkn::KIND_DIV;
				if(self->c == '=')
				{
					tkn.kind = Tkn::KIND_DIV_EQUAL;
					scanner_eat(self);
				}
				else if(self->c == '/')
				{
					tkn.kind = Tkn::KIND_COMMENT;
					scanner_eat(self); //for the second /
					tkn.str = scanner_comment(self);
					no_intern = true;
				}
				break;
			case '%':
				tkn.kind = Tkn::KIND_MOD;
				if(self->c == '=')
				{
					tkn.kind = Tkn::KIND_MOD_EQUAL;
					scanner_eat(self);
				}
				break;
			case '|':
				tkn.kind = Tkn::KIND_BIT_OR;
				if(self->c == '=')
				{
					tkn.kind = Tkn::KIND_BIT_OR_EQUAL;
					scanner_eat(self);
				}
				else if(self->c == '|')
				{
					tkn.kind = Tkn::KIND_LOGIC_OR;
					scanner_eat(self);
				}
				break;
			case '&':
				tkn.kind = Tkn::KIND_BIT_AND;
				if(self->c == '=')
				{
					tkn.kind = Tkn::KIND_BIT_AND_EQUAL;
					scanner_eat(self);
				}
				else if(self->c == '&')
				{
					tkn.kind = Tkn::KIND_LOGIC_AND;
					scanner_eat(self);
				}
				break;
			case '^':
				tkn.kind = Tkn::KIND_BIT_XOR;
				if(self->c == '=')
				{
					tkn.kind = Tkn::KIND_BIT_XOR_EQUAL;
					scanner_eat(self);
				}
				break;
			case '~':
				tkn.kind = Tkn::KIND_BIT_NOT;
				break;
			case '!':
				tkn.kind = Tkn::KIND_LOGIC_NOT;
				if(self->c == '=')
				{
					tkn.kind = Tkn::KIND_NOT_EQUAL;
					scanner_eat(self);
				}
				break;
			default:
				src_err(self->src, Err{
					begin_pos,
					Rng{},
					mn::strf("illegal rune {:c}", c)
				});
				break;
			}

			if(no_intern == false)
				tkn.str = mn::str_intern(self->src->str_table, tkn.rng.begin, self->it);
		}

		tkn.rng.end = self->it;
		return tkn;
	}
}