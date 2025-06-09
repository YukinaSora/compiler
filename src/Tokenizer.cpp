#include "Tokenizer.h"

#include "enum_stringify.h"

#include "Utils.h"


using YukinaSora::enum_name;

Tokenizer::Tokenizer()
{
	m_ReadPosition = 0;
	m_TokenStartPosition = 0;
	m_LastMatchedPosition = 0;
}

Tokenizer::~Tokenizer()
{

}

void Tokenizer::Load(const RawData data)
{
	m_RawData = data;
	m_ReadPosition = 0;
	m_TokenStartPosition = 0;
	m_LastMatchedPosition = 0;
	m_ReadBuffer.clear();
	ResetEstimatingTypes();
}

void Tokenizer::Tokenize()
{
	cout << "Tokenizing...\n\n";

	// bfs
	while(m_ReadPosition < m_RawData.size())
	{
		ReadNextChar();

		for (Token& token : m_EstimatingTokens)
		{
			auto state = TryPatterningTokens(token);
			switch (state)
			{
			case Tokenizer::Estimating:
				//cout << "Estimating token: (" << enum_name(token.type) << "): ""\033[4m" << token.pattern << "\033[0m" << endl;
				break;

			case Tokenizer::Matched:
				//cout  << "Matched token: \033[4;32m" << token.pattern << "\033[0m (" << enum_name(token.type) << ")" << "\n";

				m_LastMatchedPosition = m_ReadPosition;

				if (!ArrayContains(m_EstimatedTokens, token))
					m_EstimatedTokens.push_back(token);

				if (token.dymaticLengthState == DymaticLengthTokenState::Dymatic)
					token.dymaticLengthState =  DymaticLengthTokenState::DymaticWaiting;

				break; // case Tokenizer::Matched

			case Tokenizer::Unmatched:
				//cout << "Unmatched token: ""(" << enum_name(token.type) << "): \033[4m"  << token.pattern << "\033[0m"<< endl;

				switch (token.dymaticLengthState)
				{
				case DymaticLengthTokenState::DymaticWaiting:
					for (auto& estimatedToken : m_EstimatedTokens)
					{
						if (estimatedToken.type != token.type)
							continue;

						estimatedToken.dymaticLengthState = DymaticLengthTokenState::DymaticFinished;
						break;
					}
					break;

				default:
					m_TokensToBeRemovedLater.push_back(token);
					break;
				}

				break; // case Tokenizer::Unmatched

			default:
				break;
			}
		}
		RemoveUnpatternedTokens();

		for (auto& token : m_EstimatingTokens)
			cout << "Expecting: ""\033[4;33m" << token.pattern << "\033[0m (" << enum_name(token.type) << ")\n";

		HandleMatchedTokens();
	}

	// 添加 End Token
	for (auto& token : m_EstimatingTokens)
	{
		if (token.type == TokenName::End)
		{
			Push(token);
			break;
		}
	}

	// 输出
	cout << "------------------------------------\n";
	cout << "Tokenized results:\n";
	for(auto& token : m_Tokens)
	{
		int color = 0;
		switch (token.placeholder)
		{
		case TokenName::Identifier:
			color = 33;
			break;

		case TokenName::Number:
			color = 32;
			break;

		case TokenName::String:
			color = 33;
			break;

		case TokenName::Macro:
			color = 35;
			break;

		case TokenName::ExtraType:
			color = 36;
			break;

		case TokenName::BasicType:
			color = 36;
			break;

		case TokenName::Operator:
			color = 37;
			break;

		case TokenName::Separator:
			color = 37;
			break;

		case TokenName::Grammar:
			color = 36;
			break;

		case TokenName::Condition:
			color = 36;
			break;

		default:
			break;
		}
		std::cout << "\033[" << std::to_string(color) << "m" << token.pattern << "\033[0m ";
	}
	cout << endl;
}

void Tokenizer::ReadNextChar()
{
	if (m_ReadPosition < m_RawData.size())
	{
		cout << "------------------------------------\n";
		cout << "Estimating: ";
		char ch[2] = { '\0', '\0' };
		ch[0] = m_RawData[m_ReadPosition++];
		m_ReadBuffer += ch;
		cout << "\033[36m" << m_ReadBuffer.substr(0, m_ReadBuffer.size() - 1) 
			 << "\033[32m" << m_ReadBuffer[m_ReadBuffer.size() - 1] << "\033[0m\n";

		String space = "           "; // length of "Estimating: " - 1
		size_t offset = m_ReadPosition - m_TokenStartPosition;
		for (int i = 0; i < offset; i++)
			space += " ";
		cout << space << "^\n";
	}
}

void Tokenizer::RemoveUnpatternedTokens()
{
	auto it = std::remove_if(m_EstimatingTokens.begin(), m_EstimatingTokens.end(), [this](const Token& token) {
		return std::find(m_TokensToBeRemovedLater.begin(), m_TokensToBeRemovedLater.end(), token) != m_TokensToBeRemovedLater.end();
	});

	m_EstimatingTokens.erase(it, m_EstimatingTokens.end());
	m_TokensToBeRemovedLater.clear();
}

void Tokenizer::HandleMatchedTokens()
{
	if (m_EstimatedTokens.empty())
		return;

	if (m_EstimatingTokens.size() - m_TokensToBeRemovedLater.size() > m_EstimatedTokens.size())
		return;

	// 匹配到多个Token时，优先匹配基本类型、关键词
	if (HandlePriorityTokens())
		return;

	Token& token = m_EstimatedTokens.front();
	switch (token.dymaticLengthState)
	{
	case DymaticLengthTokenState::Undymatic:
	case DymaticLengthTokenState::DymaticFinished:
		Push(token);
		break;

	case DymaticLengthTokenState::Dymatic:
	case DymaticLengthTokenState::DymaticWaiting:
		break;
	}
}

bool Tokenizer::HandlePriorityTokens()
{
	if (m_EstimatedTokens.size() <= 1)
		return false;

	for (auto& priorityToken : c_PatternPriority)
	{
		for (auto& token : m_EstimatedTokens)
		{
			if (token.placeholder != priorityToken)
				continue;

			// 等待动态长度Token
			if(token.dymaticLengthState == DymaticLengthTokenState::DymaticWaiting ||
			   token.dymaticLengthState == DymaticLengthTokenState::Dymatic)
				return true;

			// 非动态长度Token仅允许完全匹配
			else if (token.pattern != m_ReadBuffer)
				continue;


			Push(token);

			return true;
		}
	}
	return false;
}

Tokenizer::TokenEstimateState Tokenizer::TryPatterningTokens(const Token& token)
{
	TokenEstimateState state = TokenEstimateState::Unmatched;
	switch (token.type)
	{
	case (TokenName::Identifier):
		return TryPatterningIdentifier() ? TokenEstimateState::Matched : TokenEstimateState::Unmatched;

	case (TokenName::Number):
		return TryPatterningNumber() ? TokenEstimateState::Matched : TokenEstimateState::Unmatched;

	case (TokenName::String):
		return TryPatterningString() ? TokenEstimateState::Matched : TokenEstimateState::Unmatched;

	default:
		break;
	}

	// len(buffer) > len(pattern)
	if (m_ReadBuffer.size() > token.pattern.size())
		return TokenEstimateState::Unmatched;

	// len(buffer) <= len(pattern)
	String patternBuffer = token.pattern.substr(0, m_ReadBuffer.size());
	if (m_ReadBuffer == patternBuffer)
	{
		if (m_ReadBuffer.size() == token.pattern.size())
			return TokenEstimateState::Matched;

		else 
			return TokenEstimateState::Estimating;
	}
	return TokenEstimateState::Unmatched;
}

bool Tokenizer::TryPatterningIdentifier()
{
	if (isdigit(m_ReadBuffer[0]))
		return false;

	char lastChar = m_ReadBuffer[m_ReadBuffer.size() - 1];
	return isalnum(lastChar) || lastChar == '_';
}

bool Tokenizer::TryPatterningNumber()
{
	auto lastChar = m_ReadBuffer[m_ReadBuffer.size() - 1];
	return isdigit(lastChar) 
		// || lastChar == '.' // floating point number disabled
		;
}

bool Tokenizer::TryPatterningString()
{
	if (m_ReadBuffer.size() == 1)
	{
		return m_ReadBuffer[0] == '\"';
	}
	else
	{
		size_t lastPosition = m_ReadBuffer.find_last_of('\"');
		return lastPosition == m_ReadBuffer.size() - 1 || lastPosition == 0;
	}
}

void Tokenizer::Push(const Token& token)
{
	Token pushToken;
	size_t offset;

	// 空格不作为Token
	if (TokenName::Space == token.type)
		goto end;

//push:
	pushToken = token;
	offset = m_ReadPosition - m_LastMatchedPosition;
	pushToken.pattern = m_ReadBuffer.substr(0, m_ReadBuffer.size() - offset);
	m_ReadPosition = m_LastMatchedPosition;

	cout << "Pushing token: \033[4;32m" << pushToken.pattern << "\033[0m (" << enum_name(pushToken.type) << ") \n";
	m_Tokens.push_back(pushToken);

end:
	m_ReadBuffer.clear();
	//m_ReadPosition -= 1;

	m_TokenStartPosition = m_ReadPosition;

	ResetEstimatingTypes();
}

void Tokenizer::ResetEstimatingTypes()
{
	m_TokensToBeRemovedLater.clear();
	m_EstimatedTokens.clear();
	m_EstimatingTokens.clear();
	m_EstimatingTokens = c_Tokens;
}