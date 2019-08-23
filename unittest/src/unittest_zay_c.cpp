#include <doctest/doctest.h>

#include <zay/c/Preprocessor.h>

using namespace mn;
using namespace zay;

inline static Str
preprocess(const char* code)
{
	return c::preprocess(code, memory::tmp());
}

TEST_CASE("simple")
{
	Str answer = preprocess(R"""(
#pragma once
#include <stdio.h>
#include <stdlib.h>

#define MyFive do {\
	return 5;\
}while(0)

inline static int foo()
{
	MyFive;
}

int main()
{
	return 0;
}
)""");

	const char* expected = R"""(


inline static int foo()
{
	MyFive;
}

int main()
{
	return 0;
}
)""";
	CHECK(answer == expected);
}
