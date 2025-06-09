#include "Compiler.h"
#include "SyntaxParser.h"
#include "SemanticAnalyser.h"

#include "enum_stringify.h"
#include "Utils.h"

#include <unordered_map>

using std::get;
using std::get_if;
using std::string;
using std::wcout;

using YukinaSora::enum_name;

SyntaxParser::SyntaxSequence::SyntaxSequence(ConstructArgs args)
	: SyntaxTemplateUnitList(args.sequences), name(args.name)
{

}

SyntaxParser::SyntaxMetaSequence::SyntaxMetaSequence(ConstructArgs args)
	: SyntaxSequence(args)
	, templateNameMetaDataList{ 
		TemplateNameMetaData{
		.name = args.name,
		.size = args.sequences.size()
		}
	}
{

}

SyntaxParser::SyntaxMetaSequence::SyntaxMetaSequence(const SyntaxSequence& sequence)
	: SyntaxSequence{
		ConstructArgs{
			.name = sequence.name,
			.sequences = sequence
		}
	}
{

}

// 添加sequence[position]处的模板展开信息
void SyntaxParser::SyntaxMetaSequence::AddTemplateSequence(const SyntaxMetaSequence& sequence, const size_t position)
{
	templateNameMetaDataList.push_back(TemplateNameMetaData{
		.name = sequence.name,
		.position = position,
		.size = sequence.size()
	});
}

// 添加模板展开的终止符
void SyntaxParser::SyntaxMetaSequence::AddTemplateEnd()
{
	templateNameMetaDataList.push_back(TemplateNameMetaData{
		.name = SyntaxTemplateName::TemplateEnd,
		.size = 0
	});
}

SyntaxParser::SyntaxParser()
{
	m_EstimatingPosition = 0;
	m_ReadingSequenceSize = 0;
	m_LastEstimatedPosition = 0;
	m_ReadingPosition = 0;
	m_ResetSignal = false;
	m_AST = new CST();
}

SyntaxParser::~SyntaxParser()
{

}

void SyntaxParser::Load(TokenList& tokens)
{
	m_Tokens = tokens;
}

void SyntaxParser::Parse()
{
	cout << "Parsing syntax...\n";

	m_ReadingToken = m_Tokens[0];
	m_ReadingPosition = 0;
	m_LastEstimatedPosition = 0;
	m_ResetSignal = true;

	while (m_ReadingToken.type != TokenName::End || m_ReadingPosition > m_Tokens.size())
	{
		if (m_ResetSignal)
			ResetEstimatingSequences();

		UpdateReadingToken();

		ParseNextToken();

		UpdateSequences();

		PrintEstimatingSequences();

		CheckEstimated();

		m_ReadingPosition++;
		m_EstimatingPosition++;
	}

	if (m_EstimatedSequences.empty())
		return;

	m_EstimatedSequences[0].AddTemplateEnd();

	MakeAST();

	PrintEstimatedSequences();
}

void SyntaxParser::ParseNextToken()
{
	// BFS(在EstimatingSequences中，随Token读取而逐个比对)
	// DFS(在Template解析中深度匹配)
	for (size_t i = 0; i < m_EstimatingSequences.size(); i++)
	{
		auto& sequence = m_EstimatingSequences[i];

		if (m_EstimatingPosition >= sequence.size())
		{
			RemoveEstimatingSequence(sequence);
			continue;
		}

		auto unit = sequence[m_EstimatingPosition];

		// 比较TokenName
		if (IsTokenName(unit))
		{
			PatternTokenNameWithSequence(sequence);
		}
		// 展开模板后比较
		else if (IsTemplate(unit))
		{
			// 循环展开模板，直至TokenIsTokenName
			// *语法模板中不应有左递归
			// *似乎总是返回一个sequence，待确定
			auto retSequences = ExtendTemplates(sequence, m_EstimatingPosition);

			for (size_t i = 0; i < retSequences.size(); i++)
			{
				auto& retSequence = retSequences[i];

				if (retSequence.size() <= m_ReadingPosition)
					continue;

				//retSequence.insert(retSequence.begin(), { TokenName::Empty });

				auto& tokenInTemplate = get<TokenName>(retSequence[m_ReadingPosition]);
				if (m_ReadingToken != tokenInTemplate)
					continue;

				// 去重
				if (!AddEstimatingSequence(retSequence)) 
					continue;

				// -------- 分支逻辑部分完毕 --------

				// 输出展开部分
				// 别看，看了也不懂
				// only god knows now
				SyntaxSequence subSequence;
				size_t size = m_ReadingSequenceSize - 1;
				for (size_t i = 0; i < size; i++)
				{
					subSequence.push_back(sequence[i]);
				}

				// 当前匹配模板名称
				if (!retSequence.templateNameMetaDataList.empty()) {
					auto name = enum_name(retSequence.templateNameMetaDataList[retSequence.templateNameMetaDataList.size()-2].name);
					name = name.substr(name.find_last_of("::") + 1);
					cout << "\033[35m[" << name << "]\033[0m:\n";
				}

				// template之前的部分
				PrintSequence("", subSequence, "36", static_cast<int>(size), false, false);
				cout << " \033[35m(";
				cout << "\033[35m" << EnumName(unit) << "\033[0m";
				cout << " -> ";

				// 当模板展开时，由于空模板已被删除，所以retSequence.size()会比sequence.size()小
				// 所以当跳过了空模板时，这里打印一条空模板推导的日志
				// TODO: 目前仅支持跳过一条空模板，多条跳过日志待完善
				subSequence.clear();
				bool isTemplateExtendedEmpty = false;
				if (retSequence.size() > sequence.size())
				{
					subSequence.push_back(retSequence[m_ReadingSequenceSize - 1]);
				}
				else {
					isTemplateExtendedEmpty = true;
					subSequence.push_back(SyntaxTemplateUnit{ TokenName::Empty });
				}
				PrintSequence("", subSequence, "37", 0, false, false);
				cout << "\033[35m)\033[0m ";

				// 展开为空后，标记下一个Token为匹配中
				if (isTemplateExtendedEmpty && m_ReadingSequenceSize <= retSequence.size())
				{
					SyntaxSequence nextToken = MakeSyntaxSequence{ .sequences = { retSequence[m_ReadingSequenceSize - 1] } };
					cout << "-> ";
					PrintSequence("", nextToken, "32", 0, false, false);
					cout << " ";
				}

				// template之后的部分
				subSequence.clear();
				size_t start = m_ReadingSequenceSize 
					+ static_cast<int>(retSequence.size()) - static_cast<int>(sequence.size()) 
					+ static_cast<int>(isTemplateExtendedEmpty);
				for (size_t i = start; i < retSequence.size(); i++)
				{
					subSequence.push_back(retSequence[i]);
				}
				// -2: 在不启用highlight的情况下，启用Template高亮
				PrintSequence("", subSequence, "37", -2, false, true);

				if (m_ReadingToken == TokenName::End)
					break;
			}
		}
	}
}

Vector<SyntaxParser::SyntaxMetaSequence> SyntaxParser::ExtendTemplates(SyntaxMetaSequence& sequence, size_t position)
{
	Vector<SyntaxMetaSequence> returnSequences;

	//for (size_t i = 0; i < m_LogIndent; i++) cout << "  ";
	//cout << "Extending template " << EnumName(get<SyntaxTemplateType>(sequence[position])) << " as:\n";
	//m_LogIndent++;

	SyntaxTemplateType type = get<SyntaxTemplateType>(sequence[position]);
	SyntaxSequenceList content = m_SyntaxUnits.find(type)->second;

	for(size_t i = 0; i < content.size(); i++)
	{
		SyntaxMetaSequence extendedSequence = content[i];

		//for (size_t i = 0; i < m_LogIndent; i++) cout << "  ";
		//PrintSequence("", extendedSequence);

		if (extendedSequence.empty())
			continue;

		// 展开的模板的第一个元素将插入原模板的位置，因此比较第一个元素
		if (get_if<TokenName>(&extendedSequence[0]))
		{
			TokenName readingType   = get<TokenName>(m_ReadingSequenceUnit);
			TokenName expectingType = get<TokenName>(extendedSequence[0]);

			// 删除空通配符后，将新序列插入content，后续再次处理
			if (expectingType == TokenName::Empty)
			{
				returnSequences.push_back(extendedSequence);

				continue;
			}

			bool estimated = false;

			// 比较展开后的TokenName
			if (readingType != expectingType)
			{
				continue;
			}

			//for (size_t i = 0; i < m_LogIndent; i++) cout << "  ";
			//cout << "\033[32mTemplate [" << EnumName(type) << "] expanded to: ";
			//PrintSequence("", extendedSequence);
			//cout << "\033[0m";

			// 复制一份sequence，加入m_EstimatingSequences
			SyntaxMetaSequence newSequence = sequence;

			// 将模板处替换为模板内容
			newSequence.erase (newSequence.begin() + position);
			newSequence.insert(newSequence.begin() + position, extendedSequence.begin(), extendedSequence.end());

			newSequence.AddTemplateSequence(extendedSequence, m_EstimatingPosition);
			newSequence.AddTemplateEnd();

			returnSequences.push_back(newSequence);
		}
		// 若展开后仍是模板，递归展开
		// 由于语法模板中不应有左递归，因此不会出现无限递归
		else if (get_if<SyntaxTemplateType>(&extendedSequence[0]))
		{
			// 用于递归的模板比较第一个元素，原因同上
			auto retSequences = ExtendTemplates(extendedSequence, 0);

			if (retSequences.empty())
				continue;

			//for (size_t i = 0; i < m_LogIndent; i++) cout << "  ";
			//cout << "Get extended sequences: \n";
			for (size_t i = 0; i < retSequences.size(); i++)
			{
				SyntaxMetaSequence retSequence = retSequences[i];

				//for (size_t i = 0; i < m_LogIndent; i++) cout << "  ";
				//cout << "\033[32mTemplate [" << EnumName(type) << "] expanded to: ";
				//PrintSequence("", retSequence);
				//cout << "\033[0m";


				// 复制一份sequence，加入m_EstimatingSequences
				SyntaxMetaSequence newSequence = sequence;

				if (get<TokenName>(retSequence[0]) == TokenName::Empty)
				{
					retSequence.erase(retSequence.begin());
					newSequence.AddTemplateEnd();
				}

				// 将模板处替换为模板内容
				newSequence.erase (newSequence.begin() + position);
				newSequence.insert(newSequence.begin() + position, retSequence.begin(), retSequence.end());

				// 若序列仍为模板开头，则继续展开
				if (get_if<SyntaxTemplateType>(&newSequence[position]))
				{
					newSequence.AddTemplateSequence(extendedSequence, m_EstimatingPosition);
					auto t = ExtendTemplates(newSequence, position);
					returnSequences.insert(returnSequences.end(), t.begin(), t.end());
					continue;
				}

				newSequence.name = retSequence.name;

				// BUG(Fixed): 此处的TemplateName是上层语法的name，更进一步说，当前展开的语法的name未正确加入List
				// - Fixed 2025.5.15 YukinaSora
				// 
				// 详细地说，如 Statement->Statement->FunctionImplStatement->Type，
				// 其语法为{ SyntaxTemplateType::Type, SyntaxTemplateType::LeftValue, ... }
				// 由于其语法第一项为另一个模板，因此最终templateNameMetaDataList中会加入FunctionImplementation，而非最终展开的Type。
				// 
				// 同样地，对于 Statement->Statement->FunctionReturn，
				// 其语法为{ TokenName::Return, SyntaxTemplateType::Expresstion, ... }
				// 由于其语法第一项为TokenName，因此最终templateNameMetaDataList中会加入StatementSequence，而非FunctionReturn。
				newSequence.AddTemplateSequence(extendedSequence, m_EstimatingPosition);
				newSequence.templateNameMetaDataList.insert(
					newSequence.templateNameMetaDataList.end(), 
					retSequence.templateNameMetaDataList.begin(), 
					retSequence.templateNameMetaDataList.end()
				);

				returnSequences.push_back(newSequence);
			}
		}
		else
		{
			// 未知类型
		}
	}

	//m_AST->ComsumeWithTemplate(sequence);

	// 由于会将展开后匹配的sequence加入m_EstimatingSequences，
	// 所以这里删除原sequence
	RemoveEstimatingSequence(sequence);

	return returnSequences;
}

void SyntaxParser::UpdateReadingToken()
{
	m_ReadingToken = m_Tokens[m_ReadingPosition];
	m_ReadingSequenceUnit = SyntaxTemplateUnit{ m_ReadingToken };
	m_ReadingSequenceSize++;

	cout << "----------------------------------------\n";
	cout << "Reading token: \033[32m" << m_ReadingToken.pattern << "\033[0m (" << EnumName(m_ReadingToken.type) << ") in pos: " << m_ReadingPosition << endl;
}

void SyntaxParser::PatternTokenNameWithSequence(SyntaxMetaSequence& sequence)
{
	TokenName readingType   = get<TokenName>(m_ReadingSequenceUnit);
	TokenName expectingType = get<TokenName>(sequence[m_EstimatingPosition]);

	if (readingType != expectingType)
	{
		RemoveEstimatingSequence(sequence);
		//cout << "\033[33mSequence unmatched: ";
		//PrintSequence("", sequence);
		//cout << "\033[0m";
		return;
	}
	//cout << "\033[32mSequence matched: ";
	//PrintSequence("", sequence);
	//cout << "\033[0m";
}

bool SyntaxParser::IsTokenName(SyntaxTemplateUnit& unit)
{
	return get_if<TokenName>(&unit);
}

bool SyntaxParser::IsTemplate(SyntaxTemplateUnit& unit)
{
	return get_if<SyntaxTemplateType>(&unit);
}

void SyntaxParser::ResetEstimatingSequences()
{
	m_EstimatingSequences.clear();
	//for(auto& sequence : m_SyntaxUnits.find(SyntaxTemplateType::SyntaxParseBegin)->second)
	//{
	//	m_EstimatingSequences.push_back(sequence);
	//}
	m_EstimatingSequences.push_back(MakeSyntaxSequence{ SyntaxTemplateName::SyntaxParseBegin, {SyntaxTemplateType::SyntaxParseBegin } });

	m_ResetSignal = false;
	m_SequencesToBeRemovedLater.clear();
	m_SequencesToBeAddedLater.clear();
	m_EstimatingPosition = 0;
	m_ReadingSequenceSize = 0;
}

bool SyntaxParser::AddEstimatingSequence(SyntaxMetaSequence& sequence)
{
	bool exists = std::find(m_SequencesToBeAddedLater.begin(), m_SequencesToBeAddedLater.end(), sequence) != m_SequencesToBeAddedLater.end();

	if (!exists) m_SequencesToBeAddedLater.push_back(sequence);
	return !exists;
}

void SyntaxParser::RemoveEstimatingSequence(SyntaxMetaSequence& sequence)
{
	bool exist = std::find(m_SequencesToBeRemovedLater.begin(), m_SequencesToBeRemovedLater.end(), sequence) != m_SequencesToBeRemovedLater.end();

	if (!exist) m_SequencesToBeRemovedLater.push_back(sequence);
}

void SyntaxParser::UpdateSequences()
{
	m_EstimatingSequences.insert(m_EstimatingSequences.end(), m_SequencesToBeAddedLater.begin(), m_SequencesToBeAddedLater.end());

	auto it = std::remove_if(m_EstimatingSequences.begin(), m_EstimatingSequences.end(), [this](const SyntaxSequence& sequence) {
		return std::find(m_SequencesToBeRemovedLater.begin(), m_SequencesToBeRemovedLater.end(), sequence) != m_SequencesToBeRemovedLater.end();
	});

	m_EstimatingSequences.erase(it, m_EstimatingSequences.end());

	m_SequencesToBeRemovedLater.clear();
	m_SequencesToBeAddedLater.clear();
}

void SyntaxParser::CheckEstimated()
{
	for (auto& sequence : m_EstimatingSequences)
	{
		// 跳过空通配符
		if (get_if<TokenName>(&sequence[0]) && get<TokenName>(sequence[0]) == TokenName::Empty)
			continue;

		if (m_ReadingSequenceSize == sequence.size())
		{
			PrintSequence("", sequence, "32");

			// 贪心算法，匹配最短的sequence
			m_EstimatedSequences.push_back(sequence);
			m_ResetSignal = true;
		}
	}
}

void SyntaxParser::MakeAST()
{
	m_AST->SetContext(
		CST::ParseContext{
			.sequence = m_EstimatedSequences[0],
			.tokens = m_Tokens,
		}
	);
	m_AST->Parse();
}

String SyntaxParser::EnumName(SyntaxTemplateUnit unit)
{
	String name;
	if (get_if<SyntaxTemplateType>(&unit))
	{
		name = enum_name(get<SyntaxTemplateType>(unit));
	}
	else if (get_if<TokenName>(&unit))
	{
		name = enum_name(get<TokenName>(unit));
	}

	name = name.substr(name.find_last_of("::") + 1);

	if (get_if<SyntaxTemplateType>(&unit))
	{
		name = String("[") + name + "]";
	}

	return name;
}

void SyntaxParser::PrintSequence(String desc, SyntaxSequence& sequence, String color, int highlightPos, bool underline, bool lineBreak)
{
	// 保留highlightPos之前的unit个数（更之前的将用省略号代替）
	static const int maxBeforeHighlight = 3;

	static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	static CONSOLE_SCREEN_BUFFER_INFO info;

	String name;

	cout << "\033[" << color << "m";
	if (!desc.empty())
		cout << desc;

	if (highlightPos > maxBeforeHighlight + 1)
		cout << "... ";

	//size_t writtenWidth = 0;
	size_t hightlightStart = 0;
	size_t hightlightEnd = 0;
	for (size_t i = max(0, highlightPos - maxBeforeHighlight); i < sequence.size(); i++)
	{
		name = EnumName(sequence[i]);

		bool isTemplate = name.starts_with('[');

		if (i == highlightPos)
		{
			GetConsoleScreenBufferInfo(hConsole, &info);

			hightlightStart = info.dwCursorPosition.X;
			hightlightEnd = hightlightStart + name.size();

			cout << "\033[32m" << name << "\033[" << color << "m";
		}
		else if (highlightPos != -1 && isTemplate)
		{
			cout << "\033[35m" << name << "\033[" << color << "m";
		}
		else if (highlightPos != -1 && i > highlightPos)
		{
			cout << "\033[0m" << name;
		}
		else
		{
			cout << name;
		};
		cout << " \0"[i == sequence.size() - 1];

		//writtenWidth += name.size() + 1;
	}

	if (underline) {
		GetConsoleScreenBufferInfo(hConsole, &info);
		size_t lineWidth = info.srWindow.Right - info.srWindow.Left + 1;
		size_t cursorX = info.dwCursorPosition.X;

		// 当光标位置在hightlightStart前时，补空格
		if (cursorX < hightlightStart)
		{
			for (size_t i = 0; i < hightlightStart - cursorX; i++)
				cout << " ";
		}
		// 在之后时，换行
		else {
			cout << "\n";

			for (size_t i = 0; i < hightlightStart % lineWidth; i++)
				cout << " ";
		}

		cout << "\033[32m";

		for (size_t i = hightlightStart % lineWidth; i < hightlightEnd % lineWidth; i++)
			cout << "~";
	}

	if (lineBreak)
		cout << '\n';

	cout << "\033[0m";
}

void SyntaxParser::PrintEstimatingSequences()
{
	cout << "Estimating sequences:\n";
	int index = 0;
	for (auto& sequence : m_EstimatingSequences)
	{
		bool underline = index++ == static_cast<int>(m_EstimatingSequences.size()) - 1;
		//bool underline = true;
		//auto name = enum_name(sequence.templateNameList.back());
		//name = name.substr(name.find_last_of("::") + 1);
		//cout << "\033[35m[" << name << "]\033[0m:\n";
		PrintSequence("", sequence, "36", static_cast<int>(m_ReadingSequenceSize) - 1, underline);
	}
}

void SyntaxParser::PrintEstimatedSequences()
{
	cout << "----------------------------------------\n";
	cout << "Syntax parsed result:\n";
	for (auto& sequence : m_EstimatedSequences)
	{
		//PrintSequence("", sequence, "32");

		//for (auto& templateName : sequence.templateNameMetaDataList)
		//{
		//	auto name = enum_name(templateName.name);
		//	name = name.substr(name.find_last_of("::") + 1);
		//	cout << "\033[35m" << name << "\033[0m ";
		//}
	}

	cout << "\n";
	m_AST->Print();
}

// ------------------- template actions -------------------

void SyntaxParser::EmptyAction(SyntaxSequence&)
{

}

