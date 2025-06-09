#include "VirtualMachine.h"

#include <iomanip>

#include "enum_stringify.h"

using Byte = VirtualMachine::Byte;
using Instruction = VirtualMachine::Instruction;
using Operation = Instruction::Operation;

using std::hex;
using std::dec;
using std::setw;
using std::setfill;

using YukinaSora::enum_name;

VirtualMachine::VirtualMachine()
{
	m_Memory = new Byte[s_MemorySize];

	m_SP.size = s_StackSize;
	m_IP.size = s_InstructionSize;
	m_DS.size = s_DataSize;

	ResetMemory();
}

VirtualMachine::~VirtualMachine()
{
	delete[] m_Memory;
}

void VirtualMachine::Execute(InstructionList instructions)
{
	for (auto& instruction : instructions)
		WriteMemory(m_IP, (Byte*) & instruction, Instruction::InstructionSize);

	PrintMemory();

	m_IP.value = m_IP.base;
	while (m_GeneralRegisters[static_cast<size_t>(s_SignFlagRegister)] == Flag::ZF)
	{
		//cout << "Reading instruction at (0x" << setw(8) << setfill('0') << hex << m_IP.value << "): ";
		Instruction* instruction = ReadInstruction();

		//cout << "operation: " << enum_name(instruction->operation) 
			 //<< ", args: " << dec << instruction->args[0] << ", " << instruction->args[1] << "\n";

		ParseInstruction(instruction);

		//cout << "----------------------------------------\n";
		//PrintGeneralRegisters();
		//cout << "----------------------------------------\n";
	}
	cout << "VM Exit with code: " << m_GeneralRegisters[static_cast<size_t>(s_ReturnValueRegister)] << "\n";

	cout << "\n";
	PrintMemory();
}

void VirtualMachine::ResetMemory()
{
	memset(m_Memory, 0, s_MemorySize);

	m_SP.Init( s_MemorySize - s_StackSize);
	m_IP.Init( m_SP.base - s_InstructionSize );
	m_DS.Init( 0 );
}

bool VirtualMachine::WriteMemory(EnhancedRegister& src, Byte* data, size_t length)
{
	if (!src.IsInRange(length))
	{
		Exit(ErrorCode::INVALID_ACCESS);
		return false;
	}

	bool ret = WriteMemory(src.value, data, length);
	src.value += length;

	return ret;
}

bool VirtualMachine::WriteMemory(Pointer address, Byte* data, size_t length)
{
	if (address < 0 || address >= s_MemorySize)
	{
		Exit(ErrorCode::INVALID_ACCESS);
		return false;
	}

	memcpy(&m_Memory[address], data, length);

	cout << "Write [";
	PrintBytes({ .description = "", .data = (Byte*)&address, .length = 4,
				 .interval = 0, .endl = false, .reverse = true });
	cout << " - ";
	address += length;
	PrintBytes({ .description = "", .data = (Byte*)&address, .length = 4,
				 .interval = 0, .endl = false, .reverse = true });
	cout << "]: ";
	PrintBytes({ .description = "", .data = data, .length = length, .interval = 2 });

	return true;
}

Byte* VirtualMachine::ReadMemory(EnhancedRegister& src, size_t length)
{
	// unsigned long long > 0
	if (src.value + length > s_MemorySize)
	{
		cout << "Error in reading (0x" << setw(8) << setfill('0') << hex << src.value << ", length: " << dec << length << "), "
			 << "at (0x" << setw(8) << setfill('0') << hex << m_IP.value - Instruction::InstructionSize << "): ";
		Exit(ErrorCode::INVALID_ACCESS);
		return nullptr;
	}

	Byte* data = m_Memory + (size_t)src.value;
	src.value += length;

	return data;
}

Instruction* VirtualMachine::ReadInstruction()
{
	Byte* data = ReadMemory(m_IP, Instruction::InstructionSize);
	return (Instruction*)data;
}

void VirtualMachine::PrintMemory()
{
	cout << "Memory (" << dec << s_MemorySize << " bytes):\n";
	cout << "----------------------------------------\n";
	cout << "Data Segment (" << dec << m_DS.size << " bytes):\n\033[32m";
	PrintSegment(m_DS);
	cout << "\033[0m";
	cout << "----------------------------------------\n";
	cout << "Instruction Segment (" << dec << m_IP.size << " bytes):\n\033[36m";
	PrintSegment(m_IP);
	cout << "\033[0m";
	cout << "----------------------------------------\n";
	cout << "Stack Segment (" << dec << m_SP.size << " bytes):\n\033[34m";
	PrintSegment(m_SP);
	cout << "\033[0m";
	cout << "----------------------------------------\n";
	PrintGeneralRegisters();
	cout << "----------------------------------------\n";
}

void VirtualMachine::PrintSegment(EnhancedRegister& src)
{
	constexpr size_t lineBlocks = 4;		  // 8 bytes per block
	constexpr size_t addressLineInterval = 1; // line interval

	constexpr size_t lineWidth = lineBlocks * 8;

	size_t offset = 0;
	for (size_t i = 0; i < src.size; i += 1)
	{
		if (i % lineWidth * (addressLineInterval + 1) == 0)
		{
			cout << "[" << hex << setw(8) << src.base + offset << "]: ";
		}
		else if (offset % lineWidth == 0)
			cout << "           ";

		PrintBytes({ .description = "", .data = m_Memory + src.base + i, .length = 1, .interval = 2, .endl = false });
		offset++;

		// line interval
		if (offset % lineWidth == 0)
			cout << "\n";
		// byte interval
		else if (offset % 8 == 0)
			cout << "- ";
	}
}

void VirtualMachine::PrintGeneralRegisters()
{
	cout << "General Registers:\n";
	for (size_t i = 0; i < static_cast<size_t>(Register::REGISTER_SIZE); i++)
	{
		cout << "  " << enum_name(static_cast<Register>(i)) << ": " << m_GeneralRegisters[i] << "\n";
	}
}

void VirtualMachine::PrintBytes(PrintBytesArgs args) const
{
	if (!args.description.empty())
		cout << args.description;

	const Byte* data = args.data;
	Byte* reversedData = new Byte[args.length];
	if (args.reverse)
	{
		for (size_t i = 0; i < args.length; i++)
			reversedData[i] = data[args.length - i - 1];

		data = reversedData;
	}

	size_t count = 0;
	//for (int i = args.length - 1; i >= 0; i--)
	for (int i = 0; i < args.length; i++)
	{
		cout << hex << setw(2) << setfill('0') << (int)data[i];

		count += 2;
		if (args.interval && count == args.interval)
		{
			cout << " ";
			count = 0;
		}
	}

	if (args.endl)
		cout << "\n";

	delete reversedData;
}

void VirtualMachine::Exit(ErrorCode error)
{
	switch (error)
	{
	case ErrorCode::INVALID_ACCESS:
		cout << "Invalid memory access.\n";
		break;

	default:
		break;
	}

	exit(0);
}

void VirtualMachine::ParseInstruction(Instruction*& instruction)
{
	switch (instruction->operation)
	{
	case Operation::MOV:
		MOV(static_cast<Register>(instruction->args[0]), instruction->args[1]);
		break;

	case Operation::MOV_OUT:
		MOV_OUT(static_cast<Register>(instruction->args[0]), instruction->args[1]);
		break;

	case Operation::ADD:
		ADD(static_cast<Register>(instruction->args[0]), static_cast<Register>(instruction->args[1]));
		break;

	case Operation::SUB:
		SUB(static_cast<Register>(instruction->args[0]), static_cast<Register>(instruction->args[1]));
		break;

	case Operation::MUL:
		MUL(static_cast<Register>(instruction->args[0]), static_cast<Register>(instruction->args[1]));
		break;

	case Operation::DIV:
		DIV(static_cast<Register>(instruction->args[0]), static_cast<Register>(instruction->args[1]));
		break;

	case Operation::MOD:
		MOD(static_cast<Register>(instruction->args[0]), static_cast<Register>(instruction->args[1]));
		break;

	case Operation::AND:
		AND(static_cast<Register>(instruction->args[0]), static_cast<Register>(instruction->args[1]));
		break;

	case Operation::OR:
		OR(static_cast<Register>(instruction->args[0]), static_cast<Register>(instruction->args[1]));
		break;

	case Operation::XOR:
		XOR(static_cast<Register>(instruction->args[0]), static_cast<Register>(instruction->args[1]));
		break;

	case Operation::NOT:
		NOT(static_cast<Register>(instruction->args[0]));
		break;

	case Operation::CMP:
		CMP(static_cast<Register>(instruction->args[0]), static_cast<Register>(instruction->args[1]));
		break;

	case Operation::JMP:
		JMP(instruction->args[0]);
		break;

	case Operation::JE:
		JE(instruction->args[0]);
		break;

	case Operation::JNE:
		JNE(instruction->args[0]);
		break;

	case Operation::JG:
		JG(instruction->args[0]);
		break;

	case Operation::JL:
		JL(instruction->args[0]);
		break;

	case Operation::JGE:
		JGE(instruction->args[0]);
		break;

	case Operation::JLE:
		JLE(instruction->args[0]);
		break;

	case Operation::CALL:
		CALL(instruction->args[0]);
		break;

	case Operation::RET:
		RET(instruction->args[0]);
		break;

	case Operation::OUT:
		OUT(static_cast<Register>(instruction->args[0]));
		break;
	}
}

void VirtualMachine::MOV(Register reg, Word value)
{
	m_GeneralRegisters[static_cast<size_t>(reg)] = value;
}

void VirtualMachine::MOV_OUT(Register reg, Pointer address)
{
	WriteMemory(address, (Byte*)&m_GeneralRegisters[static_cast<size_t>(reg)], sizeof(Word));	
}

void VirtualMachine::ADD(Register r1, Register r2)
{
	m_GeneralRegisters[static_cast<size_t>(r1)] += m_GeneralRegisters[static_cast<size_t>(r2)];
}

void VirtualMachine::SUB(Register r1, Register r2)
{
	m_GeneralRegisters[static_cast<size_t>(r1)] -= m_GeneralRegisters[static_cast<size_t>(r2)];
}

void VirtualMachine::MUL(Register r1, Register r2)
{
	m_GeneralRegisters[static_cast<size_t>(r1)] *= m_GeneralRegisters[static_cast<size_t>(r2)];
}

void VirtualMachine::DIV(Register r1, Register r2)
{
	m_GeneralRegisters[static_cast<size_t>(r1)] /= m_GeneralRegisters[static_cast<size_t>(r2)];
}

void VirtualMachine::MOD(Register r1, Register r2)
{
	m_GeneralRegisters[static_cast<size_t>(r1)] %= m_GeneralRegisters[static_cast<size_t>(r2)];
}

void VirtualMachine::AND(Register r1, Register r2)
{
	m_GeneralRegisters[static_cast<size_t>(r1)] &= m_GeneralRegisters[static_cast<size_t>(r2)];
}

void VirtualMachine::OR(Register r1, Register r2)
{
	m_GeneralRegisters[static_cast<size_t>(r1)] |= m_GeneralRegisters[static_cast<size_t>(r2)];
}

void VirtualMachine::XOR(Register r1, Register r2)
{
	m_GeneralRegisters[static_cast<size_t>(r1)] ^= m_GeneralRegisters[static_cast<size_t>(r2)];
}

void VirtualMachine::NOT(Register reg)
{
	m_GeneralRegisters[static_cast<size_t>(reg)] = ~m_GeneralRegisters[static_cast<size_t>(reg)];
}

void VirtualMachine::CMP(Register r1, Register r2)
{
	m_GeneralRegisters[static_cast<size_t>(s_ComparedResultRegister)] = m_GeneralRegisters[static_cast<size_t>(r1)] - m_GeneralRegisters[static_cast<size_t>(r2)];
}

void VirtualMachine::JMP(Pointer address)
{
	m_IP.value = m_IP.base + address * Instruction::InstructionSize;
}

void VirtualMachine::JE(Pointer address)
{
	if (m_GeneralRegisters[static_cast<size_t>(s_ComparedResultRegister)] == 0)
		JMP(address);
}

void VirtualMachine::JNE(Pointer address)
{
	if (m_GeneralRegisters[static_cast<size_t>(s_ComparedResultRegister)] != 0)
		JMP(address);
}

void VirtualMachine::JG(Pointer address)
{
	if (m_GeneralRegisters[static_cast<size_t>(s_ComparedResultRegister)] > 0)
		JMP(address);
}

void VirtualMachine::JL(Pointer address)
{
	if (m_GeneralRegisters[static_cast<size_t>(s_ComparedResultRegister)] > 32767)
		JMP(address);
}

void VirtualMachine::JGE(Pointer address)
{
	if (m_GeneralRegisters[static_cast<size_t>(s_ComparedResultRegister)] >= 0)
		JMP(address);
}

void VirtualMachine::JLE(Pointer address)
{
	if (m_GeneralRegisters[static_cast<size_t>(s_ComparedResultRegister)] >= 32767)
		JMP(address);
}

void VirtualMachine::CALL(Pointer address)
{

}

void VirtualMachine::RET(Word value)
{
	m_GeneralRegisters[static_cast<size_t>(s_ReturnValueRegister)] = value;
	m_GeneralRegisters[static_cast<size_t>(s_SignFlagRegister)] = Flag::EXT;
}

void VirtualMachine::OUT(Register reg) const
{
	cout << (char)m_GeneralRegisters[static_cast<size_t>(reg)];
}