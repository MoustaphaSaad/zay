#pragma once

#include "zay/Exports.h"
#include "zay/Tkn.h"

#include <mn/Buf.h>

namespace zay
{
	typedef struct IType* Type;

	//Types
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


	//Expressions
	typedef struct IExpr* Expr;

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
		Rng rng;
		Pos pos;
		union
		{
			Tkn atom;

			struct
			{
				Expr lhs;
				Tkn op;
				Expr rhs;
			} binary;

			struct
			{
				Tkn op;
				Expr expr;
			} unary;

			struct
			{
				Expr base;
				Tkn member;
			} dot;

			struct
			{
				Expr base;
				Expr index;
			} indexed;

			struct
			{
				Expr base;
				mn::Buf<Expr> args;
			} call;

			struct
			{
				Expr base;
				Type_Sign type;
				Type to_type;
			} cast;

			Expr paren;
		};
	};

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


	//Statements
	typedef struct IStmt* Stmt;

	struct Variable
	{
		mn::Buf<Tkn> ids;
		Type_Sign type;
		mn::Buf<Expr> exprs;
	};

	inline static Variable
	variable_new()
	{
		return Variable{
			mn::buf_new<Tkn>(),
			type_sign_new(),
			mn::buf_new<Expr>()
		};
	}

	inline static void
	variable_free(Variable& self)
	{
		mn::buf_free(self.ids);
		type_sign_free(self.type);
		destruct(self.exprs);
	}

	inline static void
	destruct(Variable& self)
	{
		variable_free(self);
	}

	struct Else_If
	{
		Expr cond;
		Stmt body;
	};

	struct IStmt
	{
		enum KIND
		{
			KIND_NONE,
			KIND_BREAK,
			KIND_CONTINUE,
			KIND_RETURN,
			KIND_IF,
			KIND_FOR,
			KIND_VAR,
			KIND_ASSIGN,
			KIND_EXPR,
			KIND_BLOCK
		};

		KIND kind;
		Rng rng;
		Pos pos;
		union
		{
			Tkn break_stmt;

			Tkn continue_stmt;

			Expr return_stmt;

			struct
			{
				Expr if_cond;
				Stmt if_body;
				mn::Buf<Else_If> else_ifs;
				Stmt else_body;
			} if_stmt;

			struct
			{
				Stmt init_stmt;
				Expr loop_cond;
				Stmt post_stmt;
				Stmt loop_body;
			} for_stmt;

			Variable var_stmt;

			struct
			{
				mn::Buf<Expr> lhs;
				Tkn op;
				mn::Buf<Expr> rhs;
			} assign_stmt;

			Expr expr_stmt;

			mn::Buf<Stmt> block_stmt;
		};
	};

	ZAY_EXPORT Stmt
	stmt_break(const Tkn& t);

	ZAY_EXPORT Stmt
	stmt_continue(const Tkn& t);

	ZAY_EXPORT Stmt
	stmt_return(Expr e);

	ZAY_EXPORT Stmt
	stmt_if(Expr if_cond, Stmt if_body, const mn::Buf<Else_If>& else_ifs, Stmt else_body);

	ZAY_EXPORT Stmt
	stmt_for(Stmt init_stmt, Expr loop_cond, Stmt post_stmt, Stmt loop_body);

	ZAY_EXPORT Stmt
	stmt_var(Variable v);

	ZAY_EXPORT Stmt
	stmt_block(const mn::Buf<Stmt>& stmts);

	ZAY_EXPORT Stmt
	stmt_assign(const mn::Buf<Expr>& lhs, const Tkn& op, const mn::Buf<Expr>& rhs);

	ZAY_EXPORT Stmt
	stmt_expr(Expr e);

	ZAY_EXPORT Expr
	stmt_expr_decay(Stmt expr);

	ZAY_EXPORT void
	stmt_free(Stmt self);

	inline static void
	destruct(Stmt self)
	{
		stmt_free(self);
	}


	//Declarations
	typedef struct IDecl* Decl;

	//Struct, Union Fields
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

	//Enum Field
	struct Enum_Field
	{
		Tkn id;
		Expr expr;
	};

	//Function Arguments
	struct Arg
	{
		mn::Buf<Tkn> ids;
		Type_Sign type;
	};

	inline static Arg
	arg_new()
	{
		return Arg{
			mn::buf_new<Tkn>(),
			type_sign_new()
		};
	}

	inline static void
	arg_free(Arg& self)
	{
		mn::buf_free(self.ids);
		type_sign_free(self.type);
	}

	inline static void
	destruct(Arg& self)
	{
		arg_free(self);
	}

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
		Rng rng;
		Pos pos;
		union
		{
			mn::Buf<Field> struct_decl;

			//let's not care about unions right now
			mn::Buf<Field> union_decl;

			mn::Buf<Enum_Field> enum_decl;

			Variable var_decl;

			struct
			{
				mn::Buf<Arg> args;
				Type_Sign ret_type;
				Stmt body;
			} func_decl;
		};
	};

	ZAY_EXPORT Decl
	decl_struct(const Tkn& name, const mn::Buf<Field>& fields);

	ZAY_EXPORT Decl
	decl_union(const Tkn& name, const mn::Buf<Field>& fields);

	ZAY_EXPORT Decl
	decl_enum(const Tkn& name, const mn::Buf<Enum_Field>& fields);

	ZAY_EXPORT Decl
	decl_var(const Variable& v);

	ZAY_EXPORT Decl
	decl_func(const Tkn& name, const mn::Buf<Arg>& args, const Type_Sign& ret_type, Stmt body);

	ZAY_EXPORT void
	decl_free(Decl self);

	inline static void
	destruct(Decl self)
	{
		decl_free(self);
	}


	//Abstract Syntax Tree
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