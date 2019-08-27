#include "vm/Program.h"

#include <mn/File.h>

#include <stdint.h>

namespace vm
{
	inline static void
	write_str(mn::File f, const mn::Str& str)
	{
		assert(str.count <= UINT32_MAX);
		uint32_t count = uint32_t(str.count);
		mn::stream_write(f, mn::block_from(count));
		mn::stream_write(f, mn::block_from(str));
	}

	inline static void
	write_bytes(mn::File f, const mn::Buf<uint8_t>& bytes)
	{
		assert(bytes.count <= UINT32_MAX);
		uint32_t count = uint32_t(bytes.count);
		mn::stream_write(f, mn::block_from(count));
		mn::stream_write(f, mn::block_from(bytes));
	}

	inline static mn::Str
	read_str(mn::File f)
	{
		auto res = mn::str_new();
		uint32_t count = 0;
		mn::stream_read(f, mn::block_from(count));
		mn::str_resize(res, count);
		mn::stream_read(f, mn::block_from(res));
		return res;
	}

	inline static mn::Buf<uint8_t>
	read_bytes(mn::File f)
	{
		auto res = mn::buf_new<uint8_t>();
		uint32_t count = 0;
		mn::stream_read(f, mn::block_from(count));
		mn::buf_resize(res, count);
		mn::stream_read(f, mn::block_from(res));
		return res;
	}

	//API
	Fixup
	fixup_new(const mn::Str& func, const mn::Str& target, uint32_t offset)
	{
		Fixup self{};
		self.func = func;
		self.target = target;
		self.offset = offset;
		return self;
	}

	void
	fixup_free(Fixup& self)
	{
		mn::str_free(self.func);
		mn::str_free(self.target);
	}


	Program
	program_new()
	{
		Program self{};
		self.funcs = mn::map_new<mn::Str, mn::Buf<uint8_t>>();
		self.fixups = mn::buf_new<Fixup>();
		return self;
	}

	void
	program_free(Program& self)
	{
		mn::destruct(self.funcs);
		mn::destruct(self.fixups);
	}

	void
	program_func_add(Program& self, const mn::Str& name, const mn::Buf<uint8_t> &bytes)
	{
		assert(mn::map_lookup(self.funcs, name) == nullptr);
		mn::map_insert(self.funcs, name, bytes);
	}

	void
	program_fixup(Program& self, const Fixup& fixup)
	{
		mn::buf_push(self.fixups, fixup);
	}

	void
	program_save(Program& self, const mn::Str& filename)
	{
		auto f = mn::file_open(filename, mn::IO_MODE::WRITE, mn::OPEN_MODE::CREATE_OVERWRITE);

		//write the number of functions
		uint32_t func_count = uint32_t(self.funcs.count);
		mn::stream_write(f, mn::block_from(func_count));

		//write the functions
		for(auto it = mn::map_begin(self.funcs);
			it != mn::map_end(self.funcs);
			it = mn::map_next(self.funcs, it))
		{
			write_str(f, it->key);
			write_bytes(f, it->value);
		}

		//write fixup table
		uint32_t fixup_count = uint32_t(self.fixups.count);
		mn::stream_write(f, mn::block_from(fixup_count));

		for(const auto& fixup: self.fixups)
		{
			write_str(f, fixup.func);
			write_str(f, fixup.target);
			mn::stream_write(f, mn::block_from(fixup.offset));
		}

		mn::file_close(f);
	}

	Program
	program_load(const mn::Str& filename)
	{
		auto self = program_new();
		auto f = mn::file_open(filename, mn::IO_MODE::READ, mn::OPEN_MODE::OPEN_ONLY);

		uint32_t func_count = 0;
		mn::stream_read(f, mn::block_from(func_count));
		mn::map_reserve(self.funcs, func_count);

		for(uint32_t i = 0; i < func_count; ++i)
		{
			auto name = read_str(f);
			auto bytes = read_bytes(f);
			mn::map_insert(self.funcs, name, bytes);
		}

		uint32_t fixup_count = 0;
		mn::stream_read(f, mn::block_from(fixup_count));
		mn::buf_resize(self.fixups, fixup_count);

		for(uint32_t i = 0; i < fixup_count; ++i)
		{
			self.fixups[i].func = read_str(f);
			self.fixups[i].target = read_str(f);
			mn::stream_read(f, mn::block_from(self.fixups[i].offset));
		}

		mn::file_close(f);
		return self;
	}
}