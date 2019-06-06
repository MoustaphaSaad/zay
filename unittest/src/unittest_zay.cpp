#include <doctest/doctest.h>

#include <zay/Src.h>
#include <zay/Scanner.h>
#include <zay/Parser.h>
#include <zay/AST_Lisp.h>

using namespace mn;
using namespace zay;

inline static Str
scan(const char* str)
{
	Src src = src_from_str(str);
	CHECK(src_scan(src));
	Str res = src_tkns_dump(src, memory::tmp());
	src_free(src);
	return res;
}

inline static Str
parse(const char* str)
{
	Src src = src_from_str(str);
	CHECK(src_scan(src));
	CHECK(src_parse(src));
	Str res = src_ast_dump(src, memory::tmp());
	src_free(src);
	return res;
}

inline static Str
parse_expr(const char* str)
{
	Src src = src_from_str(str);
	CHECK(src_scan(src));

	Parser parser = parser_new(src);
	Expr e = parser_expr(parser);
	parser_free(parser);

	Stream out = stream_tmp();
	AST_Lisp writer = ast_lisp_new(out);
	ast_lisp_expr(writer, e);
	Str res = str_from_c(stream_str(out), memory::tmp());

	expr_free(e);
	src_free(src);
	return res;
}

TEST_CASE("[zay]: scan basic comment")
{
	const char* code = R"CODE(//type foo struct{}
var x: int = 234;
)CODE";

	const char* expected = R"EXPECTED(line: 1, col: 0, kind: "<COMMENT>" str: "type foo struct{"
line: 2, col: 1, kind: "var" str: "var"
line: 2, col: 5, kind: "<ID>" str: "x"
line: 2, col: 6, kind: ":" str: ":"
line: 2, col: 8, kind: "int" str: "int"
line: 2, col: 12, kind: "=" str: "="
line: 2, col: 14, kind: "<INTEGER>" str: "234"
line: 2, col: 17, kind: ";" str: ";"
)EXPECTED";

	Str answer = scan(code);
	CHECK(answer == expected);
}

TEST_CASE("[zay]: basic scan")
{
	const char* code = R"CODE(type foo struct{
	x, y: int;
	z, w: float32;
}

var x: int = 234;

func add(x, y: int): int {
	return x + y;
}
)CODE";

	const char* expected = R"EXPECTED(line: 1, col: 0, kind: "type" str: "type"
line: 1, col: 5, kind: "<ID>" str: "foo"
line: 1, col: 9, kind: "struct" str: "struct"
line: 1, col: 15, kind: "{" str: "{"
line: 2, col: 2, kind: "<ID>" str: "x"
line: 2, col: 3, kind: "," str: ","
line: 2, col: 5, kind: "<ID>" str: "y"
line: 2, col: 6, kind: ":" str: ":"
line: 2, col: 8, kind: "int" str: "int"
line: 2, col: 11, kind: ";" str: ";"
line: 3, col: 2, kind: "<ID>" str: "z"
line: 3, col: 3, kind: "," str: ","
line: 3, col: 5, kind: "<ID>" str: "w"
line: 3, col: 6, kind: ":" str: ":"
line: 3, col: 8, kind: "float32" str: "float32"
line: 3, col: 15, kind: ";" str: ";"
line: 4, col: 1, kind: "}" str: "}"
line: 6, col: 1, kind: "var" str: "var"
line: 6, col: 5, kind: "<ID>" str: "x"
line: 6, col: 6, kind: ":" str: ":"
line: 6, col: 8, kind: "int" str: "int"
line: 6, col: 12, kind: "=" str: "="
line: 6, col: 14, kind: "<INTEGER>" str: "234"
line: 6, col: 17, kind: ";" str: ";"
line: 8, col: 1, kind: "func" str: "func"
line: 8, col: 6, kind: "<ID>" str: "add"
line: 8, col: 9, kind: "(" str: "("
line: 8, col: 10, kind: "<ID>" str: "x"
line: 8, col: 11, kind: "," str: ","
line: 8, col: 13, kind: "<ID>" str: "y"
line: 8, col: 14, kind: ":" str: ":"
line: 8, col: 16, kind: "int" str: "int"
line: 8, col: 19, kind: ")" str: ")"
line: 8, col: 20, kind: ":" str: ":"
line: 8, col: 22, kind: "int" str: "int"
line: 8, col: 26, kind: "{" str: "{"
line: 9, col: 2, kind: "return" str: "return"
line: 9, col: 9, kind: "<ID>" str: "x"
line: 9, col: 11, kind: "+" str: "+"
line: 9, col: 13, kind: "<ID>" str: "y"
line: 9, col: 14, kind: ";" str: ";"
line: 10, col: 1, kind: "}" str: "}"
)EXPECTED";

	Str answer = scan(code);
	CHECK(answer == expected);
}

TEST_CASE("[zay]: parse basic struct")
{
	const char* code = R"CODE(type foo struct{
	x, y: int
	z, w: float32
}
)CODE";

	const char* expected = R"EXPECTED((struct foo
	(field x, y: (type  int))
	(field z, w: (type  float32))
)
)EXPECTED";

	Str answer = parse(code);
	CHECK(answer == expected);
}

TEST_CASE("[zay]: parse basic union")
{
	const char* code = R"CODE(type foo union{
	f64: float64
	f32: [2]float32
	u8: [8]uint8
	ptr: [8]*uint8
}
)CODE";

	const char* expected = R"EXPECTED((union foo
	(field f64: (type  float64))
	(field f32: (type [2] float32))
	(field u8: (type [8] uint8))
	(field ptr: (type [8]* uint8))
)
)EXPECTED";

	Str answer = parse(code);
	CHECK(answer == expected);
}

TEST_CASE("[zay]: parse basic expression")
{
	const char* code = "123 + 456";
	const char* expected = "(binary + (atom 123) (atom 456))";
	Str answer = parse_expr(code);
	CHECK(answer == expected);
}

TEST_CASE("[zay]: parse add mul expression")
{
	const char* code = "123 + 456 * v.xy";
	const char* expected = "(binary + (atom 123) (binary * (atom 456) (dot (atom v).xy)))";
	Str answer = parse_expr(code);
	CHECK(answer == expected);
}

TEST_CASE("[zay]: complex expression")
{
	const char* code = "(*arr[v.x + b * c[123]]).koko(z: float, w)";
	const char* expected = "(call (dot (paren (unary * (indexed (atom arr)[(binary + (dot (atom v).x) (binary * (atom b) (indexed (atom c)[(atom 123)])))]))).koko) (cast (atom z)(type  float)), (atom w))";
	Str answer = parse_expr(code);
	CHECK(answer == expected);
}
