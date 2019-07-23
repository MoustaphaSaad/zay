#include <doctest/doctest.h>

#include <cypher/Pkg.h>
#include <cypher/Builder.h>

using namespace mn;
using namespace cypher;

TEST_CASE("Koko")
{
	Pkg p = pkg_new("test");
	Func f = pkg_func(p, "add", type_func(type_int, {type_int, type_int}));

	Builder b = builder_new(f);
	Basic_Block_ID entry = builder_basic_block(b);

	builder_start_block(b, entry);
	{
		Val x = builder_arg(b, 0);
		Val y = builder_arg(b, 1);
		Val res = builder_add(b, x, y);
		builder_ret(b, res);
	}
	builder_end_block(b);

	builder_free(b);
	pkg_free(p);
}
