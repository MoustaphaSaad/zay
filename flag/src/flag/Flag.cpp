#include "flag/Flag.h"

#include <mn/IO.h>

#include <algorithm>
#include <assert.h>

namespace flag
{
	//API
	Var
	var_str(mn::Str* var, const mn::Str& key, const mn::Str& def, const mn::Str& desc)
	{
		Var self{};
		self.kind = Var::KIND_STR;
		self.key = key;
		self.desc = desc;
		self.str.var = var;
		self.str.def = def;
		return self;
	}

	Var
	var_bool(bool *var, const mn::Str& key, bool def, const mn::Str& desc)
	{
		Var self{};
		self.kind = Var::KIND_BOOL;
		self.key = key;
		self.desc = desc;
		self.boolean.var = var;
		self.boolean.def = def;
		return self;
	}

	void
	var_free(Var& self)
	{
		mn::str_free(self.key);
		mn::str_free(self.desc);
		if(self.kind == Var::KIND_STR)
			mn::str_free(self.str.def);
	}

	Args
	args_new()
	{
		Args self{};
		self.vars = mn::buf_new<Var>();
		self.cmds = mn::buf_new<Cmd>();
		self.args = mn::buf_new<mn::Str>();
		self.cmd = mn::str_new();
		return self;
	}

	void
	args_free(Args& self)
	{
		destruct(self.vars);
		destruct(self.cmds);
		destruct(self.args);
		mn::str_free(self.cmd);
		
	}

	bool
	args_parse(Args& self, int argc, char** argv)
	{
		if (argc <= 1)
			return false;

		std::sort(begin(self.vars), end(self.vars), [](const Var& a, const Var& b){
			return a.key.count < b.key.count;
		});

		int start = 1;
		for(const auto& c: self.cmds)
		{
			if(c.name == argv[start])
			{
				self.cmd = mn::str_from_c(argv[start]);
				++start;
				break;
			}
		}

		for(int i = start; i < argc; ++i)
		{
			auto arg = mn::str_lit(argv[i]);
			for(auto& v: self.vars)
			{
				if(mn::str_prefix(arg, v.key))
				{
					v.found = true;
					switch(v.kind)
					{
					case Var::KIND_STR:
					{
						size_t ix = mn::str_find(arg.ptr + v.key.count, "=", 0);
						if(ix != size_t(-1))
						{
							if(v.str.var)
								*v.str.var = mn::str_from_substr(begin(arg) + ix + 1, end(arg));
						}
						else
						{
							if(i + 1 < argc)
							{
								++i;
								if(v.str.var)
									*v.str.var = mn::str_from_c(argv[i]);
							}
							else
							{
								return false;
							}
						}
						break;
					}
					case Var::KIND_BOOL:
					{
						if(v.boolean.var)
							*v.boolean.var = true;
						break;
					}
					default:
						assert(false && "unreachable");
						break;
					}
				}
				else
				{
					mn::buf_push(self.args, mn::str_from_c(arg.ptr));
				}
			}
		}

		for(auto& v: self.vars)
		{
			if(v.found == false)
			{
				switch(v.kind)
				{
				case Var::KIND_STR:
					if(v.str.var)
						*v.str.var = mn::str_clone(v.str.def);
					break;
				case Var::KIND_BOOL:
					if(v.boolean.var)
						*v.boolean.var = v.boolean.def;
					break;
				default:
					assert(false && "unreachable");
					break;
				}
			}
		}

		return true;
	}

	void
	args_help(Args& self, mn::Stream out)
	{
		if(self.cmds.count > 0)
			mn::print_to(out, "COMMANDS:\n");

		for (const auto& c : self.cmds)
			mn::print_to(out, "  {}: {}\n", c.name, c.desc);

		if(self.vars.count > 0)
			mn::print_to(out, "FLAGS:\n");
		
		for(const auto& v: self.vars)
		{
			mn::print_to(out, "  -{}", v.key);
			switch(v.kind)
			{
			case Var::KIND_STR:
				if(v.str.def.count > 0)
					mn::print_to(out, ": string(default={})", v.str.def);
				else
					mn::print_to(out, ": string(default=\"\")");
				break;
			case Var::KIND_BOOL:
				mn::print_to(out, ": bool(default={})", v.boolean.def);
				break;
			default:
				assert(false && "unreachable");
				break;
			}
			mn::print_to(out, "\n");
		}
	}
}