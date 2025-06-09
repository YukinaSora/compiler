#pragma once

#include "../Common.h"

#include "../SyntaxParser.h"
#include "../Tokenizer.h"

class MetaObject
{
public:
	using TokenName = Tokenizer::TokenName;
	using SyntaxTemplateName = SyntaxParser::SyntaxTemplateName;
};