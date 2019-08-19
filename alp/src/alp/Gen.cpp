#include "alp/Gen.h"

#include <mn/Memory_Stream.h>
#include <mn/Defer.h>
#include <mn/IO.h>
#include <mn/Rune.h>
#include <mn/Map.h>

#include <rgx/Compiler.h>

namespace alp
{
	using namespace mn;

	const char* VM_IMPL = R"""(
	struct Bytes
	{
		uint8_t ptr;
		size_t count;
	};

	struct Result
	{
		const char* it;
		bool matched;
	};

	struct Thread
	{
		size_t ip;
		const char* dp;
	};

	inline static uint8_t
	pop_byte(const Bytes& bytes, size_t &ip)
	{
		return bytes.ptr[ip++];
	}

	inline static int
	pop_int(const Bytes& bytes, size_t &ip)
	{
		assert(ip + sizeof(int) <= bytes.count);
		int res = *(int*)(bytes.ptr+ip);
		ip += sizeof(int);
		return res;
	}

	Result
	match(const Bytes& bytes, const char* str)
	{
		std::vector<Thread> threads;
		threads.push_back(Thread{0, str});
		buf_push(threads, Thread{0, str});
		for(;threads.empty() == false;)
		{
			bool halted = false;
			Thread t = threads.back();
			threads.pop_back();

			for(;;)
			{
				if(t.ip == bytes.count)
					break;

				auto op = ISA(pop_byte(bytes, t.ip));
				if(op == ISA_BYTE)
				{
					if(t.dp == nullptr || *t.dp != char(pop_byte(bytes, t.ip)))
						break;
					++t.dp;
				}
				else if(op == ISA_ANY)
				{
					if(t.dp == nullptr)
						break;
					++t.dp;
				}
				else if(op == ISA_SPLIT)
				{
					int offset1 = pop_int(bytes, t.ip);
					int offset2 = pop_int(bytes, t.ip);

					Thread t2 = t;
					t2.ip += offset2;

					t.ip += offset1;
					threads.push_back(t2);
				}
				else if(op == ISA_JUMP)
				{
					t.ip += pop_int(bytes, t.ip);
				}
				else if(op == ISA_SET || op == ISA_NSET)
				{
					int count = pop_int(bytes, t.ip);
					bool match = false;
					for(size_t i = 0; i < count; ++i)
					{
						auto local_op = ISA(pop_byte(bytes, t.ip));
						if(local_op == ISA_RANGE)
						{
							char a = char(pop_byte(bytes, t.ip));
							char z = char(pop_byte(bytes, t.ip));
							match |= (*t.dp >= a && *t.dp <= z);
						}
						else
						{
							match |= *t.dp == local_op;
						}
					}
					if(op == ISA_SET)
					{
						if(match) ++t.dp;
						else break;
					}
					else if(op == ISA_NSET)
					{
						if(match == false) ++t.dp;
						else break;
					}
				}
				else if(op == ISA_HALT)
				{
					halted = true;
					break;
				}
			}

			if (halted)
				return { t.dp, true };
		}

		return { str, false };
	}
)""";

	struct Gen
	{
		Src src;
		Memory_Stream out;
		size_t level;
		Buf<Decl> exported_tokens;
		Map<const char*, Str> regex_cache;
	};

	inline static Gen
	gen_new(Src src)
	{
		Gen self{};
		self.src = src;
		self.out = memory_stream_new();
		self.level = 0;
		self.exported_tokens = buf_new<Decl>();
		self.regex_cache = map_new<const char*, Str>();
		return self;
	}

	inline static void
	gen_free(Gen& self)
	{
		memory_stream_free(self.out);
		buf_free(self.exported_tokens);
		destruct(self.regex_cache);
	}

	inline static void
	destruct(Gen& self)
	{
		gen_free(self);
	}

	const char*
	gen_regex(Gen& self, const char* key)
	{
		if (auto it = map_lookup(self.regex_cache, key))
			return it->value.ptr;
		return "<UNKNOWN_TOKEN>";
	}

	inline static bool
	gen_build_cache(Gen& self)
	{
		for(Decl d: self.src->ast->decls)
		{
			if (d->kind != IDecl::KIND_TOKEN)
				continue;

			if(auto it = map_lookup(self.regex_cache, d->name.str))
			{
				src_err(self.src, err_tkn(d->name, strf("'{}' token redefinition, first defined", d->name.str)));
				continue;
			}

			Str r = str_new();
			for(Tkn t: d->regex)
			{
				if(t.kind == Tkn::KIND_ID)
				{
					//resolve the name
					if(auto it = map_lookup(self.regex_cache, t.str))
					{
						str_push(r, it->value);
					}
					else
					{
						src_err(self.src, err_tkn(t, strf("'{}' undefined token", t.str)));
					}
				}
				else
				{
					str_push(r, t.str);
				}
			}
			map_insert(self.regex_cache, d->name.str, r);
		}
		return src_has_err(self.src) == false;
	}


	inline static void
	indent(Gen& self)
	{
		for (size_t i = 0; i < self.level; ++i)
			print_to(self.out, "\t");
	}

	inline static void
	newline(Gen& self)
	{
		print_to(self.out, "\n");
		indent(self);
	}

	inline static void
	scope_begin(Gen& self)
	{
		print_to(self.out, " {{");
		++self.level;
		newline(self);
	}

	inline static void
	scope_end(Gen& self, const char* semicolon = "")
	{
		--self.level;
		newline(self);
		print_to(self.out, "}}{}", semicolon);
	}

	inline static bool
	sym_exported(Decl d)
	{
		return rune_upper(d->name.str[0]) == d->name.str[0];
	}

	inline static void
	gen_std_header(Gen& self, Code& res, Src src)
	{
		print_to(self.out, "#pragma once"); newline(self);
		print_to(self.out, "#include <vector>"); newline(self);
		print_to(self.out, "#include <stdint.h>"); newline(self);
		print_to(self.out, "namespace {}", src->ast->package.str);
		scope_begin(self);
		{
			print_to(self.out, "struct Pos {{ uint32_t line, col; }};"); newline(self);
			print_to(self.out, "struct Rng {{ const char *begin, *end; }};"); newline(self);

			print_to(self.out, "struct Tkn");
			scope_begin(self);
			{
				print_to(self.out, "enum class KIND");
				scope_begin(self);
				{
					print_to(self.out, "None,");
					for (size_t i = 0; i < self.exported_tokens.count; ++i)
					{
						newline(self);
						const char* name = self.exported_tokens[i]->name.str;
						print_to(self.out, "{},\\\\{}", name, gen_regex(self, name));
					}
				}
				scope_end(self, ";"); newline(self);

				print_to(self.out, "KIND kind;"); newline(self);
				print_to(self.out, "Rng rng;"); newline(self);
				print_to(self.out, "Pos pos;");
			}
			scope_end(self, ";"); newline(self);

			print_to(self.out, "struct Scan_Result");
			scope_begin(self);
			{
				print_to(self.out, "std::vector<Tkn> tokens;");
			}
			scope_end(self, ";"); newline(self);

			print_to(self.out, "Scan_Result scan(const char* str);");
		}
		scope_end(self);

		str_push(res.header, memory_stream_ptr(self.out));
		memory_stream_clear(self.out);
	}

	inline static void
	gen_std_cpp(Gen& self, Code& res, Src src)
	{
		print_to(self.out, "#include \"{}.h\"", src->ast->package.str); newline(self);
		print_to(self.out, "namespace {}", src->ast->package.str);
		scope_begin(self);
		{
			for(size_t i = 0; i < self.exported_tokens.count; ++i)
			{
				auto regex = gen_regex(self, self.exported_tokens[i]->name.str);
				auto prog = rgx::compile(regex);
				mn_defer(buf_free(prog));

				print_to(self.out, "//{}", regex); newline(self);
				print_to(self.out, "size_t TKN_{}_SIZE = {};", self.exported_tokens[i]->name.str, prog.count); newline(self);
				print_to(self.out, "uint8_t TKN_{}[] =", self.exported_tokens[i]->name.str);
				scope_begin(self);
				{
					for (size_t j = 0; j < prog.count; ++j)
					{
						if (j != 0)
							print_to(self.out, ", ");
						print_to(self.out, "{}", prog[j]);
					}
				}
				scope_end(self);
				print_to(self.out, ";"); newline(self);
			}

			print_to(self.out, "{}", VM_IMPL); newline(self);

			print_to(self.out, "Bytes PROGRAMS[] =");
			scope_begin(self);
			{
				for (size_t i = 0; i < self.exported_tokens.count; ++i)
				{
					if (i != 0)
					{
						print_to(self.out, ",");
						newline(self);
					}
					print_to(self.out, "Bytes {{ TKN_{0}, TKN_{0}_SIZE }}", self.exported_tokens[i]->name.str);
				}
			}
			scope_end(self, ";"); newline(self);

			print_to(self.out, "Scan_Result scan(const char* str)");
			scope_begin(self);
			{
				print_to(self.out, "Scan_Result res;"); newline(self);
				print_to(self.out, "const char* it = str;"); newline(self);
				print_to(self.out, "while(it && *it)");
				scope_begin(self);
				{
					print_to(self.out, "bool found = false;"); newline(self);
					print_to(self.out, "for(size_t i = 0; i < {}; ++i)", self.exported_tokens.count);
					scope_begin(self);
					{
						print_to(self.out, "Result match_res = match(programs[i], it);"); newline(self);
						print_to(self.out, "if(match_res.matched)");
						scope_begin(self);
						{
							print_to(self.out, "Tkn tkn{{}};"); newline(self);
							print_to(self.out, "tkn.kind = Tkn::KIND(i);"); newline(self);
							print_to(self.out, "tkn.rng.begin = it;"); newline(self);
							print_to(self.out, "tkn.rng.end = match_res.it;"); newline(self);
							print_to(self.out, "res.tokens.push_back(tkn);"); newline(self);
							print_to(self.out, "it = match_res.it;"); newline(self);
							print_to(self.out, "found = true;"); newline(self);
							print_to(self.out, "break;");
						}
						scope_end(self);
					}
					scope_end(self); newline(self);

					print_to(self.out, "if(found == false) break;");
				}
				scope_end(self); newline(self);
				print_to(self.out, "return res;");
			}
			scope_end(self);
		}
		scope_end(self);

		str_push(res.cpp, memory_stream_ptr(self.out));
		memory_stream_clear(self.out);
	}



	//API
	Code
	gen_std(Src src, Allocator allocator)
	{
		Code res{ str_with_allocator(allocator), str_with_allocator(allocator) };

		Gen self = gen_new(src);
		mn_defer(gen_free(self));

		if (gen_build_cache(self) == false)
			return res;

		//extract exported tokens
		for (Decl d : src->ast->decls)
			if (d->kind == IDecl::KIND_TOKEN && sym_exported(d))
				buf_push(self.exported_tokens, d);

		gen_std_header(self, res, src);
		gen_std_cpp(self, res, src);
		return res;
	}
}
