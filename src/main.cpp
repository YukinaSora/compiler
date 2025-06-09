#include "VirtualMachine.h"
#include "Compiler.h"

#include "Utils.h"

int main(int argc, char** argv, char** envp)
{
	InitConsole();

	std::ios::sync_with_stdio(false);
	std::cin.tie(0);
	std::cout.tie(0);

	const char* source = nullptr;

	if (argc > 1)
	{
		source = argv[1];
	}
	else
	{
		//source = "test/test.cpp";
		source = "test/hello.c";
		//source = "src/Compiler.h";
	}

	Compiler compiler;
	compiler.Compile(source);

	using ConstructArgs = VirtualMachine::Instruction::ConstructArgs;
	using Instruction = VirtualMachine::Instruction;
	using Operation = Instruction::Operation;
	using Register = VirtualMachine::Register;
	VirtualMachine vm;

	//vm.Execute({
	//	Instruction{{ Operation::MOV, Register::CX, 0x00 }},
	//	Instruction{{ Operation::MOV, Register::DX, 0x09 }},

	//	Instruction{{ Operation::MOV, Register::AX, '\033' }},
	//	Instruction{{ Operation::OUT, Register::AX, 0x00 }},
	//	Instruction{{ Operation::MOV, Register::AX, '[' }},
	//	Instruction{{ Operation::OUT, Register::AX, 0x00 }},
	//	Instruction{{ Operation::MOV, Register::AX, '3' }},
	//	Instruction{{ Operation::OUT, Register::AX, 0x00 }},
	//	Instruction{{ Operation::MOV, Register::AX, '2' }},
	//	Instruction{{ Operation::OUT, Register::AX, 0x00 }},
	//	Instruction{{ Operation::MOV, Register::AX, 'm' }},
	//	Instruction{{ Operation::OUT, Register::AX, 0x00 }},

	//	Instruction{{ Operation::MOV, Register::BX, '0' }},
	//	Instruction{{ Operation::ADD, Register::BX, Register::DX }},
	//	Instruction{{ Operation::OUT, Register::BX, 0x00 }},
	//	Instruction{{ Operation::MOV, Register::BX, 0x01 }},
	//	Instruction{{ Operation::SUB, Register::DX, Register::BX }},

	//	Instruction{{ Operation::MOV, Register::AX, ' ' }},
	//	Instruction{{ Operation::OUT, Register::AX, 0x00 }},

	//	Instruction{{ Operation::MOV, Register::BX, 0x01 }},
	//	Instruction{{ Operation::ADD, Register::CX, Register::BX }},
	//	Instruction{{ Operation::MOV, Register::BX, 0x0A }},
	//	Instruction{{ Operation::CMP, Register::CX, Register::BX }},

	//	Instruction{{ Operation::JL, 0x0C, 0x00 }},

	//	Instruction{{ Operation::MOV, Register::AX, '\033' }},
	//	Instruction{{ Operation::OUT, Register::AX, 0x00 }},
	//	Instruction{{ Operation::MOV, Register::AX, '[' }},
	//	Instruction{{ Operation::OUT, Register::AX, 0x00 }},
	//	Instruction{{ Operation::MOV, Register::AX, '0' }},
	//	Instruction{{ Operation::OUT, Register::AX, 0x00 }},
	//	Instruction{{ Operation::MOV, Register::AX, 'm' }},
	//	Instruction{{ Operation::OUT, Register::AX, 0x00 }},
	//	Instruction{{ Operation::MOV, Register::AX, '\n' }},
	//	Instruction{{ Operation::OUT, Register::AX, 0x00 }},

	//	Instruction{{ Operation::RET, 0x00, 0x00 }}
	//});

	return 0;
}