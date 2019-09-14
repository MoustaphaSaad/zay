#include "cypher/VM.h"
#include "cypher/Ins.h"

#include <mn/Memory.h>
#include <mn/Map.h>
#include <mn/Defer.h>

namespace cypher
{
	struct IVM
	{
		const mn::Buf<VM_Value> *args;
		mn::Map<Ins_ID, VM_Value> env;
	};

	inline static VM_Value
	val_resolve(Val v, VM self)
	{
		switch(v.kind)
		{
		case Val::KIND_ARG: return (*self->args)[v.arg.ix];
		case Val::KIND_INS:
			if(auto it = mn::map_lookup(self->env, v.ins.ins_id))
				return it->value;
			return VM_Value{};
		default: assert(false && "unimplemented"); return VM_Value{};
		}
	}

	// API
	VM
	vm_new()
	{
		auto self = mn::alloc<IVM>();
		self->args = nullptr;
		self->env = mn::map_new<Ins_ID, VM_Value>();
		return self;
	}

	void
	vm_free(VM self)
	{
		mn::map_free(self->env);
		mn::free(self);
	}

	VM_Value
	vm_func_run(VM self, Pkg pkg, const mn::Str& func_name, const mn::Buf<VM_Value>& args)
	{
		self->args = &args;
		mn_defer(self->args = nullptr);

		Func f = pkg_func_get(pkg, func_name);
		if(f == nullptr)
			return VM_Value{};

		if(f->blocks.count == 0)
			return VM_Value{};

		bool exit = false;
		VM_Value ret{};
		// Block 0 is the entry block
		Basic_Block_ID block_ix = 0;
		while(exit == false)
		{
			//we can traverse the instructions since each basic block doesn't contain
			//jumps, or other indirections
			for(auto ins_ix: f->blocks[block_ix].instructions)
			{
				auto& ins = f->ins[ins_ix];
				switch(ins.op)
				{
				case Ins::OP_ADD:
				{
					auto res = vm_value_add(val_resolve(ins.op_add.a, self), val_resolve(ins.op_add.b, self));
					mn::map_insert(self->env, ins_ix, res);
					break;
				}

				case Ins::OP_RET:
				{
					ret = val_resolve(ins.op_ret, self);
					exit = true;
					break;
				}

				default:
					assert(false && "unimplemented");
					break;
				}

				if(exit) break;
			}
		}

		return ret;
	}
}