#pragma once

#include "flag/Exports.h"

#include <mn/Str.h>
#include <mn/Buf.h>
#include <mn/Stream.h>

namespace flag
{
	struct Var
	{
		enum KIND
		{
			KIND_STR,
			KIND_BOOL
		};

		KIND kind;
		mn::Str key;
		mn::Str desc;
		bool found;
		union {
			struct
			{
				//default value
				mn::Str def;
				//pointer to actual value
				mn::Str *var;
			} str;
			struct
			{
				bool def;
				bool *var;
			} boolean;
		};
	};

	FLAG_EXPORT Var
	var_str(mn::Str* var, const mn::Str& key, const mn::Str& def, const mn::Str& desc);

	FLAG_EXPORT Var
	var_bool(bool* var, const mn::Str& key, bool def, const mn::Str& desc);

	FLAG_EXPORT void
	var_free(Var& self);

	inline static void
	destruct(Var& self)
	{
		var_free(self);
	}

	struct Cmd
	{
		mn::Str name;
		mn::Str desc;
	};

	inline static Cmd
	cmd_new(const mn::Str& name, const mn::Str& desc)
	{
		return Cmd{ name, desc };
	}

	inline static void
	cmd_free(Cmd& self)
	{
		mn::str_free(self.name);
		mn::str_free(self.desc);
	}

	inline static void
	destruct(Cmd& self)
	{
		cmd_free(self);
	}

	struct Args
	{
		mn::Buf<Var> vars;
		mn::Buf<Cmd> cmds;
		mn::Buf<mn::Str> args;
		mn::Str cmd;
	};

	FLAG_EXPORT Args
	args_new();

	FLAG_EXPORT void
	args_free(Args& self);

	inline static void
	destruct(Args& self)
	{
		args_free(self);
	}

	inline static void
	args_cmd(Args& self, const mn::Str& name, const mn::Str& desc)
	{
		mn::buf_push(self.cmds, cmd_new(name, desc));
	}

	inline static void
	args_cmd(Args& self, const char* name, const char* desc)
	{
		mn::buf_push(self.cmds, cmd_new(mn::str_from_c(name), mn::str_from_c(desc)));
	}

	inline static void
	args_str(Args& self, mn::Str* var, const mn::Str& key, const mn::Str& def, const mn::Str& desc)
	{
		mn::buf_push(self.vars, var_str(var, key, def, desc));
	}

	inline static void
	args_str(Args& self, mn::Str* var, const char* key, const char* def, const char* desc)
	{
		mn::buf_push(self.vars, var_str(var, mn::str_from_c(key), mn::str_from_c(def), mn::str_from_c(desc)));
	}

	inline static void
	args_bool(Args& self, bool *var, const mn::Str& key, bool def, const mn::Str& desc)
	{
		mn::buf_push(self.vars, var_bool(var, key, def, desc));
	}

	inline static void
	args_bool(Args& self, bool *var, const char* key, bool def, const char* desc)
	{
		mn::buf_push(self.vars, var_bool(var, mn::str_from_c(key), def, mn::str_from_c(desc)));
	}

	FLAG_EXPORT bool
	args_parse(Args& self, int argc, char** argv);

	FLAG_EXPORT void
	args_help(Args& self, mn::Stream stream);
}