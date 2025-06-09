#pragma once

#include "Common.h"

#include <map>

class Tokenizer
{
public:
	enum class TokenName
	{
		Identifier, Number, String,
		Separator, Semicolon, Space, LeftBracket, RightBracket, LeftBrace, RightBrace, 
				   LeftSquare, RightSquare, DoubleQuote, SingleQuote, Tab, NewLine, Slash, Backslash,
		BasicType, Int, Long, Float, Char, Void, Bool, Const,
		ExtraType, Enum, Struct, Typedef, Class,
		Macro, Define, Include, Pragma,
		Operator, Assign, Add, Sub, Mul, Div, Mod, And, Or, Xor, Not, Les, Ger, Leq, Geq, Equ, Neq, Question, 
				  Comma, Dot, Colon, Tilde, DoubleLeft, DoubleRight, DoubleColon,
		Grammar, Using, Namespace,
		Condition, If, Else, While, For, Do, Switch, Case, Default, Return, Break, Continue,
		End, Empty, Unknown
	};

	enum class TokenType
	{
		Identifier, Number, String, Separator, BasicType, ExtraType, Macro, Operator, Grammar, Condition, Unknown
	};

	enum TokenEstimateState
	{
		Estimating, Matched, Unmatched
	};

	enum DymaticLengthTokenState
	{
		Undymatic, Dymatic, DymaticWaiting, DymaticFinished
	};

public:
	struct Token
	{
		Token()
			: pattern(""), type(TokenName::Unknown), placeholder(TokenName::Unknown)
			, dymaticLengthState(DymaticLengthTokenState::Undymatic) { }

		Token(const char* chars,
			TokenName type,
			TokenName placeholder,
			DymaticLengthTokenState dymaticLengthState
		)
			: pattern(chars), type(type), placeholder(placeholder)
			, dymaticLengthState(dymaticLengthState) { }

		bool operator == (const Token& token) const
		{
			return pattern == token.pattern && type == token.type;
		}

		operator TokenName() const { return type; }

		String pattern;
		TokenName type;
		TokenName placeholder;
		DymaticLengthTokenState dymaticLengthState;
	};


public:
	using RawData = String;
	using TokenList = std::vector<Token>;
	using TokenTypeList = std::vector<TokenName>;

	// type: [first, second)
	using TokenTypeRange = std::pair<TokenName, TokenName>;
	using TokenTypeRangeMap = std::map<TokenName, TokenTypeRange>;

private:
	// 注册Token信息
	const TokenList c_Tokens = {
		{ "[identifier]", TokenName::Identifier, TokenName::Identifier, DymaticLengthTokenState::Dymatic },
		{ "[number]", TokenName::Number, TokenName::Number, DymaticLengthTokenState::Dymatic },
		{ "[string]", TokenName::String, TokenName::String, DymaticLengthTokenState::Dymatic },
		{ ";", TokenName::Semicolon, TokenName::Separator, DymaticLengthTokenState::Undymatic },
		{ " ", TokenName::Space, TokenName::Separator, DymaticLengthTokenState::Undymatic },
		{ "(", TokenName::LeftBracket, TokenName::Separator, DymaticLengthTokenState::Undymatic },
		{ ")", TokenName::RightBracket, TokenName::Separator, DymaticLengthTokenState::Undymatic },
		{ "{", TokenName::LeftBrace, TokenName::Separator, DymaticLengthTokenState::Undymatic },
		{ "}", TokenName::RightBrace, TokenName::Separator, DymaticLengthTokenState::Undymatic },
		{ "[", TokenName::LeftSquare, TokenName::Separator, DymaticLengthTokenState::Undymatic },
		{ "]", TokenName::RightSquare, TokenName::Separator, DymaticLengthTokenState::Undymatic },
		{ "\"", TokenName::DoubleQuote, TokenName::Separator, DymaticLengthTokenState::Undymatic },
		{ "'", TokenName::SingleQuote, TokenName::Separator, DymaticLengthTokenState::Undymatic },
		//{ "\t", TokenName::Space, TokenName::Separator, DymaticLengthTokenState::Undymatic },
		{ "\t", TokenName::Space, TokenName::Separator, DymaticLengthTokenState::Undymatic },// 视作空格
		{ "\n", TokenName::NewLine, TokenName::Separator, DymaticLengthTokenState::Undymatic },
		{ "/", TokenName::Slash, TokenName::Separator, DymaticLengthTokenState::Undymatic },
		{ "\\", TokenName::Backslash, TokenName::Separator, DymaticLengthTokenState::Undymatic },
		{ "int", TokenName::Int, TokenName::BasicType, DymaticLengthTokenState::Undymatic },
		{ "long", TokenName::Long, TokenName::BasicType, DymaticLengthTokenState::Undymatic },
		{ "float", TokenName::Float, TokenName::BasicType, DymaticLengthTokenState::Undymatic },
		{ "char", TokenName::Char, TokenName::BasicType, DymaticLengthTokenState::Undymatic },
		{ "void", TokenName::Void, TokenName::BasicType, DymaticLengthTokenState::Undymatic },
		{ "bool", TokenName::Bool, TokenName::BasicType, DymaticLengthTokenState::Undymatic },
		{ "const", TokenName::Const, TokenName::BasicType, DymaticLengthTokenState::Undymatic },
		{ "enum", TokenName::Enum, TokenName::ExtraType, DymaticLengthTokenState::Undymatic },
		{ "struct", TokenName::Struct, TokenName::ExtraType, DymaticLengthTokenState::Undymatic },
		{ "typedef", TokenName::Typedef, TokenName::ExtraType, DymaticLengthTokenState::Undymatic },
		{ "class", TokenName::Class, TokenName::ExtraType, DymaticLengthTokenState::Undymatic },
		{ "#define", TokenName::Define, TokenName::Macro, DymaticLengthTokenState::Undymatic },
		{ "#include", TokenName::Include, TokenName::Macro, DymaticLengthTokenState::Undymatic },
		{ "#pragma", TokenName::Pragma, TokenName::Macro, DymaticLengthTokenState::Undymatic },
		{ "=", TokenName::Assign, TokenName::Operator, DymaticLengthTokenState::Undymatic },
		{ "+", TokenName::Add, TokenName::Operator, DymaticLengthTokenState::Undymatic },
		{ "-", TokenName::Sub, TokenName::Operator, DymaticLengthTokenState::Undymatic },
		{ "*", TokenName::Mul, TokenName::Operator, DymaticLengthTokenState::Undymatic },
		{ "/", TokenName::Div, TokenName::Operator, DymaticLengthTokenState::Undymatic },
		{ "%", TokenName::Mod, TokenName::Operator, DymaticLengthTokenState::Undymatic },
		{ "&", TokenName::And, TokenName::Operator, DymaticLengthTokenState::Undymatic },
		{ "|", TokenName::Or, TokenName::Operator, DymaticLengthTokenState::Undymatic },
		{ "^", TokenName::Xor, TokenName::Operator, DymaticLengthTokenState::Undymatic },
		{ "!", TokenName::Not, TokenName::Operator, DymaticLengthTokenState::Undymatic },
		{ "<", TokenName::Les, TokenName::Operator, DymaticLengthTokenState::Undymatic },
		{ ">", TokenName::Ger, TokenName::Operator, DymaticLengthTokenState::Undymatic },
		{ "<=", TokenName::Leq, TokenName::Operator, DymaticLengthTokenState::Undymatic },
		{ ">=", TokenName::Geq, TokenName::Operator, DymaticLengthTokenState::Undymatic },
		{ "==", TokenName::Equ, TokenName::Operator, DymaticLengthTokenState::Undymatic },
		{ "!=", TokenName::Neq, TokenName::Operator, DymaticLengthTokenState::Undymatic },
		{ "?", TokenName::Question, TokenName::Operator, DymaticLengthTokenState::Undymatic },
		{ ",", TokenName::Comma, TokenName::Operator, DymaticLengthTokenState::Undymatic },
		{ ".", TokenName::Dot, TokenName::Operator, DymaticLengthTokenState::Undymatic },
		{ ":", TokenName::Colon, TokenName::Operator, DymaticLengthTokenState::Undymatic },
		{ "~", TokenName::Tilde, TokenName::Operator, DymaticLengthTokenState::Undymatic },
		{ "<<", TokenName::DoubleLeft, TokenName::Operator, DymaticLengthTokenState::Undymatic },
		{ ">>", TokenName::DoubleRight, TokenName::Operator, DymaticLengthTokenState::Undymatic },
		{ "::", TokenName::DoubleColon, TokenName::Operator, DymaticLengthTokenState::Undymatic },
		{ "using", TokenName::Using, TokenName::Grammar, DymaticLengthTokenState::Undymatic },
		{ "namespace", TokenName::Namespace, TokenName::Grammar, DymaticLengthTokenState::Undymatic },
		{ "if", TokenName::If, TokenName::Condition, DymaticLengthTokenState::Undymatic },
		{ "else", TokenName::Else, TokenName::Condition, DymaticLengthTokenState::Undymatic },
		{ "while", TokenName::While, TokenName::Condition, DymaticLengthTokenState::Undymatic },
		{ "for", TokenName::For, TokenName::Condition, DymaticLengthTokenState::Undymatic },
		{ "do", TokenName::Do, TokenName::Condition, DymaticLengthTokenState::Undymatic },
		{ "switch", TokenName::Switch, TokenName::Condition, DymaticLengthTokenState::Undymatic },
		{ "case", TokenName::Case, TokenName::Condition, DymaticLengthTokenState::Undymatic },
		{ "default", TokenName::Default, TokenName::Condition, DymaticLengthTokenState::Undymatic },
		{ "return", TokenName::Return, TokenName::Condition, DymaticLengthTokenState::Undymatic },
		{ "break", TokenName::Break, TokenName::Condition, DymaticLengthTokenState::Undymatic },
		{ "continue", TokenName::Continue, TokenName::Condition, DymaticLengthTokenState::Undymatic },
		{ "", TokenName::End, TokenName::End, DymaticLengthTokenState::Undymatic },
		{ "", TokenName::Empty, TokenName::Empty, DymaticLengthTokenState::Undymatic },
	};

	// 匹配优先级
	const TokenTypeList c_PatternPriority = {
		TokenName::BasicType, TokenName::ExtraType
	  , TokenName::Macro, TokenName::Operator, TokenName::Grammar, TokenName::Condition
	  , TokenName::Identifier, TokenName::Number, TokenName::String, TokenName::Separator
	};

	const TokenTypeList c_DymaticLengthTokenTypes = {
		TokenName::Identifier, TokenName::Number, TokenName::String
	};

	const TokenTypeRangeMap c_TypeRangeMap = {
		{ TokenName::Identifier, { TokenName::Identifier, TokenName::Separator  } },
		{ TokenName::Separator,  { TokenName::Separator,  TokenName::BasicType  } },
		{ TokenName::BasicType,  { TokenName::BasicType,  TokenName::ExtraType } },
		{ TokenName::ExtraType, { TokenName::ExtraType, TokenName::Macro	    } },
		{ TokenName::Macro,		 { TokenName::Macro,	  TokenName::Operator   } },
		{ TokenName::Operator,   { TokenName::Operator,   TokenName::Grammar    } },
		{ TokenName::Grammar,    { TokenName::Grammar,    TokenName::Condition  } },
		{ TokenName::Condition,  { TokenName::Condition,  TokenName::End    } }
	};	
	
	const std::map<TokenName, int> c_TokenTypeColors = {
		{ TokenName::Macro, 35 },
		{ TokenName::ExtraType, 36 },
		{ TokenName::BasicType, 36 },
		{ TokenName::Operator, 37 },
		{ TokenName::Separator, 37 },
		{ TokenName::Identifier, 33 },
		{ TokenName::Number, 32 },
		{ TokenName::String, 33 }
	};

public:
	Tokenizer();
	~Tokenizer();

	void Load(const RawData data);
	void Tokenize();

	const TokenList& Tokens() const { return m_Tokens; }

private:
	void ReadNextChar();
	TokenEstimateState TryPatterningTokens(const Token& token);
	bool TryPatterningIdentifier();
	bool TryPatterningNumber();
	bool TryPatterningString();
	void RemoveUnpatternedTokens();
	void HandleMatchedTokens();
	bool HandlePriorityTokens();	// return: 函数中是否Push了Token
	void Push(const Token& token);
	void ResetEstimatingTypes();

private:
	RawData m_RawData;
	size_t m_ReadPosition;
	size_t m_TokenStartPosition;
	size_t m_LastMatchedPosition;
	String m_ReadBuffer;
	TokenList m_Tokens;
	TokenList m_TokensToBeRemovedLater;

	// 目前有可能匹配到的Token
	TokenList m_EstimatingTokens;
	TokenList m_EstimatedTokens;
};