#pragma once

#include "SyntaxParser.h"

#include "Common.h"

class CST {
public:
	friend class SyntaxParser;

	using String = std::string;
	using WString = std::wstring;

	template<class ...Ty>
	using UnorderedMap = std::unordered_map<Ty...>;

	using TokenList = Tokenizer::TokenList;
	using TokenName = Tokenizer::TokenName;
	using SyntaxSequence = SyntaxParser::SyntaxSequence;
	using SyntaxMetaSequence = SyntaxParser::SyntaxMetaSequence;
	using SyntaxTemplateName = SyntaxParser::SyntaxTemplateName;
	using SyntaxTemplateType = SyntaxParser::SyntaxTemplateType;
	using SyntaxMetaSequenceList = SyntaxParser::SyntaxMetaSequenceList;
	using TemplateNameMetaData = SyntaxParser::SyntaxMetaSequence::TemplateNameMetaData;

	using ValueType =
		Variant<
		TokenName,	 		// 特殊符号
		SyntaxTemplateName	// 模板名称
		>;

public:
	// token与模板列表
	struct ParseContext
	{
		SyntaxMetaSequence sequence;
		TokenList tokens;
	};

	struct Node
	{
		//Node() noexcept;

		ValueType value;
		size_t size;
		Node* parent;
		Vector<Node> children;
	};

public:
	CST() noexcept;

	void SetContext(const ParseContext& context);
	void Parse();

	void Print();

private:
	void ReadTokenToPosition(const size_t position);

	// 回溯
	void Backtrack();

	void AddNode(TemplateNameMetaData data);

	void ComsumeWithTemplate(SyntaxMetaSequence sequence);

	void Print(const Node& node, WString prefix = L"", bool isLast = true, bool moe = true) const;

	WString EnumNameWillainVer(const ValueType& value) const;

private:
	Node m_Root;
	Node* m_CurrentNode;

	ParseContext m_Context;

	bool m_ReadTokenCondition;
	size_t m_ReadTokenPosition;

private:
	UnorderedMap<SyntaxTemplateName, std::wstring> templateNameMap = {
		{SyntaxTemplateName::SyntaxParseBegin, L"安安开始读代码啦(๑•̀ㅂ•́)و"},
		{SyntaxTemplateName::SyntaxParseEnd, L"安安读完咯！(≧∇≦)ﾉ♪"},
		{SyntaxTemplateName::FunctionImplStatement, L"函数定义喵awa"},
		{SyntaxTemplateName::Operation, L"操作符来咯→"},
		{SyntaxTemplateName::Empty, L"空空哒>w<"},
		{SyntaxTemplateName::EmptyStatementSequence, L"空语句序列呜0w0"},
		{SyntaxTemplateName::EmptyArgDeclSequence, L"空参数声明唉awa"},
		{SyntaxTemplateName::FunctionCall, L"函数调用呀(๑•̀ㅂ•́)"},
		{SyntaxTemplateName::FunctionReturn, L"返回语句呢owo"},
		{SyntaxTemplateName::BracketExpression, L"括号表达式哇awa"},
		{SyntaxTemplateName::ValueExpression, L"值表达式哦0w0"},
		{SyntaxTemplateName::ExpressionStament, L"表达式语句哟(๑• . •๑)"},
		{SyntaxTemplateName::VariableDeclStatement, L"变量声明啦>w<"},
		{SyntaxTemplateName::VariableImplStatement, L"变量实现咯awa"},
		{SyntaxTemplateName::FunctionDeclStatement, L"函数声明啦0w0"},
		{SyntaxTemplateName::BraceStament, L"大括号语句(´,,•ω•,,)♡"},
		{SyntaxTemplateName::ArgDeclSequence, L"参数声明序列~>w<"},
		{SyntaxTemplateName::ArgDecl, L"参数声明啦awa"},
		{SyntaxTemplateName::ArgSequence, L"参数序列呢0w0"},
		{SyntaxTemplateName::Arg, L"参数呀(๑˃̵ᴗ˂̵)"},
		{SyntaxTemplateName::Statement, L"语句哦owo"},
		{SyntaxTemplateName::StatementSequence, L"语句序列啦awa"},
		{SyntaxTemplateName::TypeValue, L"类型值~>w<"},
		{SyntaxTemplateName::LeftValue, L"左值表达式(՞˶･֊･˶՞)"},
		{SyntaxTemplateName::RightValue, L"右值表达式(๑•́ ₃ •̀๑)"},
		{SyntaxTemplateName::RightValueValue, L"右值内容喵awa"},
		{SyntaxTemplateName::IdentifierValue, L"标识符值~>w<"},
		{SyntaxTemplateName::Identifier, L"标识符呀0w0"},
	};

	UnorderedMap<TokenName, std::wstring> tokenNameMap = {
		{TokenName::LeftBracket, L"左括号(=´ω`=)♪"},
		{TokenName::RightBracket, L"右括号(,,・ω・,,)"},
		{TokenName::LeftBrace, L"左大括号qwq"},
		{TokenName::RightBrace, L"右大括号qwq"},
		{TokenName::Int, L"整数类型喵awa"},
		{TokenName::Identifier, L"标识符呀0w0"},
		{TokenName::Semicolon, L"分号呢(๑•̀ㅂ•́)"},
		{TokenName::Return, L"返回关键字啦awa"},
		{TokenName::Number, L"数字~>w<"},
		{TokenName::String, L"字符串呀0w0"},
		{TokenName::End, L"结束符啦(๑˃̵ᴗ˂̵)"},
		{TokenName::Empty, L"空空哒！"},
	};
};