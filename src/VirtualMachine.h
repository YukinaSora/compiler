#pragma once

#include "Common.h"

#include <iomanip>

class VirtualMachine
{
public:
	using Byte = unsigned char;			// 1 byte
	using Word = unsigned short;		// 2 bytes
	using Pointer = unsigned long long; // 8 bytes

	struct Instruction
	{
		enum Operation
		{
			PASS = 0x00,
			MOV, MOV_OUT,
			ADD, SUB, MUL, DIV, MOD,
			AND, OR, XOR, NOT,
			CMP,
			JMP, JE, JNE, JG, JL, JGE, JLE,
			CALL, RET,
			OUT
		};

		struct ConstructArgs
		{
			Operation operation;
			Word args[2];

			ConstructArgs(Operation operation, Word arg1, Word arg2)
			{
				this->operation = operation;
				args[0] = arg1;
				args[1] = arg2;
			}
		};

		Instruction(ConstructArgs args)
		{
			operation = args.operation;
			this->args[0] = args.args[0];
			this->args[1] = args.args[1];
		}

		Operation operation;
		Word args[2];

		constexpr static const size_t InstructionSize = sizeof(Operation) + 2 * sizeof(Word);
	};

	enum Register
	{
		AX = 0x00,
		BX,
		CX,
		DX,
		SF, // signal flag
		REGISTER_SIZE
	};

	enum Flag
	{
		ZF = 0x00, // zero flag
		EXT // exit
	};

	enum ErrorCode
	{
		INVALID_INSTRUCTION = 0x00,
		INVALID_REGISTER,
		INVALID_ACCESS
	};

	template<class T>
	struct RegisterTemplate
	{
		struct ConstructArgs
		{
			const T value;
			const T base;
		};

		RegisterTemplate(ConstructArgs args = { 0, 0 }) : value(args.value), base(args.base), size(sizeof(T)) {}

		template<class _Ty>
		void Init(_Ty value) { this->value = static_cast<T>(value); this->base = static_cast<T>(value); }

		template<class _Ty>
		void operator = (_Ty value) { this->value = reinterpret_cast<T>(value); }

		template<class _Ty>
		T operator - (const _Ty& value) { return this->value - static_cast<T>(value); }

		template<class _Ty>
		T operator + (const _Ty& value) { return this->value + static_cast<T>(value); }

		template<class _Ty>
		T operator -= (const _Ty& value) { return this->value -= static_cast<T>(value); }

		template<class _Ty>
		T operator += (const _Ty& value) { return this->value += static_cast<T>(value); }

		bool IsInRange(size_t length = 0) { return (value + length) >= base && 
												   (value + length) <  base + size; }

		T value;
		T base;
		size_t size;
	};

private:
	using InstructionList = Vector<Instruction>;
	using EnhancedRegister = RegisterTemplate<Pointer>;
	using GeneralRegister = Word;

private:
	constexpr static const size_t s_MemorySize		= 1 << 10;
	constexpr static const size_t s_InstructionSize = 1 << 9;
	constexpr static const size_t s_StackSize		= 1 << 8;
	constexpr static const size_t s_DataSize		= s_MemorySize - s_StackSize - s_InstructionSize;

public:
	VirtualMachine();
	~VirtualMachine();

	void Execute(InstructionList instructions);

private:
	void ResetMemory();
	bool WriteMemory(EnhancedRegister& src, Byte* data, size_t length);
	bool WriteMemory(Pointer address, Byte* data, size_t length);
	Byte* ReadMemory(EnhancedRegister& src, size_t length);
	Instruction* ReadInstruction();
	void PrintMemory();
	void PrintSegment(EnhancedRegister& src);
	void PrintGeneralRegisters();
	void Exit(ErrorCode error);

// asm
private:
	constexpr static const Register s_ComparedResultRegister = Register::AX;
	constexpr static const Register s_SignFlagRegister = Register::SF;
	constexpr static const Register s_ReturnValueRegister = Register::AX;

private:
	void ParseInstruction(Instruction*& instruction);
	void MOV(Register reg, Word value);
	void MOV_OUT(Register reg, Pointer addr);
	void ADD(Register r1, Register r2);
	void SUB(Register r1, Register r2);
	void MUL(Register r1, Register r2);
	void DIV(Register r1, Register r2);
	void MOD(Register r1, Register r2);
	void AND(Register r1, Register r2);
	void OR(Register r1, Register r2);
	void XOR(Register r1, Register r2);
	void NOT(Register reg);
	void CMP(Register r1, Register r2);
	void JMP(Pointer address);
	void JE(Pointer address);
	void JNE(Pointer address);
	void JG(Pointer address);
	void JL(Pointer address);
	void JGE(Pointer address);
	void JLE(Pointer address);
	void CALL(Pointer address);
	void RET(Word value);
	void OUT(Register reg) const;


	struct PrintBytesArgs
	{
		const String description;
		const Byte* data;
		const size_t length;
		const size_t interval = 2;
		const bool endl = true;
		const bool reverse = false;
	};
	void PrintBytes(PrintBytesArgs args) const;

private:
	Byte* m_Memory;
	EnhancedRegister m_SP;						// 堆栈指针
	EnhancedRegister m_IP;						// 指令指针
	EnhancedRegister m_DS;						// 数据段指针
	GeneralRegister m_GeneralRegisters[static_cast<size_t>(Register::REGISTER_SIZE)];
};

