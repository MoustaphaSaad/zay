#include <mn/IO.h>
#include <mn/Buf.h>
#include <mn/Str.h>
#include <mn/Defer.h>

#include <zay/Src.h>
#include <zay/scan/Scanner.h>
#include <zay/parse/Parser.h>

using namespace mn;
using namespace zay;

const char* HELP_MSG = R"MSG(zyc the zay compiler
zyc [command] [targets] [flags]

COMMANDS:

  help: prints this message
    'zyc help'
  scan: scans the file
    'zyc scan path/to/file.zy'
  parse: parses the file
    'zyc parse path/to/file.zy'
)MSG";

inline static void
print_help()
{
	printfmt("{}\n", HELP_MSG);
}


struct Args
{
	Str command;
	Buf<Str> targets;
	Buf<Str> flags;
};

inline static void
args_parse(Args& self, int argc, char** argv)
{
	if(argc < 2)
	{
		print_help();
		return;
	}

	self.command = str_from_c(argv[1]);
	for(size_t i = 2; i < size_t(argc); ++i)
	{
		if(str_prefix(argv[i], "--"))
			buf_push(self.flags, str_from_c(argv[i] + 2));
		else if(str_prefix(argv[i], "-"))
			buf_push(self.flags, str_from_c(argv[i] + 1));
		else
			buf_push(self.targets, str_from_c(argv[i]));
	}
}

inline static Args
args_new(int argc, char** argv)
{
	Args self{};
	self.command = str_new();
	self.targets = buf_new<Str>();
	self.flags = buf_new<Str>();
	args_parse(self, argc, argv);
	return self;
}

inline static void
args_free(Args& self)
{
	str_free(self.command);
	destruct(self.targets);
	destruct(self.flags);
}

inline static bool
args_has_flag(Args& self, const char* search)
{
	for(const Str& f: self.flags)
		if(f == search)
			return true;
	return false;
}

int
main(int argc, char** argv)
{
	Args args = args_new(argc, argv);
	mn_defer(args_free(args));

	if(args.command == "scan")
	{
		for(size_t i = 0; i < args.targets.count; ++i)
		{
			if(path_is_file(args.targets[i]) == false)
			{
				printfmt_err("'{}' is not a file\n", args.targets[i]);
				continue;
			}

			Src src = src_from_file(args.targets[i].ptr);
			mn_defer(src_free(src));

			//scan the file
			if(src_scan(src) == false)
			{
				printfmt("{}\n", src_errs_dump(src, memory::tmp()));
				continue;
			}

			//write the tokens
			printfmt("{}\n", src_tkns_dump(src, memory::tmp()));
		}
	}
	else if(args.command == "parse")
	{
		for(size_t i = 0; i < args.targets.count; ++i)
		{
			if(path_is_file(args.targets[i]) == false)
			{
				printfmt_err("'{}' is not a file\n", args.targets[i]);
				continue;
			}

			Src src = src_from_file(args.targets[i].ptr);
			mn_defer(src_free(src));

			//scan the file
			if(src_scan(src) == false)
			{
				printfmt("{}\n", src_errs_dump(src, memory::tmp()));
				continue;
			}

			//parse the file
			if(src_parse(src, MODE::LIB) == false)
			{
				printfmt("{}\n", src_errs_dump(src, memory::tmp()));
				continue;
			}

			//write the ast
			printfmt("{}\n", src_ast_dump(src, memory::tmp()));
		}
	}
	return 0;
}