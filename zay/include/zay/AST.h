#pragma once

#include "zay/Exports.h"
#include "zay/Tkn.h"

#include <mn/Buf.h>

namespace zay
{
	struct Type_Atom
	{
		enum KIND
		{
			KIND_NONE,
			KIND_NAMED,
			KIND_PTR,
			KIND_ARRAY
		};

		KIND kind;
		union
		{
			Tkn named;
			Tkn count;
		};
	};

	inline static Type_Atom
	type_atom_named(const Tkn& t)
	{
		Type_Atom self{};
		self.kind = Type_Atom::KIND_NAMED;
		self.named = t;
		return self;
	}

	inline static Type_Atom
	type_atom_ptr()
	{
		Type_Atom self{};
		self.kind = Type_Atom::KIND_PTR;
		return self;
	}

	inline static Type_Atom
	type_atom_array(const Tkn& t)
	{
		Type_Atom self{};
		self.kind = Type_Atom::KIND_ARRAY;
		self.count = t;
		return self;
	}


	typedef mn::Buf<Type_Atom> Type_Sign;

	inline static Type_Sign
	type_sign_new()
	{
		return mn::buf_new<Type_Atom>();
	}

	inline static void
	type_sign_free(Type_Sign& self)
	{
		mn::buf_free(self);
	}


	struct IExpr
	{
		enum KIND
		{
			KIND_NONE,
			KIND_ATOM,
			KIND_BINARY,
			KIND_UNARY,
			KIND_DOT,
			KIND_INDEXED,
			KIND_CALL,
			KIND_CAST,
			KIND_PAREN
		};

		KIND kind;
		union
		{
			Tkn atom;

			struct
			{
				IExpr* lhs;
				Tkn op;
				IExpr* rhs;
			} binary;

			struct
			{
				Tkn op;
				IExpr* expr;
			} unary;

			struct
			{
				IExpr* base;
				Tkn member;
			} dot;

			struct
			{
				IExpr* base;
				IExpr* index;
			} indexed;

			struct
			{
				IExpr* base;
				mn::Buf<IExpr*> args;
			} call;

			struct
			{
				IExpr* base;
				Type_Sign type;
			} cast;

			IExpr* paren;
		};
	};
	typedef IExpr* Expr;

	ZAY_EXPORT Expr
	expr_atom(const Tkn& t);

	ZAY_EXPORT Expr
	expr_paren(Expr e);

	ZAY_EXPORT Expr
	expr_call(Expr base, const mn::Buf<Expr>& args);

	ZAY_EXPORT Expr
	expr_indexed(Expr base, Expr index);

	ZAY_EXPORT Expr
	expr_dot(Expr base, const Tkn& t);

	ZAY_EXPORT Expr
	expr_unary(const Tkn& op, Expr expr);

	ZAY_EXPORT Expr
	expr_cast(Expr expr, const Type_Sign& type);

	ZAY_EXPORT Expr
	expr_binary(Expr lhs, const Tkn& op, Expr rhs);

	ZAY_EXPORT void
	expr_free(Expr self);

	inline static void
	destruct(Expr self)
	{
		expr_free(self);
	}


	struct Field
	{
		mn::Buf<Tkn> ids;
		Type_Sign type;
	};

	inline static Field
	field_new()
	{
		Field self{};
		self.ids = mn::buf_new<Tkn>();
		self.type = type_sign_new();
		return self;
	}

	inline static void
	field_free(Field& self)
	{
		mn::buf_free(self.ids);
		type_sign_free(self.type);
	}

	inline static void
	destruct(Field& self)
	{
		field_free(self);
	}

	struct Arg
	{
		mn::Buf<Tkn> ids;
		Type_Sign type;
	};


	struct IDecl
	{
		enum KIND
		{
			KIND_NONE,
			KIND_STRUCT,
			KIND_UNION,
			KIND_ENUM,
			KIND_VAR,
			KIND_FUNC
		};

		KIND kind;
		//all declarations have names
		Tkn name;
		union
		{
			mn::Buf<Field> struct_decl;

			mn::Buf<Field> union_decl;

			mn::Buf<Tkn> enum_decl;

			struct
			{
				mn::Buf<Tkn> ids;
				Type_Sign type;
			} var_decl;

			struct
			{
				mn::Buf<Arg> args;
				Type_Sign type;
			} func_decl;
		};
	};
	typedef IDecl* Decl;

	ZAY_EXPORT Decl
	decl_struct(const Tkn& name, const mn::Buf<Field>& fields);

	ZAY_EXPORT Decl
	decl_union(const Tkn& name, const mn::Buf<Field>& fields);

	ZAY_EXPORT void
	decl_free(Decl self);

	inline static void
	destruct(Decl self)
	{
		decl_free(self);
	}


	struct IAST
	{
		mn::Buf<Decl> decls;
	};
	typedef IAST* AST;

	ZAY_EXPORT AST
	ast_new();

	ZAY_EXPORT void
	ast_free(AST self);

	inline static void
	destruct(AST self);
}