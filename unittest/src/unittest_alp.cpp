#include <doctest/doctest.h>

#include <alp/Src.h>
#include <alp/Scanner.h>
#include <alp/Parser.h>

#include <mn/Defer.h>
#include <mn/Str.h>

using namespace alp;
using namespace mn;

Str
scan(const char* code)
{
	Src src = src_from_str(code);
	mn_defer(src_free(src));
	CHECK(src_scan(src) == true);
	return src_tkns_dump(src, memory::tmp());
}

TEST_CASE("scan")
{
	Str answer = scan(R"""(
		package calc;

		token digit = [0-9];
		token not_digit = [^0-9];
		token Id = not_digit (not_digit | digit)*;
		token String = "\"([^\\\"]|\\.)*\"";
	)""");
	const char* expected = R"""(line: 2, col: 3, kind: "package" str: "package"
line: 2, col: 11, kind: "<ID>" str: "calc"
line: 2, col: 15, kind: ";" str: ";"
line: 4, col: 3, kind: "token" str: "token"
line: 4, col: 9, kind: "<ID>" str: "digit"
line: 4, col: 15, kind: "=" str: "="
line: 4, col: 17, kind: "<SET>" str: "[0-9]"
line: 4, col: 22, kind: ";" str: ";"
line: 5, col: 3, kind: "token" str: "token"
line: 5, col: 9, kind: "<ID>" str: "not_digit"
line: 5, col: 19, kind: "=" str: "="
line: 5, col: 21, kind: "<NSET>" str: "[^0-9]"
line: 5, col: 27, kind: ";" str: ";"
line: 6, col: 3, kind: "token" str: "token"
line: 6, col: 9, kind: "<ID>" str: "Id"
line: 6, col: 12, kind: "=" str: "="
line: 6, col: 14, kind: "<ID>" str: "not_digit"
line: 6, col: 24, kind: "(" str: "("
line: 6, col: 25, kind: "<ID>" str: "not_digit"
line: 6, col: 35, kind: "|" str: "|"
line: 6, col: 37, kind: "<ID>" str: "digit"
line: 6, col: 42, kind: ")" str: ")"
line: 6, col: 43, kind: "*" str: "*"
line: 6, col: 44, kind: ";" str: ";"
line: 7, col: 3, kind: "token" str: "token"
line: 7, col: 9, kind: "<ID>" str: "String"
line: 7, col: 16, kind: "=" str: "="
line: 7, col: 18, kind: "<STRING>" str: "\"([^\\\"]|\\.)*\""
line: 7, col: 38, kind: ";" str: ";"
)""";
	CHECK(answer == expected);
}

Str
parse(const char* code)
{
	Src src = src_from_str(code);
	mn_defer(src_free(src));
	CHECK(src_scan(src) == true);
	CHECK(src_parse(src) == true);
	return src_ast_dump(src, memory::tmp());
}

TEST_CASE("parse")
{
	Str answer = parse(R"""(
		package calc;

		token digit = [0-9];
		token not_digit = [^0-9];
		token Id = not_digit (not_digit | digit)*;
		token String = "\"([^\\\"]|\\.)*\"";
	)""");
	const char* expected = R"""(token digit: [0-9];
token not_digit: [^0-9];
token Id: not_digit(not_digit|digit)*;
token String: \"([^\\\"]|\\.)*\";
)""";
	CHECK(answer == expected);
}

