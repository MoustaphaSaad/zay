#include <doctest/doctest.h>

#include <zay/Src.h>
#include <zay/scan/Scanner.h>
#include <zay/parse/Parser.h>
#include <zay/parse/AST_Lisp.h>
#include <zay/typecheck/Typer.h>
#include <zay/CGen.h>

inline static mn::Str
scan(const char* str)
{
	auto src = zay::src_from_str(str);
	CHECK(zay::src_scan(src));
	auto res = zay::src_tkns_dump(src, mn::memory::tmp());
	zay::src_free(src);
	return res;
}

inline static mn::Str
parse(const char* str)
{
	auto src = zay::src_from_str(str);
	CHECK(zay::src_scan(src));
	CHECK(zay::src_parse(src, zay::MODE::NONE));
	auto res = zay::src_ast_dump(src, mn::memory::tmp());
	zay::src_free(src);
	return res;
}

inline static mn::Str
parse_expr(const char* str)
{
	auto src = zay::src_from_str(str);
	CHECK(zay::src_scan(src));

	auto parser = zay::parser_new(src);
	auto e = zay::parser_expr(parser);
	zay::parser_free(parser);

	auto out = mn::memory_stream_new(mn::memory::tmp());
	auto writer = zay::ast_lisp_new(out);
	zay::ast_lisp_expr(writer, e);
	auto res = mn::memory_stream_str(out);

	zay::expr_free(e);
	zay::src_free(src);
	return res;
}

inline static mn::Str
parse_stmt(const char* str)
{
	auto src = zay::src_from_str(str);
	CHECK(zay::src_scan(src));

	auto parser = zay::parser_new(src);
	auto s = zay::parser_stmt(parser);
	zay::parser_free(parser);

	auto out = mn::memory_stream_new(mn::memory::tmp());
	auto writer = zay::ast_lisp_new(out);
	zay::ast_lisp_stmt(writer, s);
	auto res = mn::memory_stream_str(out);

	zay::stmt_free(s);
	zay::src_free(src);
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

	auto answer = scan(code);
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

	auto answer = scan(code);
	CHECK(answer == expected);
}

TEST_CASE("[zay]: parse basic struct")
{
	const char* code = R"CODE(type foo struct{
	x, y: int
	z, w: float32
}
)CODE";

	const char* expected = R"EXPECTED((type-decl foo
	(type-sign (struct
		(field x, y: (type-sign  int))
		(field z, w: (type-sign  float32))
	))
)
)EXPECTED";

	auto answer = parse(code);
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

	const char* expected = R"EXPECTED((type-decl foo
	(type-sign (union
		(field f64: (type-sign  float64))
		(field f32: (type-sign [2] float32))
		(field u8: (type-sign [8] uint8))
		(field ptr: (type-sign [8]* uint8))
	))
)
)EXPECTED";

	auto answer = parse(code);
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

	const char* expected = R"EXPECTED((type-decl Direction
	(type-sign (enum
		(field Up (atom 1))
		(field Down (unary - (atom 1)))
		(field Left)
		(field Right)
	))
)
(type-decl Point
	(type-sign (struct
		(field x, y: (type-sign  float32))
	))
)
(func point_new
	x, y: (type-sign  float32) 
	: (type-sign  Point)
	(block-stmt
		(var self
			(type-sign  Point)
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
	(type-sign  Point)
	(call (atom point_new) (atom 0), (atom 0))
)
(func add
	a, b: (type-sign  Point) 
	: (type-sign  Point)
	(block-stmt
		(var res
			(type-sign  Point)
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
	a: (type-sign * Point) b: (type-sign  int) 
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

	auto answer = parse(code);
	CHECK(answer == expected);
}

TEST_CASE("[zay]: parse basic expression")
{
	const char* code = "123 + 456";
	const char* expected = "(binary + (atom 123) (atom 456))";
	auto answer = parse_expr(code);
	CHECK(answer == expected);
}

TEST_CASE("[zay]: parse add mul expression")
{
	const char* code = "123 + 456 * v.xy";
	const char* expected = "(binary + (atom 123) (binary * (atom 456) (dot (atom v).xy)))";
	auto answer = parse_expr(code);
	CHECK(answer == expected);
}

TEST_CASE("[zay]: complex dereference returned pointer")
{
	const char* code = "*arr[v.x + b * c[123]].koko(z: float32, w)";
	const char* expected = "(unary * (call (dot (indexed (atom arr)[(binary + (dot (atom v).x) (binary * (atom b) (indexed (atom c)[(atom 123)])))]).koko) (cast (atom z)(type-sign  float32)), (atom w)))";
	auto answer = parse_expr(code);
	CHECK(answer == expected);
}

TEST_CASE("[zay]: complex dereference indexed expression")
{
	const char* code = "(*arr[v.x + b * c[123]]).koko(z: float, w)";
	const char* expected = "(call (dot (paren (unary * (indexed (atom arr)[(binary + (dot (atom v).x) (binary * (atom b) (indexed (atom c)[(atom 123)])))]))).koko) (cast (atom z)(type-sign  float)), (atom w))";
	auto answer = parse_expr(code);
	CHECK(answer == expected);
}

TEST_CASE("[zay]: simple var")
{
	const char* code = R"CODE(var x, y, z: float = 1.0, 2.0, 3.0)CODE";
	const char* expected = R"EXPECTED((var x, y, z
	(type-sign  float)
	(atom 1.0), (atom 2.0), (atom 3.0)
))EXPECTED";
	auto answer = parse_stmt(code);
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
	auto answer = parse_stmt(code);
	CHECK(answer == expected);
}

TEST_CASE("[zay]: composite literal")
{
	const char* code = R"CODE(
		type Vec3f struct { x, y, z: float32 }
		var origin = &Vec3f{x: 0, y: 0, z: 0}
		var origin2 = Vec3f{}
		func IsOrigin(a: Vec3f): bool { if a == Vec3f{x: 0, y: 0, z: 0} { return true } else { return false } }
		func foo(a: Vec3f) { if a {} }
		func foo2(a: Vec3f) { if a == Vec3f{} {x: 0} }
	)CODE";
	const char* expected = R"CODE((type-decl Vec3f
	(type-sign (struct
		(field x, y, z: (type-sign  float32))
	))
)
(var origin
	(unary & (complit (type-sign  Vec3f)
		(atom x) (atom 0)
		(atom y) (atom 0)
		(atom z) (atom 0)
	))
)
(var origin2
	(complit (type-sign  Vec3f)
	)
)
(func IsOrigin
	a: (type-sign  Vec3f) 
	: (type-sign  bool)
	(block-stmt
		(if (binary == (atom a) (complit (type-sign  Vec3f)
			(atom x) (atom 0)
			(atom y) (atom 0)
			(atom z) (atom 0)
		))
			(block-stmt
				(return (atom true))
			)
			(block-stmt
				(return (atom false))
			)
		)
	)
)
(func foo
	a: (type-sign  Vec3f) 
	(block-stmt
		(if (atom a)
			(block-stmt
			)

		)
	)
)
(func foo2
	a: (type-sign  Vec3f) 
	(block-stmt
		(if (binary == (atom a) (complit (type-sign  Vec3f)
		))
			(block-stmt
				(expr-stmt
					(cast (atom x)(type-sign ))
				)
				(expr-stmt
					(atom 0)
				)
			)

		)
	)
)
)CODE";
	auto answer = parse(code);
	CHECK(answer == expected);
}

bool
typecheck(const char* str)
{
	auto src = zay::src_from_str(str);
	CHECK(zay::src_scan(src));
	CHECK(zay::src_parse(src, zay::MODE::NONE));
	bool res = zay::src_typecheck(src, zay::Typer::MODE_NONE);
	auto errs = zay::src_errs_dump(src);
	zay::src_free(src);
	mn::str_free(errs);
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

TEST_CASE("[zay]: basic struct pointer")
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

TEST_CASE("[zay]: circular structs")
{
	CHECK(typecheck(R"CODE(
		type X struct {
			x: int
			y: X
		}
	)CODE") == false);

	CHECK(typecheck(R"CODE(
		type X struct {
			x: int
			y: Y
		}
		type Y struct {
			x: X
		}
	)CODE") == false);
}

TEST_CASE("[zay] simple union")
{
	CHECK(typecheck(R"CODE(
		type Register union {
			i8: int8 u8: uint8
			i16: int16 u16: uint16
			i32: int32 u32: uint32
			i64: int64 u64: uint64
		}
	)CODE") == true);
}

TEST_CASE("[zay]: circular unions")
{
	CHECK(typecheck(R"CODE(
		type X union {
			x: int
			y: X
		}
	)CODE") == false);

	CHECK(typecheck(R"CODE(
		type X union {
			x: int
			y: Y
		}
		type Y struct {
			x: X
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

TEST_CASE("[zay]: good enums")
{
	CHECK(typecheck(R"CODE(
		type Node_Type enum {
			None = -1,
			Binary,
			Unary
		}
	)CODE") == true);
}

TEST_CASE("[zay]: bad enums")
{
	CHECK(typecheck(R"CODE(
		type Node_Type enum {
			None = -3.14,
			Binary,
			Unary
		}
	)CODE") == false);
}

TEST_CASE("[zay]: strong type alias")
{
	CHECK(typecheck(R"CODE(
	type X int
	func getX(): int {
		var x: X = 0
		return x
	}
	)CODE") == false);
}

TEST_CASE("[zay]: strong type alias casting")
{
	CHECK(typecheck(R"CODE(
	type X int
	func getX(): int {
		var x: X = 0: X
		return x: int
	}
	)CODE") == true);

	CHECK(typecheck(R"CODE(
	type X float32
	func getX(): int {
		var x: X = 0: X
		return x: int
	}
	)CODE") == true);
}

TEST_CASE("[zay]: missing return")
{
	CHECK(typecheck(R"CODE(
	func add(a, b: int): int {
	}
	)CODE") == false);

	CHECK(typecheck(R"CODE(
	func add(a, b: int): int {
		if a < b {} else { return a - b }
	}
	)CODE") == false);

	CHECK(typecheck(R"CODE(
	func add(a, b: int): int {
		if a < b { return a + b } else if a > b {} else { return a - b }
	}
	)CODE") == false);

	CHECK(typecheck(R"CODE(
	func add(a, b: int): int {
		if a < b { a *= -1 } else { return a - b }
		return a + b
	}
	)CODE") == true);

	CHECK(typecheck(R"CODE(
	func add(a, b: int): int {
		for var i = 0; i < a; ++i {
			return a + i + b
		}
	}
	)CODE") == true);

	CHECK(typecheck(R"CODE(
	func add(a, b: int): int {
		for var i = 0; i < a; ++i {
			if a < b {
				return a + i + b
			} else {
			}
		}
	}
	)CODE") == false);
}

inline static mn::Str
cgen(const char* str)
{
	auto src = zay::src_from_str(str);
	CHECK(zay::src_scan(src));
	CHECK(zay::src_parse(src, zay::MODE::NONE));
	CHECK(zay::src_typecheck(src, zay::Typer::MODE_NONE));
	auto res = zay::src_c(src, mn::memory::tmp());
	zay::src_free(src);
	return res;
}

TEST_CASE("[zay]: struct gen")
{
	auto answer = cgen(R"CODE(
		type X struct {
			x, y: int
			z, w: float32
		}
	)CODE");
	const char* expected = R"CODE(typedef struct X {
	ZayInt x;
	ZayInt y;
	float z;
	float w;
} X;)CODE";
	CHECK(answer == expected);
}

TEST_CASE("[zay]: union gen")
{
	auto answer = cgen(R"CODE(
		type X union {
			x, y: int
			z, w: float32
		}
	)CODE");
	const char* expected = R"CODE(typedef union X {
	ZayInt x;
	ZayInt y;
	float z;
	float w;
} X;)CODE";
	CHECK(answer == expected);
}

TEST_CASE("[zay]: fib func gen")
{
	auto answer = cgen(R"CODE(
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
	auto answer = cgen(R"CODE(
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
	auto answer = cgen(R"CODE(
		var x, y = 0, 3.14
	)CODE");
	const char* expected = R"CODE(ZayInt x = 0;
double y = 3.14;)CODE";
	CHECK(answer == expected);
}

TEST_CASE("[zay]: enum codegen")
{
	auto answer = cgen(R"CODE(
	type Direction enum {
		Up = 1,
		Down = -1,
		Left,
		Right
	}
	)CODE");
	const char* expected = R"CODE(typedef enum Direction {
	Up = 1, 
	Down = -1, 
	Left, 
	Right
} Direction;)CODE";
	CHECK(answer == expected);
}

TEST_CASE("[zay]: enum var codegen")
{
	auto answer = cgen(R"CODE(
	type Direction enum {
		Up = 1,
		Down = -1,
		Left,
		Right
	}
	var d = Direction.Up
	)CODE");
	const char* expected = R"CODE(typedef enum Direction {
	Up = 1, 
	Down = -1, 
	Left, 
	Right
} Direction;
Direction d = Direction::Up;)CODE";
	CHECK(answer == expected);
}

TEST_CASE("[zay]: struct pointer")
{
	auto answer = cgen(R"CODE(
	type Point struct { x, y: float32 }
	func add(a: *Point, b: Point) {
		a.x += b.x
		a.y += b.y
	}
	)CODE");
	const char* expected = R"CODE(typedef struct Point {
	float x;
	float y;
} Point;
void add(Point (*a), Point b) {
	a->x += b.x;
	a->y += b.y;
})CODE";
	CHECK(answer == expected);
}

TEST_CASE("[zay]: struct pointer type")
{
	auto answer = cgen(R"CODE(
	type Point *struct { x, y: float32 }
	)CODE");
	const char* expected = R"CODE(typedef struct Point {
	float x;
	float y;
} (*Point);)CODE";
	CHECK(answer == expected);
}

TEST_CASE("[zay]: int alias")
{
	auto answer = cgen(R"CODE(
	type X int
	func add(a, b: X): X {
		return a + b
	}
	)CODE");
	const char* expected = R"CODE(typedef ZayInt X;
ZayInt add(ZayInt a, ZayInt b) {
	return a + b;
})CODE";
	CHECK(answer == expected);
}

TEST_CASE("[zay]: anonymous struct in union")
{
	auto answer = cgen(R"CODE(
	type vec3f union {
		data: [3]float32
		elements: struct {
			x, y, z: float32
		}
	}
	)CODE");
	const char* expected = R"CODE(typedef struct __unnamed_struct_0 {
	float x;
	float y;
	float z;
} __unnamed_struct_0;
typedef union vec3f {
	float (data[3]);
	__unnamed_struct_0 elements;
} vec3f;)CODE";
	CHECK(answer == expected);
}

TEST_CASE("[zay]: array types")
{
	auto answer = cgen(R"CODE(
	type vec3 [3]float32
	type vec4 [4]float32
	type mat4 [4]vec4
	type mat3x4 [3]vec4
	)CODE");
	const char* expected = R"CODE(typedef float (vec3[3]);
typedef float (vec4[4]);
typedef float ((mat4[4])[4]);
typedef float ((mat3x4[3])[4]);)CODE";
	CHECK(answer == expected);
}

TEST_CASE("[zay]: C interface")
{
	//i haven't yet done function types
	auto answer = cgen(R"CODE(
	type Reader struct {
		read: func(request_amount: uint): uint
	}
	)CODE");
	const char* expected = R"CODE(typedef struct Reader {
	ZayUint (*read)(ZayUint);
} Reader;)CODE";
	CHECK(answer == expected);
}

TEST_CASE("[zay]: func pointers")
{
	//i haven't yet done function types
	auto answer = cgen(R"CODE(
	type OP func(a, b: int): int
	func exec(a, b: int, op: OP): int { return op(a, b) }
	func add(a, b: int): int { return a + b }
	var res = exec(1, 2, add)
	)CODE");
	const char* expected = R"CODE(typedef ZayInt (*OP)(ZayInt, ZayInt);
ZayInt exec(ZayInt a, ZayInt b, ZayInt (*op)(ZayInt, ZayInt)) {
	return op(a, b);
}
ZayInt add(ZayInt a, ZayInt b) {
	return a + b;
}
ZayInt res = exec(1, 2, add);)CODE";
	CHECK(answer == expected);
}

TEST_CASE("[zay]: composite literal")
{
	auto answer = cgen(R"CODE(
		type Vec3f struct {x, y, z: float32}
		var origin = Vec3f{}
		type Color struct {r, g, b, a: float32}
		var red = Color{r: 1.0}
		var color_ptr = &Color{g: 1.0}
		var vertices = [3]Vec3f{
			[0]: Vec3f{x: -1.0, y: -1.0},
			[1]: Vec3f{x:  0.0, y:  1.0},
			[2]: Vec3f{x:  1.0, y: -1.0},
		}
	)CODE");
	const char* expected = R"CODE(typedef struct Vec3f {
	float x;
	float y;
	float z;
} Vec3f;
Vec3f origin = (Vec3f){};
typedef struct Color {
	float r;
	float g;
	float b;
	float a;
} Color;
Color red = (Color){
	.r = 1.0
};
Color (*color_ptr) = &(Color){
	.g = 1.0
};
Vec3f (vertices[3]) = {
	[0] = (Vec3f){
		.x = -1.0,
		.y = -1.0
	},
	[1] = (Vec3f){
		.x = 0.0,
		.y = 1.0
	},
	[2] = (Vec3f){
		.x = 1.0,
		.y = -1.0
	}
};)CODE";
	CHECK(answer == expected);
}

TEST_CASE("[zay]: anonymous composite literal")
{
	auto answer = cgen(R"CODE(
		var color = struct { r, g, b, a: float32 } {
			g: 1.0,
			a: 1.0
		}
	)CODE");
	const char* expected = R"CODE(typedef struct __unnamed_struct_0 {
	float r;
	float g;
	float b;
	float a;
} __unnamed_struct_0;
__unnamed_struct_0 color = (__unnamed_struct_0){
	.g = 1.0,
	.a = 1.0
};)CODE";
	CHECK(answer == expected);
}

TEST_CASE("[zay]: function prototypes")
{
	auto answer = cgen(R"CODE(
		func puts(str: string): int32
	)CODE");
	const char* expected = R"CODE(int32_t puts(ZayString str);)CODE";
	CHECK(answer == expected);
}