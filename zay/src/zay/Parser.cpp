#include "zay/Parser.h"

#include <mn/Memory.h>
#include <mn/IO.h>

namespace zay
{
	using namespace mn;

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

	inline static bool
	is_type(const Tkn& t)
	{
		return (
			t.kind == Tkn::KIND_ID ||
			t.kind == Tkn::KIND_KEYWORD_BOOL ||
			t.kind == Tkn::KIND_KEYWORD_INT ||
			t.kind == Tkn::KIND_KEYWORD_UINT ||
			t.kind == Tkn::KIND_KEYWORD_INT8 ||
			t.kind == Tkn::KIND_KEYWORD_UINT8 ||
			t.kind == Tkn::KIND_KEYWORD_INT16 ||
			t.kind == Tkn::KIND_KEYWORD_UINT16 ||
			t.kind == Tkn::KIND_KEYWORD_INT32 ||
			t.kind == Tkn::KIND_KEYWORD_UINT32 ||
			t.kind == Tkn::KIND_KEYWORD_INT64 ||
			t.kind == Tkn::KIND_KEYWORD_UINT64 ||
			t.kind == Tkn::KIND_KEYWORD_FLOAT32 ||
			t.kind == Tkn::KIND_KEYWORD_FLOAT64 ||
			t.kind == Tkn::KIND_STRING
		);
	}

	inline static Type_Sign
	parser_type(Parser self)
	{
		Type_Sign type = type_sign_new();
		while(true)
		{
			Tkn tkn = parser_look(self);
			if(is_type(tkn))
			{
				buf_push(type, type_atom_named(parser_eat(self)));
				break;
			}
			else if(tkn.kind == Tkn::KIND_STAR)
			{
				parser_eat(self); //for the *
				buf_push(type, type_atom_ptr());
			}
			else if(tkn.kind == Tkn::KIND_OPEN_BRACKET)
			{
				parser_eat_must(self, Tkn::KIND_OPEN_BRACKET);
				//this is an integer for now but really this should be an const expr
				buf_push(type, type_atom_array(parser_eat_must(self, Tkn::KIND_INTEGER)));
				parser_eat_must(self, Tkn::KIND_CLOSE_BRACKET);
			}
		}
		return type;
	}

	inline static Field
	parser_field(Parser self)
	{
		Field field = field_new();

		do
		{
			if(Tkn id = parser_eat_kind(self, Tkn::KIND_ID))
				buf_push(field.ids, id);
		}while(parser_eat_kind(self, Tkn::KIND_COMMA));

		parser_eat_must(self, Tkn::KIND_COLON);
		field.type = parser_type(self);
		return field;
	}

	inline static Decl
	parser_decl_aggregate(Parser self)
	{
		parser_eat_must(self, Tkn::KIND_KEYWORD_TYPE);
		Tkn name = parser_eat_must(self, Tkn::KIND_ID);
		Tkn type = parser_eat(self);
		if(type.kind != Tkn::KIND_KEYWORD_STRUCT && type.kind != Tkn::KIND_KEYWORD_UNION)
		{
			src_err(
				self->src,
				err_tkn(type, strf("expected a type but found '{}'", type.str))
			);
		}

		parser_eat_must(self, Tkn::KIND_OPEN_CURLY);
		Buf<Field> fields = buf_new<Field>();
		while(parser_look_kind(self, Tkn::KIND_CLOSE_CURLY) == false)
			buf_push(fields, parser_field(self));
		parser_eat_must(self, Tkn::KIND_CLOSE_CURLY);

		if(type.kind == Tkn::KIND_KEYWORD_STRUCT)
			return decl_struct(name, fields);
		else if(type.kind == Tkn::KIND_KEYWORD_UNION)
			return decl_union(name, fields);
		return nullptr;
	}

	inline static Decl
	parser_decl_enum(Parser )
	{
		//parse enum here
		return nullptr;
	}

	inline static Decl
	parser_decl_type(Parser self)
	{
		Tkn tkn = parser_look(self, 2);
		if(tkn.kind == Tkn::KIND_KEYWORD_STRUCT || Tkn::KIND_KEYWORD_UNION)
		{
			return parser_decl_aggregate(self);
		}
		else if(tkn.kind == Tkn::KIND_KEYWORD_ENUM)
		{
			return parser_decl_enum(self);
		}
		else
		{
			//typedef here
			return nullptr;
		}
	}

	inline static Decl
	parser_decl_var(Parser )
	{
		//parser variable here
		return nullptr;
	}

	inline static Decl
	parser_decl_func(Parser )
	{
		//parser function here
		return nullptr;
	}


	inline static bool
	is_cmp_op(const Tkn& t)
	{
		return (
			t.kind == Tkn::KIND_LESS ||
			t.kind == Tkn::KIND_GREATER ||
			t.kind == Tkn::KIND_LESS_EQUAL ||
			t.kind == Tkn::KIND_GREATER_EQUAL ||
			t.kind == Tkn::KIND_EQUAL_EQUAL ||
			t.kind == Tkn::KIND_NOT_EQUAL
		);
	}

	inline static bool
	is_add_op(const Tkn& t)
	{
		return (
			t.kind == Tkn::KIND_PLUS ||
			t.kind == Tkn::KIND_MINUS ||
			t.kind == Tkn::KIND_BIT_XOR ||
			t.kind == Tkn::KIND_BIT_OR
		);
	}

	inline static bool
	is_mul_op(const Tkn& t)
	{
		return (
			t.kind == Tkn::KIND_STAR ||
			t.kind == Tkn::KIND_DIV ||
			t.kind == Tkn::KIND_MOD ||
			t.kind == Tkn::KIND_BIT_AND ||
			t.kind == Tkn::KIND_LEFT_SHIFT ||
			t.kind == Tkn::KIND_RIGHT_SHIFT
		);
	}

	inline static bool
	is_unary_op(const Tkn& t)
	{
		return (
			t.kind == Tkn::KIND_INC ||
			t.kind == Tkn::KIND_DEC ||
			t.kind == Tkn::KIND_PLUS ||
			t.kind == Tkn::KIND_MINUS ||
			t.kind == Tkn::KIND_BIT_AND ||
			t.kind == Tkn::KIND_STAR ||
			t.kind == Tkn::KIND_LOGIC_NOT
		);
	}

	inline static Expr
	parser_expr_atom(Parser self)
	{
		Tkn tkn = parser_look(self);
		if(tkn.kind == Tkn::KIND_INTEGER)
		{
			return expr_atom(parser_eat(self));
		}
		else if(tkn.kind == Tkn::KIND_FLOAT)
		{
			return expr_atom(parser_eat(self));
		}
		else if(tkn.kind == Tkn::KIND_STRING)
		{
			return expr_atom(parser_eat(self));
		}
		else if(tkn.kind == Tkn::KIND_ID)
		{
			return expr_atom(parser_eat(self));
		}
		else if(tkn.kind == Tkn::KIND_KEYWORD_FALSE)
		{
			return expr_atom(parser_eat(self));
		}
		else if(tkn.kind == Tkn::KIND_KEYWORD_TRUE)
		{
			return expr_atom(parser_eat(self));
		}
		else if(tkn.kind == Tkn::KIND_OPEN_PAREN)
		{
			parser_eat(self); // for the (
			Expr expr = parser_expr(self);
			parser_eat_must(self, Tkn::KIND_CLOSE_PAREN);
			return expr_paren(expr);
		}
		//error here
		assert(false && "unreachable");
		return nullptr;
	}

	inline static Expr
	parser_expr_base(Parser self)
	{
		Expr expr = parser_expr_atom(self);
		while(true)
		{
			Tkn tkn = parser_look(self);
			if(tkn.kind == Tkn::KIND_OPEN_PAREN)
			{
				parser_eat(self); //for the (
				Buf<Expr> args = buf_new<Expr>();
				do
				{
					if(Expr arg = parser_expr(self))
						buf_push(args, arg);
				}while(parser_eat_kind(self, Tkn::KIND_COMMA));
				parser_eat_must(self, Tkn::KIND_CLOSE_PAREN);
				expr = expr_call(expr, args);
			}
			else if(tkn.kind == Tkn::KIND_OPEN_BRACKET)
			{
				parser_eat(self); //for the [
				Expr index = parser_expr(self);
				parser_eat_must(self, Tkn::KIND_CLOSE_BRACKET);
				expr = expr_indexed(expr, index);
			}
			else if(tkn.kind == Tkn::KIND_DOT)
			{
				parser_eat(self); //for the .
				expr = expr_dot(expr, parser_eat_must(self, Tkn::KIND_ID));
			}
			else
			{
				break;
			}
		}
		return expr;
	}

	inline static Expr
	parser_expr_unary(Parser self)
	{
		if(is_unary_op(parser_look(self)))
		{
			Tkn op = parser_eat(self);
			return expr_unary(op, parser_expr_unary(self));
		}
		return parser_expr_base(self);
	}

	inline static Expr
	parser_expr_cast(Parser self)
	{
		Expr expr = parser_expr_unary(self);
		if(parser_eat_kind(self, Tkn::KIND_COLON))
			return expr_cast(expr, parser_type(self));
		return expr;
	}

	inline static Expr
	parser_expr_mul(Parser self)
	{
		Expr expr = parser_expr_cast(self);
		while(is_mul_op(parser_look(self)))
		{
			Tkn op = parser_eat(self);
			expr = expr_binary(expr, op, parser_expr_cast(self));
		}
		return expr;
	}

	inline static Expr
	parser_expr_add(Parser self)
	{
		Expr expr = parser_expr_mul(self);
		while(is_add_op(parser_look(self)))
		{
			Tkn op = parser_eat(self);
			expr = expr_binary(expr, op, parser_expr_mul(self));
		}
		return expr;
	}

	inline static Expr
	parser_expr_cmp(Parser self)
	{
		Expr expr = parser_expr_add(self);
		if(is_cmp_op(parser_look(self)))
		{
			Tkn op = parser_eat(self);
			expr = expr_binary(expr, op, parser_expr_add(self));
		}
		return expr;
	}

	inline static Expr
	parser_expr_and(Parser self)
	{
		Expr expr = parser_expr_cmp(self);
		while(Tkn tkn = parser_eat_kind(self, Tkn::KIND_LOGIC_AND))
			expr = expr_binary(expr, tkn, parser_expr_cmp(self));
		return expr;
	}

	inline static Expr
	parser_expr_or(Parser self)
	{
		Expr expr = parser_expr_and(self);
		while(Tkn tkn = parser_eat_kind(self, Tkn::KIND_LOGIC_OR))
			expr = expr_binary(expr, tkn, parser_expr_and(self));
		return expr;
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
		buf_remove_if(self->tkns, [](const Tkn& t){
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

	Expr
	parser_expr(Parser self)
	{
		return parser_expr_or(self);
	}

	Decl
	parser_decl(Parser self)
	{
		Tkn tkn = parser_look(self);
		if(tkn.kind == Tkn::KIND_KEYWORD_TYPE)
		{
			return parser_decl_type(self);
		}
		else if(tkn.kind == Tkn::KIND_KEYWORD_VAR)
		{
			return parser_decl_var(self);
		}
		else if(tkn.kind == Tkn::KIND_KEYWORD_FUNC)
		{
			return parser_decl_func(self);
		}
		return nullptr;
	}
};
