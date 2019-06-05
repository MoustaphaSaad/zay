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
}