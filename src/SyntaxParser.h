#pragma once

#include "Tokenizer.h"

#include "Common.h"

#include <functional>

using std::bind;

class SyntaxParser
{
public:
	// using declarations
	using Token     = Tokenizer::Token;
	using TokenName = Tokenizer::TokenName;
	using TokenType = Tokenizer::TokenType;
	using TokenList = Tokenizer::TokenList;

	enum class SyntaxTemplateType
	{
		Empty,
		Type,
		ConstType,
		BasicType,
		PointerTypeSequence,
		ArrayTypeSequence,
		Value,
		Variable,
		Identifier,
		LeftValue,
		RightValue,
		Operator,
		Expresstion,
		Statement,
		StatementSequence,
		SyntaxParseBegin,
		SyntaxParseEnd,
		Function,
		ArgDeclSequence,
		ArgSequence
	};

	// 语法模板名称
	enum class SyntaxTemplateName
	{
		Empty,
		EmptyStatementSequence,
		EmptyArgDeclSequence,
		EmptyArgSequence,
		EmptyConstType,
		EmptyPointerTypeSequence,
		EmptyArrayTypeSequence,

		Operation,
		FunctionCall,
		FunctionReturn,
		BracketExpression,
		ValueExpression,
		ExpressionStament,
		VariableDecl,
		VariableDeclStatement,
		VariableImplStatement,
		FunctionDeclStatement,
		FunctionImplStatement,
		BraceStament,
		ArgDeclSequence,
		ArgDecl,
		ArgSequence,
		Arg,
		Statement,
		StatementSequence,
		SyntaxParseBegin,
		SyntaxParseEnd,
		Type,
		ConstType,
		BasicType,
		PointerTypeSequence,
		ArrayTypeSequence,

		Identifier,
		LeftValue,
		RightValue,

		// 展开为TokenName的模板，格式为[TemplateName]Value
		Value,
		VariableDeclValue,
		TypeValue,
		ConstTypeValue,
		IdentifierValue,
		OperatorValue,
		LeftValueValue, // 左值为标识符模板，非终结符，所以禁用
		RightValueValue,

		// 模板展开结束提示符
		TemplateEnd,
	};

	// 语法序列
	using SyntaxTemplateUnit     = Variant<TokenName, SyntaxTemplateType>;
	using SyntaxTemplateUnitList = Vector<SyntaxTemplateUnit>;

	template <class _Ty>
	using Function = std::function<_Ty>;

	// 语法序列
	struct SyntaxSequence: public SyntaxTemplateUnitList
	{
		struct ConstructArgs
		{
			SyntaxTemplateName name = SyntaxTemplateName::Empty;
			SyntaxTemplateUnitList sequences = { };
		};
		SyntaxSequence(const ConstructArgs args = { });

		SyntaxTemplateName name;
	};

	// 增加了语法模板展开信息的语法序列
	struct SyntaxMetaSequence : public SyntaxSequence
	{
		struct TemplateNameMetaData
		{
			SyntaxTemplateName name;
			size_t position;
			size_t size;
		};

	public:
		SyntaxMetaSequence(const ConstructArgs args = { });
		SyntaxMetaSequence(const SyntaxSequence& sequence);

		void AddTemplateSequence(const SyntaxMetaSequence& sequence, const size_t position);
		void AddTemplateEnd();

	public:
		Vector<TemplateNameMetaData> templateNameMetaDataList;
	};
	using MakeSyntaxSequence = SyntaxSequence::ConstructArgs;

	using SyntaxSequenceList     = Vector<SyntaxSequence>;
	using SyntaxMetaSequenceList = Vector<SyntaxMetaSequence>;
	
	const SyntaxSequenceList c_Empty = {
		MakeSyntaxSequence{ SyntaxTemplateName::Empty, { TokenName::Empty } }
	};

	// (const) type (const) * (const)
	const SyntaxSequenceList c_Type = {
		MakeSyntaxSequence{ SyntaxTemplateName::Type, 
						  { SyntaxTemplateType::ConstType, SyntaxTemplateType::BasicType, 
						    SyntaxTemplateType::ConstType, SyntaxTemplateType::PointerTypeSequence, 
							SyntaxTemplateType::ConstType } },
	};

	const SyntaxSequenceList c_ConstType = {
		MakeSyntaxSequence{ SyntaxTemplateName::ConstTypeValue, { TokenName::Const } },
		MakeSyntaxSequence{ SyntaxTemplateName::EmptyConstType, { SyntaxTemplateType::Empty } },
	};

	const SyntaxSequenceList c_BasicType = {
		MakeSyntaxSequence{ SyntaxTemplateName::TypeValue, { TokenName::Int } },
		MakeSyntaxSequence{ SyntaxTemplateName::TypeValue, { TokenName::Long } },
		MakeSyntaxSequence{ SyntaxTemplateName::TypeValue, { TokenName::Float } },
		MakeSyntaxSequence{ SyntaxTemplateName::TypeValue, { TokenName::Bool } },
		MakeSyntaxSequence{ SyntaxTemplateName::TypeValue, { TokenName::Char } },
		MakeSyntaxSequence{ SyntaxTemplateName::TypeValue, { TokenName::String } },
		MakeSyntaxSequence{ SyntaxTemplateName::TypeValue, { TokenName::Void } },
		MakeSyntaxSequence{ SyntaxTemplateName::TypeValue, { TokenName::Const } },
		MakeSyntaxSequence{ SyntaxTemplateName::TypeValue, { TokenName::Long, TokenName::Long } },
	};

	const SyntaxSequenceList c_PointerTypeSequence = {
		// 类型指针序列
		// Type * (PointerTypeSequence)
		MakeSyntaxSequence{ SyntaxTemplateName::PointerTypeSequence, 
						  { TokenName::Mul, SyntaxTemplateType::PointerTypeSequence  } },

		MakeSyntaxSequence{ SyntaxTemplateName::EmptyPointerTypeSequence, { SyntaxTemplateType::Empty } },
	};

	const SyntaxSequenceList c_ArrayTypeSequence = {
		// 不定长类型数组序列
		// Type Identifier [] (ArrayTypeSequence)
		MakeSyntaxSequence{ SyntaxTemplateName::ArrayTypeSequence,
						  { TokenName::LeftSquare, TokenName::RightSquare, SyntaxTemplateType::ArrayTypeSequence } },

		// 定长类型数组序列(Value: Const)
		// Type Identifier [Value] (ArrayTypeSequence)
		MakeSyntaxSequence{ SyntaxTemplateName::ArrayTypeSequence,
						{ TokenName::LeftSquare, SyntaxTemplateType::Value, TokenName::RightSquare, SyntaxTemplateType::ArrayTypeSequence } },

		MakeSyntaxSequence{ SyntaxTemplateName::EmptyArrayTypeSequence, { SyntaxTemplateType::Empty } },
	};

	const SyntaxSequenceList c_Value = {
		MakeSyntaxSequence{ SyntaxTemplateName::LeftValue , { SyntaxTemplateType::LeftValue } },
		MakeSyntaxSequence{ SyntaxTemplateName::RightValue, { SyntaxTemplateType::RightValue } },
	};

	const SyntaxSequenceList c_Identifier = {
		MakeSyntaxSequence{ SyntaxTemplateName::IdentifierValue, { TokenName::Identifier } },
	};

	const SyntaxSequenceList c_LeftValue = {
		MakeSyntaxSequence{ SyntaxTemplateName::Identifier, { SyntaxTemplateType::Identifier } }
	};

	const SyntaxSequenceList c_RightValue = {
		MakeSyntaxSequence{ SyntaxTemplateName::RightValueValue, { TokenName::String } },
		MakeSyntaxSequence{ SyntaxTemplateName::RightValueValue, { TokenName::Number } }
	};

	const SyntaxSequenceList c_Operator = {
		MakeSyntaxSequence{ SyntaxTemplateName::OperatorValue, { TokenName::Add } },
		MakeSyntaxSequence{ SyntaxTemplateName::OperatorValue, { TokenName::Sub } },
		MakeSyntaxSequence{ SyntaxTemplateName::OperatorValue, { TokenName::Mul } },
		MakeSyntaxSequence{ SyntaxTemplateName::OperatorValue, { TokenName::Div } },
		MakeSyntaxSequence{ SyntaxTemplateName::OperatorValue, { TokenName::Mod } },
		MakeSyntaxSequence{ SyntaxTemplateName::OperatorValue, { TokenName::Assign } },
		MakeSyntaxSequence{ SyntaxTemplateName::OperatorValue, { TokenName::And } },
		MakeSyntaxSequence{ SyntaxTemplateName::OperatorValue, { TokenName::Or } },
		MakeSyntaxSequence{ SyntaxTemplateName::OperatorValue, { TokenName::Not } },
		MakeSyntaxSequence{ SyntaxTemplateName::OperatorValue, { TokenName::Ger } },
		MakeSyntaxSequence{ SyntaxTemplateName::OperatorValue, { TokenName::Geq } },
		MakeSyntaxSequence{ SyntaxTemplateName::OperatorValue, { TokenName::Leq } },
		MakeSyntaxSequence{ SyntaxTemplateName::OperatorValue, { TokenName::Les } },
		MakeSyntaxSequence{ SyntaxTemplateName::OperatorValue, { TokenName::Equ } },
		MakeSyntaxSequence{ SyntaxTemplateName::OperatorValue, { TokenName::Neq } },
		MakeSyntaxSequence{ SyntaxTemplateName::OperatorValue, { TokenName::DoubleLeft } },
		MakeSyntaxSequence{ SyntaxTemplateName::OperatorValue, { TokenName::DoubleRight } },
		MakeSyntaxSequence{ SyntaxTemplateName::OperatorValue, { TokenName::Comma } },
	};

	const SyntaxSequenceList c_Variable = {
		MakeSyntaxSequence { SyntaxTemplateName::VariableDeclValue,
						   { SyntaxTemplateType::Type, SyntaxTemplateType::Identifier, SyntaxTemplateType::ArrayTypeSequence } },
	};

	const SyntaxSequenceList c_ArgDecleartionSequence = {
		// 参数声明
		// Type LeftValue , (ArgDeclSequence...)
		MakeSyntaxSequence{ SyntaxTemplateName::ArgDeclSequence, 
						  { SyntaxTemplateType::Variable, TokenName::Comma, SyntaxTemplateType::ArgDeclSequence } },

		MakeSyntaxSequence{ SyntaxTemplateName::ArgDecl, 
						  { SyntaxTemplateType::Variable } },

		MakeSyntaxSequence{ SyntaxTemplateName::EmptyArgDeclSequence, 
						  { SyntaxTemplateType::Empty } }
	};

	const SyntaxSequenceList c_ArgSequence = {
		// 参数传递
		// ValueExpression , (ArgSequence...)
		MakeSyntaxSequence{ SyntaxTemplateName::ArgSequence, 
						  { SyntaxTemplateType::Value, TokenName::Comma, SyntaxTemplateType::ArgSequence } },

		MakeSyntaxSequence{ SyntaxTemplateName::Arg, 
						  { SyntaxTemplateType::Value } },

		MakeSyntaxSequence{ SyntaxTemplateName::EmptyArgSequence, 
						  { SyntaxTemplateType::Empty } }
	};

	const SyntaxSequenceList c_Expression = {
		// 运算
		// LeftValue Operator Expresstion
		MakeSyntaxSequence{ SyntaxTemplateName::Operation, 
						  { SyntaxTemplateType::Value, SyntaxTemplateType::Operator, SyntaxTemplateType::Expresstion } },

		// 函数调用
		// Identifier ( ArgSequence )
		MakeSyntaxSequence{ SyntaxTemplateName::FunctionCall, 
						  { SyntaxTemplateType::Identifier, TokenName::LeftBracket, SyntaxTemplateType::ArgSequence, TokenName::RightBracket } },

		// 括号
		// ( Expresstion )
		MakeSyntaxSequence{ SyntaxTemplateName::BracketExpression, 
						  { TokenName::LeftBracket, SyntaxTemplateType::Expresstion, TokenName::RightBracket } },

		// 直接传值
		// ValueExpression
		MakeSyntaxSequence{ SyntaxTemplateName::ValueExpression, 
						  { SyntaxTemplateType::Value } }
	};

	const SyntaxSequenceList c_Statement = {
		// 表达式语句
		MakeSyntaxSequence{ SyntaxTemplateName::ExpressionStament, 
						  { SyntaxTemplateType::Expresstion, TokenName::Semicolon } },

		// 变量声明
		// Type Identifier ;
		MakeSyntaxSequence{ SyntaxTemplateName::VariableDeclStatement, 
						  { SyntaxTemplateType::Variable, TokenName::Semicolon } },

		// 变量声明并赋值
		// VariableDecl = Expresstion ;
		MakeSyntaxSequence{ SyntaxTemplateName::VariableImplStatement, 
						  { SyntaxTemplateType::Variable, TokenName::Assign, 
						    SyntaxTemplateType::Expresstion, TokenName::Semicolon } },

		// 函数声明
		// Type Identifier ( ArgDeclSequence ) ;
		MakeSyntaxSequence{ SyntaxTemplateName::FunctionDeclStatement, 
						  { SyntaxTemplateType::Type, SyntaxTemplateType::Identifier, 
						    TokenName::LeftBracket, SyntaxTemplateType::ArgDeclSequence, TokenName::RightBracket, 
							TokenName::Semicolon } },

		// 函数实现
		// Type Identifier ( ArgDeclSequence ) { (Statement...) }
		MakeSyntaxSequence{ SyntaxTemplateName::FunctionImplStatement, 
						  { SyntaxTemplateType::Type, SyntaxTemplateType::Identifier, 
						    TokenName::LeftBracket, SyntaxTemplateType::ArgDeclSequence, TokenName::RightBracket, 
							TokenName::LeftBrace, SyntaxTemplateType::StatementSequence, TokenName::RightBrace } },

		// 函数返回
		// Return Expresstion ;
		MakeSyntaxSequence{ SyntaxTemplateName::FunctionReturn,
						  { TokenName::Return, SyntaxTemplateType::Expresstion, TokenName::Semicolon } },

		// 大括号
		// { (Statement...) }
		MakeSyntaxSequence{ SyntaxTemplateName::VariableImplStatement, 
						  { TokenName::LeftBrace, SyntaxTemplateType::Statement, TokenName::RightBrace } },
	};

	const SyntaxSequenceList c_StatementSequence = {
		// 语句串
		// Statement (Statement...)
		MakeSyntaxSequence{ SyntaxTemplateName::Statement, 
						  { SyntaxTemplateType::Statement, SyntaxTemplateType::StatementSequence } },

		// 空语句 
		MakeSyntaxSequence{ SyntaxTemplateName::EmptyStatementSequence, { SyntaxTemplateType::Empty } }
	};

	const SyntaxSequenceList c_SyntaxParseBegin = {
		MakeSyntaxSequence{ SyntaxTemplateName::StatementSequence,
						  { SyntaxTemplateType::StatementSequence, TokenName::End } },
	};

	using SyntaxUnitsMap = Map<SyntaxTemplateType, SyntaxSequenceList>;

	const SyntaxUnitsMap m_SyntaxUnits = {
		{ SyntaxTemplateType::SyntaxParseBegin, c_SyntaxParseBegin },
		{ SyntaxTemplateType::Empty, c_Empty },
		{ SyntaxTemplateType::Type, c_Type },
		{ SyntaxTemplateType::ConstType, c_ConstType },
		{ SyntaxTemplateType::BasicType, c_BasicType },
		{ SyntaxTemplateType::PointerTypeSequence, c_PointerTypeSequence },
		{ SyntaxTemplateType::ArrayTypeSequence, c_ArrayTypeSequence },
		{ SyntaxTemplateType::Value, c_Value },
		{ SyntaxTemplateType::Variable, c_Variable },
		{ SyntaxTemplateType::Identifier, c_Identifier },
		{ SyntaxTemplateType::LeftValue, c_LeftValue },
		{ SyntaxTemplateType::RightValue, c_RightValue },
		{ SyntaxTemplateType::Operator, c_Operator },
		{ SyntaxTemplateType::Expresstion, c_Expression },
		{ SyntaxTemplateType::Statement, c_Statement },
		{ SyntaxTemplateType::StatementSequence, c_StatementSequence },
		{ SyntaxTemplateType::ArgDeclSequence, c_ArgDecleartionSequence },
		{ SyntaxTemplateType::ArgSequence, c_ArgSequence }
	};

	//using SyntaxTemplateActionFunction = void(SyntaxParser::*)(SyntaxSequence&);
	using SyntaxTemplateActionFunction = Function<void(SyntaxSequence&)>;
	using StatementActionMap = Map<SyntaxTemplateName, SyntaxTemplateActionFunction>;

	// 注册Template动作
	const StatementActionMap c_StatementActionMap =
	{
		//{ SyntaxTemplateName::Empty, bind(&SyntaxParser::EmptyAction, this, std::placeholders::_1) },
		//{ SyntaxTemplateName::ArgDeclSequence, bind(&SyntaxParser::ArgDeclSequenceAction, this, std::placeholders::_1) },
		//{ SyntaxTemplateName::ArgDecl, bind(&SyntaxParser::ArgDeclSequenceAction, this, std::placeholders::_1) },
		//{ SyntaxTemplateName::ArgSequence, bind(&SyntaxParser::ArgSequenceAction, this, std::placeholders::_1) },
		//{ SyntaxTemplateName::BraceStament, bind(&SyntaxParser::LeftValueAction, this, std::placeholders::_1) },
		//{ SyntaxTemplateName::BracketExpression, bind(&SyntaxParser::ExpresstionAction, this, std::placeholders::_1) },
		//{ SyntaxTemplateName::ExpressionStament, bind(&SyntaxParser::ExpresstionAction, this, std::placeholders::_1) },
		//{ SyntaxTemplateName::FunctionCall, bind(&SyntaxParser::FunctionAction, this, std::placeholders::_1) },
		//{ SyntaxTemplateName::FunctionSignature, bind(&SyntaxParser::FunctionAction, this, std::placeholders::_1) },
		//{ SyntaxTemplateName::FunctionDeclStatement, bind(&SyntaxParser::FunctionAction, this, std::placeholders::_1) },
		//{ SyntaxTemplateName::FunctionImplStatement, bind(&SyntaxParser::FunctionAction, this, std::placeholders::_1) },
		//{ SyntaxTemplateName::FunctionReturn, bind(&SyntaxParser::RightValueAction, this, std::placeholders::_1) },
		//{ SyntaxTemplateName::Operation, bind(&SyntaxParser::OperatorAction, this, std::placeholders::_1) },
		//{ SyntaxTemplateName::VariableDeclStatement, bind(&SyntaxParser::LeftValueAction, this, std::placeholders::_1) },
		//{ SyntaxTemplateName::VariableImplStatement, bind(&SyntaxParser::LeftValueAction, this, std::placeholders::_1) },
		//{ SyntaxTemplateName::ValueExpression, bind(&SyntaxParser::ValueAction, this, std::placeholders::_1) },
	};

private:


public:
	SyntaxParser();
	~SyntaxParser();

	void Load(TokenList& tokens);
	void Parse();

private:
	void ParseNextToken();

	Vector<SyntaxMetaSequence> ExtendTemplates(SyntaxMetaSequence& sequence, size_t position);
	void UpdateReadingToken();
	void PatternTokenNameWithSequence(SyntaxMetaSequence& sequence);
	bool IsTokenName(SyntaxTemplateUnit& unit);
	bool IsTemplate(SyntaxTemplateUnit& unit);

	void ResetEstimatingSequences();
	bool AddEstimatingSequence(SyntaxMetaSequence& sequence);
	void RemoveEstimatingSequence(SyntaxMetaSequence& sequence);
	void UpdateSequences();
	void CheckEstimated();
	void MakeAST();

	static String EnumName(SyntaxTemplateUnit unit);
	static void PrintSequence(String desc, SyntaxSequence& sequence, String color = "37", int highlightPos = -1, bool underline = false, bool lineBreak = true);
	void PrintEstimatingSequences();
	void PrintEstimatedSequences();

// template动作
private:
	void EmptyAction(SyntaxSequence&);
	void TypeAction(SyntaxSequence&);
	void ValueAction(SyntaxSequence&);
	void IdentifierAction(SyntaxSequence&);
	void LeftValueAction(SyntaxSequence&);
	void RightValueAction(SyntaxSequence&);
	void OperatorAction(SyntaxSequence&);
	void ExpresstionAction(SyntaxSequence&);
	void StatementAction(SyntaxSequence&);
	void FunctionAction(SyntaxSequence&);
	void ArgDeclSequenceAction(SyntaxSequence&);
	void ArgSequenceAction(SyntaxSequence&);

private:
	TokenList m_Tokens;
	Token m_ReadingToken;
	SyntaxTemplateUnit m_ReadingSequenceUnit;
	size_t m_ReadingPosition;
	size_t m_EstimatingPosition;
	size_t m_LastEstimatedPosition;
	size_t m_ReadingSequenceSize;

	SyntaxMetaSequenceList m_EstimatingSequences;
	SyntaxMetaSequenceList m_SequencesToBeRemovedLater;
	SyntaxMetaSequenceList m_SequencesToBeAddedLater;

	bool m_ResetSignal;
	SyntaxMetaSequenceList m_EstimatedSequences;

	CST* m_AST;
};
