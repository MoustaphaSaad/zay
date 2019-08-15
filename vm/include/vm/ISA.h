#pragma once

#include <stdint.h>

namespace vm
{
	enum class ISA: uint8_t
	{
		//illegal instruction
		IGL,

		//Arithmetic instructions

		//Addition instruction
		//ADD {Result Register} {Operand 1 Register} {Operand 2 Register}
		//Result = Operand 1 + Operand 2
		I8_ADD,
		I16_ADD,
		I32_ADD,
		I64_ADD,
		U8_ADD,
		U16_ADD,
		U32_ADD,
		U64_ADD,

		//Subtraction instruction
		//SUB {Result Register} {Operand 1 Register} {Operand 2 Register}
		//Result = Operand 1 - Operand 2
		I8_SUB,
		I16_SUB,
		I32_SUB,
		I64_SUB,
		U8_SUB,
		U16_SUB,
		U32_SUB,
		U64_SUB,

		//Multiplication instruction
		//MUL {Result Register} {Operand 1 Register} {Operand 2 Register}
		//Result = Operand 1 * Operand 2
		I8_MUL,
		I16_MUL,
		I32_MUL,
		I64_MUL,
		U8_MUL,
		U16_MUL,
		U32_MUL,
		U64_MUL,

		//Division instruction
		//DIV {Result Register} {Operand 1 Register} {Operand 2 Register}
		//Result = Operand 1 / Operand 2
		I8_DIV,
		I16_DIV,
		I32_DIV,
		I64_DIV,
		U8_DIV,
		U16_DIV,
		U32_DIV,
		U64_DIV,

		//Mod instruction
		//MOD {Result Register} {Operand 1 Register} {Operand 2 Register}
		//Result = Operand 1 % Operand 2
		I8_MOD,
		I16_MOD,
		I32_MOD,
		I64_MOD,
		U8_MOD,
		U16_MOD,
		U32_MOD,
		U64_MOD,

		//Compare instructions

		//Equal instruction: compares the bits without regard to its signedness
		//EQ {Result Register} {Operand 1 Register} {Operand 2 Register}
		//Result = Operand 1 == Operand 2
		U8_EQ,
		U16_EQ,
		U32_EQ,
		U64_EQ,

		//less than instruction
		//LT {Result Register} {Operand 1 Register} {Operand 2 Register}
		//Result = Operand 1 < Operand 2
		I8_LT,
		I16_LT,
		I32_LT,
		I64_LT,
		U8_LT,
		U16_LT,
		U32_LT,
		U64_LT,

		//greater than instruction
		//GT {Result Register} {Operand 1 Register} {Operand 2 Register}
		//Result = Operand 1 > Operand 2
		I8_GT,
		I16_GT,
		I32_GT,
		I64_GT,
		U8_GT,
		U16_GT,
		U32_GT,
		U64_GT,

		//less than or equal instruction
		//LE {Result Register} {Operand 1 Register} {Operand 2 Register}
		//Result = Operand 1 <= Operand 2
		I8_LE,
		I16_LE,
		I32_LE,
		I64_LE,
		U8_LE,
		U16_LE,
		U32_LE,
		U64_LE,

		//greater than or equal instruction
		//GE {Result Register} {Operand 1 Register} {Operand 2 Register}
		//Result = Operand 1 >= Operand 2
		I8_GE,
		I16_GE,
		I32_GE,
		I64_GE,
		U8_GE,
		U16_GE,
		U32_GE,
		U64_GE,


		//Constant loading instruction: loads the given bit pattern in register
		//CONST {Dest Register} {Constant value}
		//Dest = Constant
		U8_CONST,
		U16_CONST,
		U32_CONST,
		U64_CONST,

		//Sign extend instruction: moves the given value from register to another sign extended
		//X8 {Dest Register} {Src Register}
		//Dest = Src
		I16_X8,
		I32_X8,
		I32_X16,
		I64_X8,
		I64_X16,
		I64_X32,

		//Zero extend instruction: moves the given value from register to another zero extended
		//Z8 {Dest Register} {Src Register}
		//Dest = Src
		U16_X8,
		U32_X8,
		U32_X16,
		U64_X8,
		U64_X16,
		U64_X32,

		//Conditional jump instructions: moves the ip to the given address
		//register == 0 -> false
		//register != 0 -> true
		//JT {Address} -> Jump if True
		//JF {Address} -> Jump if False
		JT,
		JF,

		//Jump instruction: moves the ip to the given address
		//JUMP {Address}
		JUMP,

		//Halt instruction: stops the execution of the program
		//HALT
		HALT,
	};
}
