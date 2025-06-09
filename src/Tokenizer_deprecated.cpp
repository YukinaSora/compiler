#include "Tokenizer_deprecated.h"

#include <algorithm>
#include <cctype>

//#define ENUM_STRINGIFY_DISABLE_NAMESPACE
#include "enum_stringify.h"

Vector<Tokenizer_deprecated::Token> Tokenizer_deprecated::s_TokenCharList;

Tokenizer_deprecated::Tokenizer_deprecated()
{
	m_TokenCount = 0;
	m_Position = 0;

	s_TokenCharList.clear();
	for (auto i = 0; i < s_TokenCharCount; i++)
	{
		s_TokenCharList.push_back(Token(s_TokenChars[i]));
	}
}

Tokenizer_deprecated::~Tokenizer_deprecated()
{

}

void Tokenizer_deprecated::Load(String rawData)
{
	m_RawData = rawData;
	m_Position = 0;
	m_TokenTypes.clear();
	cout << "Loaded data: " << m_RawData << endl;
}

[[deprecated("This function is deprecated, use Tokenize() instead.")]]
void Tokenizer_deprecated::Tokenize_old()
{
	m_LastTokenType = TokenType::UNKNOWN;
	Token token;
	bool isConstString = false;
	bool isNumber = false;
	char lastConstStringBeginChar = '\0';
	char stringBuffer[2] = { '\0', '\0'};
	cout << "Tokenizing data..." << endl;
	for (const char ch : m_RawData)
	{
		//cout << ch << " " <<endl;
		stringBuffer[0] = ch;
		bool isTokenChar = false;

		if (isConstString) {
			if (ch == lastConstStringBeginChar) {
				isConstString = false;
				m_LastTokenType = TokenType::STRING;
				Push(token);
				token = stringBuffer;
				continue;
			}
			token += stringBuffer;
			continue;
		}

		if (!isNumber) {
			if (!token.size() && isdigit(ch)) {
				isNumber = true;
				token += stringBuffer;
				continue;
			}
		}
		else {
			if (isdigit(ch) || ch == '.') {
				token += stringBuffer;
				continue;
			}
			else {
				isNumber = false;
				m_LastTokenType = TokenType::NUMBER;
				Push(token);
				token = stringBuffer;
				continue;
			}
		}

		for (auto& tokenChar : s_TokenCharList){
			if (tokenChar != stringBuffer) continue;
			isTokenChar = true;
			break;
		}

		if (!isTokenChar) {
			if (m_LastTokenType == TokenType::NUMBER) {
				Push(token);
				token = stringBuffer;
			}
			else if (token == "<" || token == ">") {
				Push(token);
				token = stringBuffer;
			}
			else {
				token += stringBuffer;
			}
			continue;
		}
		else {
			if (ch == '<' || ch == '>') {
				if (token == stringBuffer){
					token += stringBuffer;
					m_LastTokenType = TokenType::OPERATOR;
					Push(token);
					token.clear();
				}
				else {
					Push(token);
					token = stringBuffer;
				}
				continue;
			}
			if (ch == '"' || ch == '\'') {
				isConstString = !isConstString;
				lastConstStringBeginChar = ch;
			}
			Push(token);
			if (ch != ' ' && ch != '\t' && ch != '\n') {
				token = ch;
				if (m_Tokens.size() && m_Tokens[m_Tokens.size() - 1] == " ") {}
				else Push(token);
			}
			token.clear();
			continue;
		}
	}
	cout << endl;

	m_Position = 0;

	cout << "Tokenized data: ";
	size_t i = 0;
	for (const auto& token : m_Tokens)
		cout 
			//<< "[" << i++ << "]" 
			//<< "(" << i++ << ")["
			<< "\033[" << std::to_string(s_TokenTypeColors.at(m_TokenTypes[i++])) << "m"
			<< token 
			<< "\033[0m"
			<< " ";
	cout << endl;
}

String Tokenizer_deprecated::Next()
{
	// Return the next token
	return m_Tokens[m_Position++];
}

void Tokenizer_deprecated::Push(Token& token)
{
	bool accepted = Classify(token);
	if (!accepted) {
		token.clear();
		return;
	}
	m_Tokens.push_back(token);
	token.clear();
}

bool Tokenizer_deprecated::Classify(const Token& token)
{
	if (token.size() == 0) return false;
	//cout << "Token: " << token << endl;

	if (m_LastTokenType != TokenType::UNKNOWN) {
		cout << YukinaSora::enum_name(m_LastTokenType)
			 << ": \033[" << std::to_string(s_TokenTypeColors.at(m_LastTokenType)) << "m   \t" 
			 << token << "\033[0m" << endl;
		m_TokenTypes.push_back(m_LastTokenType);
		m_LastTokenType = TokenType::UNKNOWN;
		return true;
	}

	static auto keywords = {
		s_Macros, s_ExtraTypes, s_Conditions, s_BaseTypes, s_Operators, s_Separators
	};
	static size_t keywordCounts[] = {
		s_MacroCount, s_ExtraTypeCount, s_ConditionCount, s_BaseTypeCount, s_OperatorCount, s_SeparatorCount
	};
	static TokenType keywordTypes[] = {
		TokenType::MACRO, TokenType::EXTRA_TYPE, TokenType::CONDITION, TokenType::BASE_TYPE, TokenType::OPERATOR, TokenType::SEPARATOR
	};
	size_t i = 0;
	for (auto& keyword : keywords) {
		//cout << "Comparing token \033[4;32m" << token << "\033[0m with keyword list " << YukinaSora::enum_name(keywordTypes[i]) << endl;
		if (ArrayContains(keyword, keywordCounts[i], token)) {
			cout << YukinaSora::enum_name(keywordTypes[i]) 
				 << ": \033[" << std::to_string(s_TokenTypeColors.at(keywordTypes[i])) << "m   \t"
				 << token << "\033[0m" << endl;
			m_TokenTypes.push_back(keywordTypes[i]);
			return true;
		}
		i++;
	}
	cout << YukinaSora::enum_name(TokenType::IDENTIFIER)
		<< ": \033[" << std::to_string(s_TokenTypeColors.at(TokenType::IDENTIFIER)) << "m   \t"
		<< token << "\033[0m" << endl;
	m_TokenTypes.push_back(TokenType::IDENTIFIER);
	return true;
}