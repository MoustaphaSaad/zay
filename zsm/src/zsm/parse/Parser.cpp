#include "zsm/parse/Parser.h"

#include <mn/Memory.h>
#include <mn/IO.h>
#include <mn/Defer.h>

namespace zsm
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

	inline static Tkn
	parser_reg(Parser self)
	{
		auto tkn = parser_look(self);
		if(is_reg(tkn))
			return parser_eat(self);
		
		src_err(self->src, err_tkn(tkn, mn::strf("expected a register, but found '{}'", tkn.str)));
		return Tkn{};
	}

	inline static Ins
	parser_ins(Parser self)
	{
		auto tkn = parser_look(self);
		Ins ins{};
		if(is_mode3(tkn))
		{
			ins.op = parser_eat(self);
			ins.mode3.dst = parser_reg(self);
			ins.mode3.op1 = parser_reg(self);
			ins.mode3.op2 = parser_reg(self);
		}
		else if(is_mode2(tkn))
		{
			ins.op = parser_eat(self);
			ins.mode2.dst = parser_reg(self);
			ins.mode2.src = parser_reg(self);
		}
		else if(is_const(tkn))
		{
			ins.op = parser_eat(self);
			ins.constant.dst = parser_reg(self);
			ins.constant.value = parser_eat_must(self, Tkn::KIND_INTEGER);
		}
		else if(tkn.kind == Tkn::KIND_KEYWORD_JT || tkn.kind == Tkn::KIND_KEYWORD_JF)
		{
			ins.op = parser_eat(self);
			ins.cond_jump.cond = parser_reg(self);
			ins.cond_jump.label = parser_eat_must(self, Tkn::KIND_ID);
		}
		else if(tkn.kind == Tkn::KIND_KEYWORD_JUMP)
		{
			ins.op = parser_eat(self);
			ins.jump.label = parser_eat_must(self, Tkn::KIND_ID);
		}
		else if(tkn.kind == Tkn::KIND_KEYWORD_HALT ||
				tkn.kind == Tkn::KIND_KEYWORD_RET ||
				tkn.kind == Tkn::KIND_KEYWORD_CALL)
		{
			ins.op = parser_eat(self);
		}
		else if(tkn.kind == Tkn::KIND_ID)
		{
			ins.op = parser_eat(self);
			parser_eat_must(self, Tkn::KIND_COLON);
		}
		else
		{
			src_err(self->src, err_tkn(tkn, mn::strf("'{}' unknown instruction", tkn.str)));
		}
		
		
		return ins;
	}


	//API
	Parser
	parser_new(Src src)
	{
		Parser self = alloc<IParser>();
		self->src = src;
		self->tkns = clone(src->tkns);
		self->ix = 0;
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
		free(self);
	}

	Func
	parser_func(Parser self)
	{
		parser_eat_must(self, Tkn::KIND_KEYWORD_FUNC);
		auto name = parser_eat_must(self, Tkn::KIND_ID);
		Func res = func_new(name);
		while(parser_eat_kind(self, Tkn::KIND_KEYWORD_END) == false)
		{
			mn::buf_push(res.ins, parser_ins(self));
		}
		return res;
	}
};
