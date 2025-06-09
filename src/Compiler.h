#pragma once

#include "Tokenizer.h"
#include "SyntaxParser.h"

class Compiler
{
	friend class CST;

private:
	using Type = ConstexprString;

public:
	Compiler();
	~Compiler();

	void Compile(String source);

private:
	Tokenizer m_Tokenizer;
	SyntaxParser m_SyntaxParser;
};

