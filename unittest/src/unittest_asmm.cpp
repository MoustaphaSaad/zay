#include <doctest/doctest.h>

#include <asmm/Src.h>
#include <asmm/scan/Scanner.h>
#include <asmm/parse/Parser.h>
#include <asmm/Gen.h>

#include <mn/Str.h>


inline static mn::Str
scan(const char* code)
{
	auto src = asmm::src_from_str(code);
	CHECK(asmm::src_scan(src));
	auto res = asmm::src_tkns_dump(src, mn::memory::tmp());
	asmm::src_free(src);
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
	auto src = asmm::src_from_str(code);
	CHECK(asmm::src_scan(src));
	CHECK(asmm::src_parse(src));
	CHECK(asmm::gen(src));
	asmm::src_free(src);
}
