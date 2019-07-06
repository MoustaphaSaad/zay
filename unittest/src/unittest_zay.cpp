#include <doctest/doctest.h>

#include <zay/Src.h>
#include <zay/Scanner.h>
#include <zay/Parser.h>
#include <zay/AST_Lisp.h>
#include <zay/Typer.h>
#include <zay/CGen.h>

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

inline static Str
parse_stmt(const char* str)
{
	Src src = src_from_str(str);
	CHECK(src_scan(src));

	Parser parser = parser_new(src);
	Stmt s = parser_stmt(parser);
	parser_free(parser);

	Stream out = stream_tmp();
	AST_Lisp writer = ast_lisp_new(out);
	ast_lisp_stmt(writer, s);
	Str res = str_from_c(stream_str(out), memory::tmp());

	stmt_free(s);
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

TEST_CASE("[zay]: parse small program")
{
	const char* code = R"CODE(type Direction enum
{
	Up = 1,
	Down = -1,
	Left,
	Right
}

type Point struct
{
	x, y: float32
}

func point_new(x, y: float32): Point {
	var self: Point
	self.x = x
	self.y = y
	return self
}

var origin: Point = point_new(0, 0)

func add(a, b: Point): Point {
	var res: Point
	res.x = a.x + b.x
	res.y = a.y + b.y
	return res
}

func foo(a: *Point, b: int) {
	a.x *= b
	a.y *= b
}
)CODE";

	const char* expected = R"EXPECTED((enum Direction
	Up = (atom 1)
	Down = (unary - (atom 1))
	Left
	Right
)
(struct Point
	(field x, y: (type  float32))
)
(func point_new
	x, y: (type  float32) 
	: (type  Point)
	(block-stmt
		(var self
			(type  Point)
		)
		(= 
			(dot (atom self).x)
			(atom x)
		)
		(= 
			(dot (atom self).y)
			(atom y)
		)
		(return (atom self))
	)
)
(var origin
	(type  Point)
	(call (atom point_new) (atom 0), (atom 0))
)
(func add
	a, b: (type  Point) 
	: (type  Point)
	(block-stmt
		(var res
			(type  Point)
		)
		(= 
			(dot (atom res).x)
			(binary + (dot (atom a).x) (dot (atom b).x))
		)
		(= 
			(dot (atom res).y)
			(binary + (dot (atom a).y) (dot (atom b).y))
		)
		(return (atom res))
	)
)
(func foo
	a: (type * Point) b: (type  int) 
	(block-stmt
		(*= 
			(dot (atom a).x)
			(atom b)
		)
		(*= 
			(dot (atom a).y)
			(atom b)
		)
	)
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

TEST_CASE("[zay]: complex dereference returned pointer")
{
	const char* code = "*arr[v.x + b * c[123]].koko(z: float, w)";
	const char* expected = "(unary * (call (dot (indexed (atom arr)[(binary + (dot (atom v).x) (binary * (atom b) (indexed (atom c)[(atom 123)])))]).koko) (cast (atom z)(type  float)), (atom w)))";
	Str answer = parse_expr(code);
	CHECK(answer == expected);
}

TEST_CASE("[zay]: complex dereference indexed expression")
{
	const char* code = "(*arr[v.x + b * c[123]]).koko(z: float, w)";
	const char* expected = "(call (dot (paren (unary * (indexed (atom arr)[(binary + (dot (atom v).x) (binary * (atom b) (indexed (atom c)[(atom 123)])))]))).koko) (cast (atom z)(type  float)), (atom w))";
	Str answer = parse_expr(code);
	CHECK(answer == expected);
}

TEST_CASE("[zay]: simple var")
{
	const char* code = R"CODE(var x, y, z: float = 1.0, 2.0, 3.0)CODE";
	const char* expected = R"EXPECTED((var x, y, z
	(type  float)
	(atom 1.0), (atom 2.0), (atom 3.0)
))EXPECTED";
	Str answer = parse_stmt(code);
	CHECK(answer == expected);
}

TEST_CASE("[zay]: complex stmt")
{
	const char* code = R"CODE({
	for var i = 0; i < 1000; ++i {
		if i % 2 == 0 {
			print("% is even", i)
		} else {
			print("% is odd", i)
		}
		if i % 7 {
			continue
		} else {
			break
		}
	}
	return false
})CODE";
	const char* expected = R"EXPECTED((block-stmt
	(for
		(var i
			(atom 0)
		)
		(binary < (atom i) (atom 1000))
		(expr-stmt
			(unary ++ (atom i))
		)
		(block-stmt
			(if (binary == (binary % (atom i) (atom 2)) (atom 0))
				(block-stmt
					(expr-stmt
						(call (atom print) (atom "% is even"), (atom i))
					)
				)
				(block-stmt
					(expr-stmt
						(call (atom print) (atom "% is odd"), (atom i))
					)
				)
			)
			(if (binary % (atom i) (atom 7))
				(block-stmt
					(continue)
				)
				(block-stmt
					(break)
				)
			)
		)
	)
	(return (atom false))
))EXPECTED";
	Str answer = parse_stmt(code);
	CHECK(answer == expected);
}

bool
typecheck(const char* str)
{
	Src src = src_from_str(str);
	CHECK(src_scan(src));
	CHECK(src_parse(src));
	bool res = src_typecheck(src, ITyper::MODE_NONE);
	Str errs = src_errs_dump(src);
	src_free(src);
	str_free(errs);
	return res;
}

TEST_CASE("[zay]: simple function")
{
	CHECK(typecheck("func add(x, y: int): int { return x + y }") == true);
	CHECK(typecheck("func add(x, y: int): int { return a + y }") == false);
	CHECK(typecheck("func add(x, y: int): float32 { return a + y }") == false);
	CHECK(typecheck("func add(x, y: int): float32 { return x + y }") == false);
}

TEST_CASE("[zay]: simple variables")
{
	CHECK(typecheck("var x: int") == true);
	CHECK(typecheck("var x: int = 0") == true);
	CHECK(typecheck("var x: int = 0.0") == false);

	CHECK(typecheck(R"CODE(
		var x = 0.0: float32
		var y: float32 = x
	)CODE") == true);

	CHECK(typecheck(R"CODE(
		var x = 0
		var y: float32 = x
	)CODE") == false);

	CHECK(typecheck(R"CODE(
		var x = 0.0
		var y: float32 = x: float32
	)CODE") == true);
}

TEST_CASE("[zay]: pointers")
{
	CHECK(typecheck(R"CODE(
		func foo(x: int): *int { return &x }
		var x: *int = foo(143);
		var y: int = *x
	)CODE") == true);

	CHECK(typecheck(R"CODE(
		func foo(x: int): *int { return &x }
		var x: *float32 = foo(143);
		var y: float32 = *x
	)CODE") == false);

	CHECK(typecheck(R"CODE(
		func foo(x: int): *int { return &x }
		var x: *float32 = foo(143): *float32;
		var y: float32 = *x
	)CODE") == true);
}

TEST_CASE("[zay]: out of order functions")
{
	CHECK(typecheck(R"CODE(
		func add3(x, y, z: int): int { return add2(x, add2(y, z)) }
		func add2(x, y: int): int { return x + y }
	)CODE") == true);
}

TEST_CASE("[zay]: circular functions")
{
	CHECK(typecheck(R"CODE(
		func add3(x, y, z: int): int { return add2(x, add2(y, z)) }
		func add2(x, y: int): int { return add3(x, y, 1) }
	)CODE") == true);
}

TEST_CASE("[zay]: basic struct")
{
	CHECK(typecheck(R"CODE(
		type Point struct { x, y: float32 }
		func add(a, b: Point): Point {
			var r: Point
			r.x = a.x + b.x
			r.y = a.y + b.y
			return r
		}
	)CODE") == true);

	CHECK(typecheck(R"CODE(
		type Point struct { x, y: float32 }
		func add(a, b: Point): Point {
			var r: Point
			r.x = a.x + b.x
			r.y = a.g + b.y
			return r
		}
	)CODE") == false);
}

TEST_CASE("[zay]: basic struct")
{
	CHECK(typecheck(R"CODE(
		type Point struct { x, y: float32 }
		func add(a, b: *Point) {
			a.x += b.x
			a.y += b.y
		}
	)CODE") == true);

	CHECK(typecheck(R"CODE(
		type Point struct { x, y: float32 }
		func add(a, b: *Point) {
			a.x += b.x
			a.y += b.a
		}
	)CODE") == false);
}

TEST_CASE("[zay]: basic even odd")
{
	CHECK(typecheck(R"CODE(
		func even(n: int): bool{
			if n { return true } else { return false }
		}
	)CODE") == false);

	CHECK(typecheck(R"CODE(
		func even(n: int): bool{
			if n % 2 == 0 { return 1 } else { return false }
		}
		func odd(n: int): bool { return !even(n) }
	)CODE") == false);

	CHECK(typecheck(R"CODE(
		func even(n: int): bool{
			if n % 2 == 0 { return true } else { return false }
		}
		func odd(n: int): bool { return !even(n) }
	)CODE") == true);
}

TEST_CASE("[zay]: basic sum")
{
	CHECK(typecheck(R"CODE(
		func sum(n: int): int {
			var res = 0
			for var i = 0; i < n; ++i {
				res += i
			}
			return res
		}
	)CODE") == true);

	CHECK(typecheck(R"CODE(
		func sum(n: int): float64 {
			var res = 0: float64
			for var i = 0; i < n; ++i {
				res += i
			}
			return res
		}
	)CODE") == false);

	CHECK(typecheck(R"CODE(
		func sum(n: int): float64 {
			var res = 0: float64
			for var i = 0; i < n; ++i {
				res += i: float64
			}
			return res
		}
	)CODE") == true);

	CHECK(typecheck(R"CODE(
		func sum(n: int): float64 {
			var res = 0: float64
			break
			for var i = 0; i < n; ++i {
				res += i: float64
			}
			return res
		}
	)CODE") == false);

	CHECK(typecheck(R"CODE(
		func sum(n: int): float64 {
			var res = 0: float64
			for var i = 0; i < n; ++i {
				if i == 1000 { break } else { continue }
				res += i: float64
			}
			return res
		}
	)CODE") == true);
}

TEST_CASE("[zay]: fib")
{
	CHECK(typecheck(R"CODE(
		func fib(x: int): int {
			if x == 0 { return 0 } else if x == 1 { return 1 } else { return fib(x - 1) + fib(x - 2) }
		}
	)CODE") == true);

	CHECK(typecheck(R"CODE(
		func fib(x: int): int {
			if x == 0 { return 0 } else if x == 1 { return 1 } else { return fib(x - 1) + fibonacci(x - 2) }
		}
	)CODE") == false);
}

TEST_CASE("[zay]: scopes")
{
	CHECK(typecheck(R"CODE(
		func test(x: int) {
			{
				var x: float64
				x = 3.14
			}
		}
	)CODE") == true);

	CHECK(typecheck(R"CODE(
		func test(x: int) {
			{
				var x: float64
				x = 3.14
			}
			x = 3.14
		}
	)CODE") == false);
}

inline static Str
cgen(const char* str)
{
	Src src = src_from_str(str);
	CHECK(src_scan(src));
	CHECK(src_parse(src));
	CHECK(src_typecheck(src, ITyper::MODE_NONE));
	Str res = src_c(src, memory::tmp());
	src_free(src);
	return res;
}

TEST_CASE("[zay]: struct gen")
{
	Str answer = cgen(R"CODE(
		type X struct {
			x, y: int
			z, w: float32
		}
	)CODE");
	const char* expected = R"CODE(typedef struct X {
	ZayInt x;
	ZayInt y;
	ZayFloat32 z;
	ZayFloat32 w;
} X;)CODE";
	CHECK(answer == expected);
}

TEST_CASE("[zay]: fib func gen")
{
	Str answer = cgen(R"CODE(
		func fib(x: int): int {
			if x == 0 { return 0: int } else if x == 1 { return 1 } else { return fib(x - 1) + fib(x - 2) }
		}
	)CODE");
	const char* expected = R"CODE(ZayInt fib(ZayInt x) {
	if (x == 0) {
		return (ZayInt)0;
	} else if (x == 1) {
		return 1;
	} else {
		return fib(x - 1) + fib(x - 2);
	}
})CODE";
	CHECK(answer == expected);
}

TEST_CASE("[zay]: sum func gen")
{
	Str answer = cgen(R"CODE(
		func sum(n: int): int {
			var res = 0
			for var i = 0; i < n; ++i { res += i }
			return res
		}
	)CODE");
	const char* expected = R"CODE(ZayInt sum(ZayInt n) {
	ZayInt res = 0;
	for (ZayInt i = 0; i < n; ++i) {
		res += i;
	}
	return res;
})CODE";
	CHECK(answer == expected);
}

TEST_CASE("[zay]: global var gen")
{
	Str answer = cgen(R"CODE(
		var x, y = 0, 3.14
	)CODE");
	const char* expected = R"CODE(ZayInt x = 0;
ZayFloat64 y = 3.14;)CODE";
	CHECK(answer == expected);
}
