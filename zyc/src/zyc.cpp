#include <mn/IO.h>
#include <mn/Buf.h>
#include <mn/Str.h>
#include <mn/Defer.h>
#include <mn/Path.h>
#include <mn/Map.h>
#include <mn/Stream.h>
#include <mn/Result.h>

#include <zay/Src.h>
#include <zay/scan/Scanner.h>
#include <zay/parse/Parser.h>
#include <zay/typecheck/Typer.h>
#include <zay/CGen.h>

static const char* HELP = R"(zyc the zay compiler
zyc command [flags] PATH...

COMMAND:
help:  prints this message
scan:  scans the given input
parse: parses the given input
build: builds and generates the C code for the given input

FLAGS:
-output: specifies the output file
-lib: changes the compiler mode from executable mode (default) to library mode
)";

struct Args
{
	enum KIND
	{
		KIND_NONE,
		KIND_HELP,
		KIND_SCAN,
		KIND_PARSE,
		KIND_BUILD
	};

	KIND kind;
	bool lib;
	union
	{
		struct
		{
			mn::Str path;
		} scan;

		struct
		{
			mn::Str path;
		} parse;

		struct
		{
			mn::Str path;
			mn::Str output;
		} build;
	};
};

inline static Args
args_new()
{
	Args self{};
	return self;
}

inline static void
args_free(Args* self)
{
	switch(self->kind)
	{
	case Args::KIND_NONE:
	case Args::KIND_HELP:
		// do nothing
		break;
	case Args::KIND_SCAN:
		mn::str_free(self->scan.path);
		break;
	case Args::KIND_PARSE:
		mn::str_free(self->parse.path);
		break;
	case Args::KIND_BUILD:
		mn::str_free(self->build.path);
		mn::str_free(self->build.output);
		break;
	default:
		assert(false && "unreachable");
		break;
	}
}

inline static mn::Err
args_parse_flags(Args* self, int* i, int argc, char** argv)
{
	mn::Err res{};
	if(*i >= argc)
		return res;

	for(; *i < argc; ++(*i))
	{
		auto flag = mn::str_lit(argv[*i]);
		if(flag == "-lib")
		{
			self->lib = true;
		}
		else if(flag == "-output")
		{
			if(self->kind == Args::KIND_BUILD)
			{
				self->build.output = mn::str_from_c(flag.ptr);
			}
			else
			{
				res = mn::Err{ "can't specify output flag" };
				break;
			}
		}
		else
		{
			break;
		}
	}
	return res;
}

inline static mn::Err
args_parse(Args* self, int argc, char** argv)
{
	int i = 1;
	if(i >= argc)
		return mn::Err{ "unspecified command" };

	auto cmd = mn::str_lit(argv[i]);
	if(cmd == "help")
	{
		self->kind = Args::KIND_HELP;
	}
	else if(cmd == "scan")
	{
		self->kind = Args::KIND_SCAN;

		++i;
		if(i >= argc)
			return mn::Err{ "unspecified path to scan" };

		if(auto err = args_parse_flags(self, &i, argc, argv))
			return err;

		if(mn::path_exists(argv[i]))
			self->scan.path = mn::str_from_c(argv[i]);
		else
			return mn::Err{ "'{}' is not a file or a folder", argv[i] };
	}
	else if(cmd == "parse")
	{
		self->kind = Args::KIND_PARSE;

		++i;
		if(i >= argc)
			return mn::Err{ "unspecified path to parse" };

		if(auto err = args_parse_flags(self, &i, argc, argv))
			return err;

		if(mn::path_exists(argv[i]))
			self->parse.path = mn::str_from_c(argv[i]);
		else
			return mn::Err{ "'{}' is not a file or a folder", argv[i] };
	}
	else if(cmd == "build")
	{
		self->kind = Args::KIND_BUILD;

		++i;
		if(i >= argc)
			return mn::Err{ "unspecified path to parse" };

		if(auto err = args_parse_flags(self, &i, argc, argv))
			return err;

		if(mn::path_exists(argv[i]))
			self->parse.path = mn::str_from_c(argv[i]);
		else
			return mn::Err{ "'{}' is not a file or a folder", argv[i] };
	}
	else
	{
		return mn::Err{"unknown command '{}'", cmd};
	}
	return mn::Err{};
}


int
main(int argc, char** argv)
{
	auto args = args_new();
	mn_defer(args_free(&args));

	auto err = args_parse(&args, argc, argv);
	if(err)
	{
		mn::printerr("Error: {}\n", err);
		mn::print("{}", HELP);
		return 1;
	}

	if(args.kind == Args::KIND_HELP)
	{
		mn::print("{}", HELP);
		return 0;
	}
	else if(args.kind == Args::KIND_SCAN)
	{
		if(mn::path_is_file(args.scan.path) == false)
		{
			mn::printerr("'{}' is not a file\n", args.scan.path);
			return 1;
		}

		auto src = zay::src_from_file(args.scan.path.ptr);
		mn_defer(zay::src_free(src));

		//scan the file
		if(zay::src_scan(src) == false)
		{
			mn::print("{}\n", zay::src_errs_dump(src, mn::memory::tmp()));
			return 1;
		}

		//write the tokens
		mn::print("{}\n", zay::src_tkns_dump(src, mn::memory::tmp()));
	}
	else if(args.kind == Args::KIND_PARSE)
	{
		if(mn::path_is_file(args.parse.path) == false)
		{
			mn::printerr("'{}' is not a file\n", args.parse.path);
			return 1;
		}

		auto src = zay::src_from_file(args.parse.path.ptr);
		mn_defer(zay::src_free(src));

		//scan the file
		if(zay::src_scan(src) == false)
		{
			mn::print("{}\n", zay::src_errs_dump(src, mn::memory::tmp()));
			return 1;
		}

		//parse the file
		if(zay::src_parse(src, zay::MODE::LIB) == false)
		{
			mn::print("{}\n", zay::src_errs_dump(src, mn::memory::tmp()));
			return 1;
		}

		//write the ast
		mn::print("{}\n", zay::src_ast_dump(src, mn::memory::tmp()));
	}
	else if(args.kind == Args::KIND_BUILD)
	{
		if(mn::path_is_file(args.build.path) == false)
		{
			mn::printerr("'{}' is not a file\n", args.build.path);
			return 1;
		}

		auto src = zay::src_from_file(args.build.path.ptr);
		mn_defer(zay::src_free(src));

		//scan the file
		if(zay::src_scan(src) == false)
		{
			mn::print("{}\n", zay::src_errs_dump(src, mn::memory::tmp()));
			return 1;
		}

		auto parser_mode = args.lib ? zay::MODE::LIB : zay::MODE::EXE;
		//parse the file
		if(zay::src_parse(src, parser_mode) == false)
		{
			mn::print("{}\n", zay::src_errs_dump(src, mn::memory::tmp()));
			return 1;
		}

		auto typer_mode = args.lib ? zay::Typer::MODE_LIB : zay::Typer::MODE_EXE;
		//typecheck the file
		if(zay::src_typecheck(src, typer_mode) == false)
		{
			mn::print("{}\n", zay::src_errs_dump(src, mn::memory::tmp()));
			return 1;
		}

		auto c = zay::src_c(src, mn::memory::tmp());

		mn::print("{}\n", c);
	}
	else
	{
		mn::printerr("unknown command");
		mn::print("{}", HELP);
		return 1;
	}
	return 0;
}
