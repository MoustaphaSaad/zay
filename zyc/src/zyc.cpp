#include <mn/IO.h>
#include <mn/Buf.h>
#include <mn/Str.h>
#include <mn/Defer.h>
#include <mn/Path.h>
#include <mn/Map.h>
#include <mn/Stream.h>

#include <flag/Flag.h>

#include <zay/Src.h>
#include <zay/scan/Scanner.h>
#include <zay/parse/Parser.h>
#include <zay/c/Preprocessor.h>

#include <zsm/Src.h>
#include <zsm/scan/Scanner.h>
#include <zsm/parse/Parser.h>
#include <zsm/Gen.h>

using namespace mn;
using namespace zay;


int
main(int argc, char** argv)
{
	auto args = flag::args_new();
	mn_defer(flag::args_free(args));

	flag::args_cmd(args, "help", "prints this message");
	flag::args_cmd(args, "scan", "scans the file");
	flag::args_cmd(args, "parse", "parses the file");
	flag::args_cmd(args, "cpp", "preprocesses c file");
	flag::args_cmd(args, "asm", "assembles the given files");
	flag::args_cmd(args, "vm-run", "runs the provided bytecode in the zay vm");

	mn::Str output;
	flag::args_str(args, &output, "-o", "a.out", "output of the compiler");
	mn_defer(mn::str_free(output));

	if(flag::args_parse(args, argc, argv) == false)
	{
		flag::args_help(args, mn::file_stdout());
		return -1;
	}

	if(args.cmd == "help")
	{
		flag::args_help(args, mn::file_stdout());
		return 0;
	}
	else if(args.cmd == "scan")
	{
		for(size_t i = 0; i < args.args.count; ++i)
		{
			if(path_is_file(args.args[i]) == false)
			{
				printerr("'{}' is not a file\n", args.args[i]);
				continue;
			}

			Src src = src_from_file(args.args[i].ptr);
			mn_defer(src_free(src));

			//scan the file
			if(src_scan(src) == false)
			{
				print("{}\n", src_errs_dump(src, memory::tmp()));
				continue;
			}

			//write the tokens
			print("{}\n", src_tkns_dump(src, memory::tmp()));
		}
	}
	else if(args.cmd == "parse")
	{
		for(size_t i = 0; i < args.args.count; ++i)
		{
			if(path_is_file(args.args[i]) == false)
			{
				printerr("'{}' is not a file\n", args.args[i]);
				continue;
			}

			Src src = src_from_file(args.args[i].ptr);
			mn_defer(src_free(src));

			//scan the file
			if(src_scan(src) == false)
			{
				print("{}\n", src_errs_dump(src, memory::tmp()));
				continue;
			}

			//parse the file
			if(src_parse(src, MODE::LIB) == false)
			{
				print("{}\n", src_errs_dump(src, memory::tmp()));
				continue;
			}

			//write the ast
			print("{}\n", src_ast_dump(src, memory::tmp()));
		}
	}
	else if(args.cmd == "cpp")
	{
		for(size_t i = 0; i < args.args.count; ++i)
		{
			if(path_is_file(args.args[i]) == false)
			{
				printerr("'{}' is not a file\n", args.args[i]);
				continue;
			}

			Str content = file_content_str(args.args[i]);
			mn_defer(str_free(content));

			Str contentpp = c::preprocess(content);
			mn_defer(str_free(contentpp));

			print("{}\n", contentpp);
		}
	}
	else if(args.cmd == "asm")
	{
		for(size_t i = 0; i < args.args.count; ++i)
		{
			if(path_is_file(args.args[i]) == false)
			{
				printerr("'{}' is not a file\n", args.args[i]);
				continue;
			}

			auto src = zsm::src_from_file(args.args[i].ptr);
			mn_defer(zsm::src_free(src));

			if(zsm::src_scan(src) == false)
			{
				print("{}\n", src_errs_dump(src, memory::tmp()));
				continue;
			}

			if(zsm::src_parse(src) == false)
			{
				print("{}\n", src_errs_dump(src, memory::tmp()));
				continue;
			}

			if(zsm::gen(src) == false)
			{
				print("{}\n", src_errs_dump(src, memory::tmp()));
				continue;
			}

			vm::program_save(src->program, output);
			break;
		}
	}
	else if(args.cmd == "vm-run")
	{
		auto p = vm::program_load(args.args[0]);
		mn::print("VM Run\n");
	}
	else
	{
		flag::args_help(args, mn::file_stdout());
		return -1;
	}
	return 0;
}