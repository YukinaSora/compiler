/*
 * Sematic Analyser 语义分析器
 * 
 * 根据文法序列与分词序列生成CST（具体语法树）与AST（抽象语法树）
 */

#pragma once

#include "SyntaxParser.h"

#include "CST.h"
#include "AST.h"

class SemanticAnalyser
{
	friend class SyntaxParser;

public:
	SemanticAnalyser();
	~SemanticAnalyser();


	

private:
	void Reset();

};

