#pragma once

#include "Common.h"
#include "Uilts.h"

#include <map>

class Tokenizer_deprecated
{
	enum class TokenType;

public:
	struct Token: public String {
		TokenType type;

		Token() : String(), type(TokenType::UNKNOWN) {}
		Token(const char* const str) : String(str), type(TokenType::UNKNOWN) {}
		void operator = (const char* const str) { String::operator=(str); }
		void operator = (const char ch) { String::operator=(ch); }
	};

private:
	enum class TokenType
	{
		BASE_TYPE, INT, FLOAT, DOUBLE, CHAR, VOID, BOOL,
		EXTRA_TYPE, ENUM, CLASS, STRUCT, UNION, TYPEDEF,
		MACRO, DEFINE, INCLUDE, IFDEF, IFNDEF, ENDIF, UNDEF, ELIF, ERROR, PRAGMA,
		CONDITION, BREAK, CASE, CONST, CONTINUE, DEFAULT, DO, IF, ELSE, SWITCH, WHILE, FOR, RETURN, GOTO,
		VALUE, TRUE, FALSE, NULLPTR,
		OPERATOR, ASSIGN, ADD, SUB, MUL, DIV, MOD, AND, OR, XOR, NOT, LESS, GREATER, QUESTION,
		SEPARATOR, PARENTHESIS, BRACE, BRACKET,
		IDENTIFIER, STRING, NUMBER, COMMENT,
		UNKNOWN
	};

private:
	//using Token = String;
	using TokenList = Vector<Token>;
	using TokenTypeList = Vector<TokenType>;

private:
	const std::map<TokenType, int> s_TokenTypeColors = {
		{ TokenType::MACRO, 35 },
		{ TokenType::EXTRA_TYPE, 36 },
		{ TokenType::CONDITION, 35 },
		{ TokenType::BASE_TYPE, 36 },
		{ TokenType::OPERATOR, 37 },
		{ TokenType::SEPARATOR, 37 },
		{ TokenType::IDENTIFIER, 33 },
		{ TokenType::NUMBER, 32 },
		{ TokenType::COMMENT, 32 },
		{ TokenType::STRING, 33 },
		{ TokenType::UNKNOWN, 31 }
	};

	// 用于分隔不同语法块的字符
	constexpr static const char* s_TokenChars[] = {
		" ", "\t", "\n", "\r", "(", ")", "{", "}", "[", "]", 
		";", ",", ".", ":", "=", "+", "-", "*", "/", "%", 
		"&", "|", "^", "~", "!", "<", ">", "?", "#", "@", 
		"$", "'", "\"", "\\", " ", "\t", "\n", "\r"
	};
	constexpr static size_t s_TokenCharCount = ArrayLength(s_TokenChars);

	constexpr static const char* s_Macros[] = {
		"define", "include", "ifdef", "ifndef", "endif", "undef", "if", "else", "elif", "error", "pragma", "#"
	};
	constexpr static size_t s_MacroCount = ArrayLength(s_Macros);
	constexpr static const char* s_ExtraTypes[] = {
		"namespace", "using", "class", "struct", "union", "enum", "typedef", "template"
	};
	constexpr static size_t s_ExtraTypeCount = ArrayLength(s_ExtraTypes);
	constexpr static const char* s_Conditions[] = {
		"break", "case", "const", "continue", "default", "do", "if", "else", "return", "switch", "while", "for", "goto"
	};
	constexpr static size_t s_ConditionCount = ArrayLength(s_Conditions);
	constexpr static const char* s_BaseTypes[] = {
		"int", "float", "double", "char", "void", "bool"
	};
	constexpr static size_t s_BaseTypeCount = ArrayLength(s_BaseTypes);
	constexpr static const char* s_Operators[] = {
		"=", "+", "-", "*", "/", "%", "&", "|", "^", "~", "!", "<", ">", "?", "@", "$"
	};
	constexpr static size_t s_OperatorCount = ArrayLength(s_Operators);
	constexpr static const char* s_Separators[] = {
		"(", ")", "{", "}", "[", "]", ";", ",", ".", ":", "'", "\"", "\\", " ", "\t", "\n", "\r"
	};
	constexpr static size_t s_SeparatorCount = ArrayLength(s_Separators);

	static TokenList s_TokenCharList;

public:
	Tokenizer_deprecated();
	~Tokenizer_deprecated();

	void Load(String rawData);

	void Tokenize_old();
	String Next();

private:
	void Push(Token& token);
	bool Classify(const Token& token);

private:
	int m_TokenCount;
	String m_RawData;
	TokenList m_Tokens;
	TokenTypeList m_TokenTypes;
	size_t m_Position;

	TokenType m_LastTokenType;
};

