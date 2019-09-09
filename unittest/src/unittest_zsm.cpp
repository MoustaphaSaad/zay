#include <doctest/doctest.h>

#include <zsm/Src.h>
#include <zsm/scan/Scanner.h>
#include <zsm/parse/Parser.h>
#include <zsm/Gen.h>

#include <mn/Str.h>


inline static mn::Str
scan(const char* code)
{
	auto src = zsm::src_from_str(code);
	CHECK(zsm::src_scan(src));
	auto res = zsm::src_tkns_dump(src, mn::memory::tmp());
	zsm::src_free(src);
	return res;
}

TEST_CASE("scanner")
{
	auto answer = scan(R"""(
		func main.add
			i64.add r0 r0 r1
			ret
		end
	)""");
	const char* expected = R"""(line: 2, col: 3, kind: "func" str: "func"
line: 2, col: 8, kind: "<ID>" str: "main.add"
line: 3, col: 4, kind: "i64.add" str: "i64.add"
line: 3, col: 12, kind: "r0" str: "r0"
line: 3, col: 15, kind: "r0" str: "r0"
line: 3, col: 18, kind: "r1" str: "r1"
line: 4, col: 4, kind: "ret" str: "ret"
line: 5, col: 3, kind: "end" str: "end"
)""";
	CHECK(answer == expected);
}

TEST_CASE("Koko")
{
	const char* code = R"""(
	func main.is_even
		i64.const r1 2		//r1 = 2
		i64.mod r0 r0 r1	//r0 = r0 % r1
		i64.const r1 0		//r1 = 0
		i64.eq r0 r0 r1		//r0 = r0 == r1
	end
)""";
	auto src = zsm::src_from_str(code);
	CHECK(zsm::src_scan(src));
	CHECK(zsm::src_parse(src));
	CHECK(zsm::gen(src));
	zsm::src_free(src);
}
