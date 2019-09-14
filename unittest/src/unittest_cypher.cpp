#include <doctest/doctest.h>

#include <cypher/Pkg.h>
#include <cypher/Builder.h>
#include <cypher/VM.h>

using namespace mn;
using namespace cypher;

TEST_CASE("Koko")
{
	// Create a package
	Pkg p = pkg_new("test");
	Func f = pkg_func_new(p, "add", type_func(type_int32, {type_int32, type_int32}));

	// Build the function
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


	// Try execute the add function using the VM
	VM vm = vm_new();
	auto args = mn::buf_lit({ vm_int32(1), vm_int32(2) });

	VM_Value ret = vm_func_run(vm, p, "add", args);
	CHECK(ret.kind == VM_Value::KIND_INT32);
	CHECK(ret.i32 == 3);

	mn::buf_free(args);
	vm_free(vm);

	// Free the package
	pkg_free(p);
}
