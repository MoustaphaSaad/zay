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
#include <zay/typecheck/Typer.h>
#include <zay/CGen.h>
#include <zay/c/Preprocessor.h>

int
main(int argc, char** argv)
{
	auto args = flag::args_new();
	mn_defer(flag::args_free(args));

	flag::args_cmd(args, "help", "prints this message");
	flag::args_cmd(args, "scan", "scans the file");
	flag::args_cmd(args, "parse", "parses the file");
	flag::args_cmd(args, "cpp", "preprocesses c file");
	flag::args_cmd(args, "build", "builds and outputs C file");

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
			if(mn::path_is_file(args.args[i]) == false)
			{
				mn::printerr("'{}' is not a file\n", args.args[i]);
				continue;
			}

			auto src = zay::src_from_file(args.args[i].ptr);
			mn_defer(zay::src_free(src));

			//scan the file
			if(zay::src_scan(src) == false)
			{
				mn::print("{}\n", zay::src_errs_dump(src, mn::memory::tmp()));
				continue;
			}

			//write the tokens
			mn::print("{}\n", zay::src_tkns_dump(src, mn::memory::tmp()));
		}
	}
	else if(args.cmd == "parse")
	{
		for(size_t i = 0; i < args.args.count; ++i)
		{
			if(mn::path_is_file(args.args[i]) == false)
			{
				mn::printerr("'{}' is not a file\n", args.args[i]);
				continue;
			}

			auto src = zay::src_from_file(args.args[i].ptr);
			mn_defer(zay::src_free(src));

			//scan the file
			if(zay::src_scan(src) == false)
			{
				mn::print("{}\n", zay::src_errs_dump(src, mn::memory::tmp()));
				continue;
			}

			//parse the file
			if(zay::src_parse(src, zay::MODE::LIB) == false)
			{
				mn::print("{}\n", zay::src_errs_dump(src, mn::memory::tmp()));
				continue;
			}

			//write the ast
			mn::print("{}\n", zay::src_ast_dump(src, mn::memory::tmp()));
		}
	}
	else if(args.cmd == "cpp")
	{
		for(size_t i = 0; i < args.args.count; ++i)
		{
			if(mn::path_is_file(args.args[i]) == false)
			{
				mn::printerr("'{}' is not a file\n", args.args[i]);
				continue;
			}

			auto content = mn::file_content_str(args.args[i]);
			mn_defer(mn::str_free(content));

			auto contentpp = zay::c::preprocess(content);
			mn_defer(mn::str_free(contentpp));

			mn::print("{}\n", contentpp);
		}
	}
	else if(args.cmd == "build")
	{
		for(size_t i = 0; i < args.args.count; ++i)
		{
			if(mn::path_is_file(args.args[i]) == false)
			{
				mn::printerr("'{}' is not a file\n", args.args[i]);
				continue;
			}

			auto src = zay::src_from_file(args.args[i].ptr);
			mn_defer(zay::src_free(src));

			//scan the file
			if(zay::src_scan(src) == false)
			{
				mn::print("{}\n", zay::src_errs_dump(src, mn::memory::tmp()));
				continue;
			}

			//parse the file
			if(zay::src_parse(src, zay::MODE::EXE) == false)
			{
				mn::print("{}\n", zay::src_errs_dump(src, mn::memory::tmp()));
				continue;
			}

			//typecheck the file
			if(zay::src_typecheck(src, zay::ITyper::MODE_EXE) == false)
			{
				mn::print("{}\n", zay::src_errs_dump(src, mn::memory::tmp()));
				continue;
			}

			auto c = zay::src_c(src, mn::memory::tmp());

			//write the ast
			mn::print("{}\n", c);
		}
	}
	else
	{
		flag::args_help(args, mn::file_stdout());
		return -1;
	}
	return 0;
}