#include "zay/Parser.h"

#include <mn/Memory.h>
#include <mn/IO.h>

namespace zay
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
	parser_decl_enum(Parser self)
	{
		parser_eat_must(self, Tkn::KIND_KEYWORD_TYPE);
		Tkn name = parser_eat_must(self, Tkn::KIND_ID);
		parser_eat_must(self, Tkn::KIND_KEYWORD_ENUM);

		Buf<Enum_Field> enum_fields = buf_new<Enum_Field>();
		parser_eat_must(self, Tkn::KIND_OPEN_CURLY);
		do
		{
			if(Tkn id = parser_eat_kind(self, Tkn::KIND_ID))
			{
				Expr expr = nullptr;
				if(parser_eat_kind(self, Tkn::KIND_EQUAL))
					expr = parser_expr(self);
				buf_push(enum_fields, Enum_Field{id, expr});
			}
		}while(parser_eat_kind(self, Tkn::KIND_COMMA));
		parser_eat_must(self, Tkn::KIND_CLOSE_CURLY);

		return decl_enum(name, enum_fields);
	}

	inline static Decl
	parser_decl_type(Parser self)
	{
		//only working with struct, and enums
		Tkn tkn = parser_look(self, 2);
		if(tkn.kind == Tkn::KIND_KEYWORD_STRUCT || tkn.kind == Tkn::KIND_KEYWORD_UNION)
		{
			return parser_decl_aggregate(self);
		}
		else if(tkn.kind == Tkn::KIND_KEYWORD_ENUM)
		{
			return parser_decl_enum(self);
		}
		else
		{
			//let's ignore typedefs for now
			return nullptr;
		}
	}

	inline static Variable
	parser_variable(Parser self)
	{
		parser_eat_must(self, Tkn::KIND_KEYWORD_VAR);
		Variable v = variable_new();

		do
		{
			if(Tkn id = parser_eat_must(self, Tkn::KIND_ID))
				buf_push(v.ids, id);
		}while(parser_eat_kind(self, Tkn::KIND_COMMA));

		if(parser_eat_kind(self, Tkn::KIND_COLON))
			v.type = parser_type(self);

		if(parser_eat_kind(self, Tkn::KIND_EQUAL))
		{
			do
			{
				if(Expr e = parser_expr(self))
					buf_push(v.exprs, e);
			}while(parser_eat_kind(self, Tkn::KIND_COMMA));
		}

		return v;
	}

	inline static Decl
	parser_decl_var(Parser self)
	{
		return decl_var(parser_variable(self));
	}

	inline static Arg
	parser_arg(Parser self)
	{
		Arg arg = arg_new();
		do
		{
			if(Tkn id = parser_eat_kind(self, Tkn::KIND_ID))
				buf_push(arg.ids, id);
		}while(parser_eat_kind(self, Tkn::KIND_COMMA));

		//function arguments must have a type
		parser_eat_must(self, Tkn::KIND_COLON);
		arg.type = parser_type(self);
		return arg;
	}

	inline static Stmt
	parser_stmt_block(Parser self);

	inline static Decl
	parser_decl_func(Parser self)
	{
		parser_eat_must(self, Tkn::KIND_KEYWORD_FUNC);
		Tkn name = parser_eat_must(self, Tkn::KIND_ID);

		Buf<Arg> args = buf_new<Arg>();
		parser_eat_must(self, Tkn::KIND_OPEN_PAREN);
		do
		{
			//we didn't reach the end of the argument list
			if(parser_look_kind(self, Tkn::KIND_CLOSE_PAREN) == false)
			{
				buf_push(args, parser_arg(self));
			}
		}while(parser_eat_kind(self, Tkn::KIND_COMMA));
		parser_eat_must(self, Tkn::KIND_CLOSE_PAREN);

		//wait the return type ...
		Type_Sign ret_type = type_sign_new();
		if(parser_eat_kind(self, Tkn::KIND_COLON))
			ret_type = parser_type(self);

		//now that we have argument list we need to parse the body
		Stmt body = parser_stmt_block(self);
		return decl_func(name, args, ret_type, body);
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
		Expr expr = nullptr;
		if(tkn.kind == Tkn::KIND_INTEGER)
		{
			expr = expr_atom(parser_eat(self));
		}
		else if(tkn.kind == Tkn::KIND_FLOAT)
		{
			expr = expr_atom(parser_eat(self));
		}
		else if(tkn.kind == Tkn::KIND_STRING)
		{
			expr = expr_atom(parser_eat(self));
		}
		else if(tkn.kind == Tkn::KIND_ID)
		{
			expr = expr_atom(parser_eat(self));
		}
		else if(tkn.kind == Tkn::KIND_KEYWORD_FALSE)
		{
			expr = expr_atom(parser_eat(self));
		}
		else if(tkn.kind == Tkn::KIND_KEYWORD_TRUE)
		{
			expr = expr_atom(parser_eat(self));
		}
		else if(tkn.kind == Tkn::KIND_OPEN_PAREN)
		{
			parser_eat(self); // for the (
			expr = parser_expr(self);
			parser_eat_must(self, Tkn::KIND_CLOSE_PAREN);
			expr = expr_paren(expr);
		}

		if(expr)
		{
			expr->rng = Rng{ tkn.rng.begin, parser_last_tkn(self).rng.end };
			expr->pos = tkn.pos;
		}
		else
		{
			//error here
			assert(false && "unreachable");
		}
		return expr;
	}

	inline static Expr
	parser_expr_base(Parser self)
	{
		Tkn t = parser_look(self);
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

		if(expr)
		{
			expr->rng = Rng{ t.rng.begin, parser_last_tkn(self).rng.end };
			expr->pos = t.pos;
		}

		return expr;
	}

	inline static Expr
	parser_expr_unary(Parser self)
	{
		Tkn t = parser_look(self);
		Expr expr = nullptr;
		if(is_unary_op(parser_look(self)))
		{
			Tkn op = parser_eat(self);
			expr = expr_unary(op, parser_expr_unary(self));
		}
		else
		{
			expr = parser_expr_base(self);
		}

		if(expr)
		{
			expr->rng = Rng{ t.rng.begin, parser_last_tkn(self).rng.end };
			expr->pos = t.pos;
		}

		return expr;
	}

	inline static Expr
	parser_expr_cast(Parser self)
	{
		Tkn t = parser_look(self);
		Expr expr = parser_expr_unary(self);
		if(parser_eat_kind(self, Tkn::KIND_COLON))
			expr = expr_cast(expr, parser_type(self));

		if(expr)
		{
			expr->rng = Rng{ t.rng.begin, parser_last_tkn(self).rng.end };
			expr->pos = t.pos;
		}

		return expr;
	}

	inline static Expr
	parser_expr_mul(Parser self)
	{
		Tkn t = parser_look(self);

		Expr expr = parser_expr_cast(self);
		while(is_mul_op(parser_look(self)))
		{
			Tkn op = parser_eat(self);
			expr = expr_binary(expr, op, parser_expr_cast(self));
		}

		if(expr)
		{
			expr->rng = Rng{ t.rng.begin, parser_last_tkn(self).rng.end };
			expr->pos = t.pos;
		}

		return expr;
	}

	inline static Expr
	parser_expr_add(Parser self)
	{
		Tkn t = parser_look(self);
		Expr expr = parser_expr_mul(self);
		while(is_add_op(parser_look(self)))
		{
			Tkn op = parser_eat(self);
			expr = expr_binary(expr, op, parser_expr_mul(self));
		}

		if(expr)
		{
			expr->rng = Rng{ t.rng.begin, parser_last_tkn(self).rng.end };
			expr->pos = t.pos;
		}

		return expr;
	}

	inline static Expr
	parser_expr_cmp(Parser self)
	{
		Tkn t = parser_look(self);
		Expr expr = parser_expr_add(self);
		if(is_cmp_op(parser_look(self)))
		{
			Tkn op = parser_eat(self);
			expr = expr_binary(expr, op, parser_expr_add(self));
		}

		if(expr)
		{
			expr->rng = Rng{ t.rng.begin, parser_last_tkn(self).rng.end };
			expr->pos = t.pos;
		}

		return expr;
	}

	inline static Expr
	parser_expr_and(Parser self)
	{
		Tkn t = parser_look(self);
		Expr expr = parser_expr_cmp(self);
		while(Tkn tkn = parser_eat_kind(self, Tkn::KIND_LOGIC_AND))
			expr = expr_binary(expr, tkn, parser_expr_cmp(self));

		if(expr)
		{
			expr->rng = Rng{ t.rng.begin, parser_last_tkn(self).rng.end };
			expr->pos = t.pos;
		}

		return expr;
	}

	inline static Expr
	parser_expr_or(Parser self)
	{
		Tkn t = parser_look(self);
		Expr expr = parser_expr_and(self);
		while(Tkn tkn = parser_eat_kind(self, Tkn::KIND_LOGIC_OR))
			expr = expr_binary(expr, tkn, parser_expr_and(self));

		if(expr)
		{
			expr->rng = Rng{ t.rng.begin, parser_last_tkn(self).rng.end };
			expr->pos = t.pos;
		}

		return expr;
	}


	inline static bool
	is_assign_op(const Tkn& t)
	{
		return (
			t.kind == Tkn::KIND_EQUAL ||
			t.kind == Tkn::KIND_PLUS_EQUAL ||
			t.kind == Tkn::KIND_MINUS_EQUAL ||
			t.kind == Tkn::KIND_STAR_EQUAL ||
			t.kind == Tkn::KIND_DIV_EQUAL ||
			t.kind == Tkn::KIND_MOD_EQUAL ||
			t.kind == Tkn::KIND_BIT_OR_EQUAL ||
			t.kind == Tkn::KIND_BIT_AND_EQUAL ||
			t.kind == Tkn::KIND_BIT_XOR_EQUAL ||
			t.kind == Tkn::KIND_LEFT_SHIFT_EQUAL ||
			t.kind == Tkn::KIND_RIGHT_SHIFT_EQUAL
		);
	}

	inline static Stmt
	parser_stmt_block(Parser self)
	{
		parser_eat_must(self, Tkn::KIND_OPEN_CURLY);

		Buf<Stmt> stmts = buf_new<Stmt>();
		while(parser_look_kind(self, Tkn::KIND_CLOSE_CURLY) == false)
			buf_push(stmts, parser_stmt(self));

		parser_eat_must(self, Tkn::KIND_CLOSE_CURLY);
		return stmt_block(stmts);
	}

	inline static Stmt
	parser_stmt_if(Parser self)
	{
		parser_eat_must(self, Tkn::KIND_KEYWORD_IF);
		Expr if_cond = parser_expr(self);
		Stmt if_body = parser_stmt_block(self);
		Stmt else_body = nullptr;
		Buf<Else_If> else_ifs = buf_new<Else_If>();
		while(parser_eat_kind(self, Tkn::KIND_KEYWORD_ELSE))
		{
			if(parser_eat_kind(self, Tkn::KIND_KEYWORD_IF))
			{
				Expr cond = parser_expr(self);
				Stmt body = parser_stmt_block(self);
				buf_push(else_ifs, Else_If{cond, body});
			}
			else
			{
				else_body = parser_stmt_block(self);
				break;
			}
		}

		return stmt_if(if_cond, if_body, else_ifs, else_body);
	}

	inline static Stmt
	parser_stmt_for(Parser self)
	{
		parser_eat_must(self, Tkn::KIND_KEYWORD_FOR);

		Stmt init_stmt = nullptr;
		Expr loop_cond = nullptr;
		Stmt post_stmt = nullptr;
		Stmt loop_body = nullptr;

		//for {}
		if(parser_look_kind(self, Tkn::KIND_OPEN_CURLY))
		{
			loop_body = parser_stmt_block(self);
		}
		//for ;cond;post {}
		else if(parser_eat_kind(self, Tkn::KIND_SEMICOLON))
		{
			loop_cond = parser_expr(self);
			parser_eat_must(self, Tkn::KIND_SEMICOLON);
			if(parser_look_kind(self, Tkn::KIND_OPEN_CURLY) == false)
				post_stmt = parser_stmt(self);
			loop_body = parser_stmt_block(self);
		}
		//for cond {}
		//for init_stmt;loop_cond;post_stmt{}
		else
		{
			init_stmt = parser_stmt(self);
			if(parser_eat_kind(self, Tkn::KIND_SEMICOLON))
			{
				loop_cond = parser_expr(self);
				parser_eat_must(self, Tkn::KIND_SEMICOLON);
				if(parser_look_kind(self, Tkn::KIND_OPEN_CURLY) == false)
					post_stmt = parser_stmt(self);
				loop_body = parser_stmt_block(self);
			}
			else if(parser_look_kind(self, Tkn::KIND_OPEN_CURLY))
			{
				loop_cond = stmt_expr_decay(init_stmt);
				init_stmt = nullptr;
				loop_body = parser_stmt_block(self);
			}
		}

		return stmt_for(init_stmt, loop_cond, post_stmt, loop_body);
	}

	inline static Stmt
	parser_stmt_var(Parser self)
	{
		return stmt_var(parser_variable(self));
	}

	inline static Stmt
	parser_stmt_simple(Parser self)
	{
		//simple stmt could be assignment or expression
		//assignment
		Buf<Expr> lhs = buf_new<Expr>();
		do
		{
			if(Expr e = parser_expr(self))
				buf_push(lhs, e);
		}while(parser_eat_kind(self, Tkn::KIND_COMMA));

		if(is_assign_op(parser_look(self)))
		{
			Tkn op = parser_eat(self);
			Buf<Expr> rhs = buf_new<Expr>();
			do
			{
				if(Expr e = parser_expr(self))
					buf_push(rhs, e);
			}while(parser_eat_kind(self, Tkn::KIND_COMMA));
			return stmt_assign(lhs, op, rhs);
		}

		//this is not an assign stmt so it must be an expression stmt
		if(lhs.count > 1)
		{
			src_err(
				self->src,
				err_str(strf("can't have multiple expression in the same statements"))
			);
		}

		Stmt s = stmt_expr(lhs[0]);
		buf_free(lhs);
		return s;
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

	Stmt
	parser_stmt(Parser self)
	{
		Tkn tkn = parser_look(self);
		Stmt res = nullptr;

		if(tkn.kind == Tkn::KIND_KEYWORD_BREAK)
		{
			res = stmt_break(parser_eat(self));
		}
		else if(tkn.kind == Tkn::KIND_KEYWORD_CONTINUE)
		{
			res = stmt_continue(parser_eat(self));
		}
		else if(tkn.kind == Tkn::KIND_KEYWORD_RETURN)
		{
			parser_eat(self); //for the return
			res = stmt_return(parser_expr(self));
		}
		else if(tkn.kind == Tkn::KIND_OPEN_CURLY)
		{
			res = parser_stmt_block(self);
		}
		else if(tkn.kind == Tkn::KIND_KEYWORD_IF)
		{
			res = parser_stmt_if(self);
		}
		else if(tkn.kind == Tkn::KIND_KEYWORD_FOR)
		{
			res = parser_stmt_for(self);
		}
		else if(tkn.kind == Tkn::KIND_KEYWORD_VAR)
		{
			res = parser_stmt_var(self);
		}
		else
		{
			res = parser_stmt_simple(self);
		}

		if(res)
		{
			res->rng = Rng{ tkn.rng.begin, parser_last_tkn(self).rng.end };
			res->pos = tkn.pos;
		}

		return res;
	}

	Decl
	parser_decl(Parser self)
	{
		Tkn tkn = parser_look(self);
		Decl res = nullptr;

		if(tkn.kind == Tkn::KIND_KEYWORD_TYPE)
		{
			res = parser_decl_type(self);
		}
		else if(tkn.kind == Tkn::KIND_KEYWORD_VAR)
		{
			res = parser_decl_var(self);
		}
		else if(tkn.kind == Tkn::KIND_KEYWORD_FUNC)
		{
			res = parser_decl_func(self);
		}

		if(res)
		{
			res->rng = Rng{ tkn.rng.begin, parser_last_tkn(self).rng.end };
			res->pos = tkn.pos;
		}
		return res;
	}
};
