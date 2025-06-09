#include "Compiler.h"

#include <fstream>

Compiler::Compiler()
{

}

Compiler::~Compiler()
{

}

void Compiler::Compile(String source)
{
	using std::ifstream;
	using std::ios;

	cout << "Compiling " << source << endl;

	ifstream file(source, ios::in);
	if (!file.is_open())
	{
		cout << "Failed to open file: " << source << endl;
		return;
	}

	String data;
	String buffer;
	while (std::getline(file, buffer))
	{
		data += buffer;
	}

	file.close();

	m_Tokenizer.Load(data);
	m_Tokenizer.Tokenize();

	cout << '\n';

	auto tokens = m_Tokenizer.Tokens();
	m_SyntaxParser.Load(tokens);
	m_SyntaxParser.Parse();
}