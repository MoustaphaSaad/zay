#include "alp/Parser.h"
#include "alp/Tkn.h"

#include <mn/Memory.h>
#include <mn/IO.h>
#include <mn/Defer.h>

namespace alp
{
	using namespace mn;

	inline static Tkn
	parser_last_tkn(Parser self)
	{
		if(self->ix > 0)
			return self->tkns[self->ix - 1];
		return self->tkns[0];
	}

	inline static Tkn
	parser_look(Parser self, size_t k)
	{
		if(self->ix + k >= self->tkns.count)
			return Tkn{};
		return self->tkns[self->ix + k];
	}

	inline static Tkn
	parser_look(Parser self)
	{
		return parser_look(self, 0);
	}

	inline static Tkn
	parser_look_kind(Parser self, Tkn::KIND k)
	{
		Tkn t = parser_look(self);
		if(t.kind == k)
			return t;
		return Tkn{};
	}

	inline static Tkn
	parser_eat(Parser self)
	{
		if(self->ix >= self->tkns.count)
			return Tkn{};
		return self->tkns[self->ix++];
	}

	inline static Tkn
	parser_eat_kind(Parser self, Tkn::KIND kind)
	{
		Tkn t = parser_look(self);
		if(t.kind == kind)
			return parser_eat(self);
		return Tkn{};
	}

	inline static Tkn
	parser_eat_must(Parser self, Tkn::KIND kind)
	{
		if(self->ix >= self->tkns.count)
		{
			src_err(
				self->src,
				err_str(strf("expected '{}' but found EOF", Tkn::NAMES[kind]))
			);
			return Tkn{};
		}

		Tkn tkn = parser_eat(self);
		if(tkn.kind == kind)
			return tkn;

		src_err(
			self->src,
			err_tkn(tkn, strf("expected '{}' but found '{}'", Tkn::NAMES[kind], tkn.str))
		);
		return Tkn{};
	}

	Decl
	parser_token(Parser self)
	{
		parser_eat_must(self, Tkn::KIND_KEYWORD_TOKEN);
		Tkn name = parser_eat_must(self, Tkn::KIND_ID);
		parser_eat_must(self, Tkn::KIND_EQUAL);
		Str regex = str_new();
		while(parser_look_kind(self, Tkn::KIND_SEMICOLON) == false)
		{
			Tkn tkn = parser_eat(self);
			str_push(regex, tkn.str);
		}
		parser_eat_must(self, Tkn::KIND_SEMICOLON);
		return decl_token(name, regex);
	}


	//API
	Parser
	parser_new(Src src)
	{
		Parser self = alloc<IParser>();
		self->src = src;
		self->tkns = clone(src->tkns);
		self->ix = 0;
		self->typenames = buf_new<Tkn>();
		//ignore the comments and other unwanted tokens for parsing
		buf_remove_if(self->tkns, [](const Tkn& t) {
			return t.kind == Tkn::KIND_COMMENT;
		});

		return self;
	}

	void
	parser_free(Parser self)
	{
		buf_free(self->tkns);
		buf_free(self->typenames);
		free(self);
	}

	Decl
	parser_decl(Parser self)
	{
		Tkn tkn = parser_look(self);
		Decl res = nullptr;

		if(tkn.kind == Tkn::KIND_KEYWORD_TOKEN)
		{
			res = parser_token(self);
		}

		if(res)
		{
			res->rng = Rng{ tkn.rng.begin, parser_last_tkn(self).rng.end };
			res->pos = tkn.pos;
		}
		return res;
	}

	Tkn
	parser_pkg(Parser self)
	{
		parser_eat_must(self, Tkn::KIND_KEYWORD_PACKAGE);
		Tkn name = parser_eat_must(self, Tkn::KIND_ID);
		parser_eat_must(self, Tkn::KIND_SEMICOLON);
		return name;
	}
};
