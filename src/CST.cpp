#include "CST.h"

#include "Utils.h"

CST::CST() noexcept
	: m_Root{ .value = SyntaxTemplateName::Empty }
	, m_CurrentNode{ }
	, m_ReadTokenCondition{ false }, m_ReadTokenPosition{ }
{

}

void CST::SetContext(const ParseContext& context)
{
	m_Context = context;
}

void CST::Parse()
{
	m_ReadTokenCondition = false;
	m_ReadTokenPosition = 0;

	m_Root = Node{
		.value = m_Context.sequence.templateNameMetaDataList[0].name,
		.size = m_Context.sequence.templateNameMetaDataList[0].size,
		.parent = nullptr
	};
	m_CurrentNode = &m_Root;

	for (size_t i = 1; i < m_Context.sequence.templateNameMetaDataList.size(); ++i)
	{
		AddNode(m_Context.sequence.templateNameMetaDataList[i]);
	}

	// ��β��TemplateEnd�޷�����һToken���������ڴ�����Node
	ReadTokenToPosition(m_ReadTokenPosition + 1);
}

void CST::Print()
{
	std::locale::global(std::locale(""));
	std::wcout.imbue(std::locale());
	wcout << L"ConcreteSyntaxTree\n";
	//Print(m_Root, L"", true, true);
	Print(m_Root, L"", true, false);
}

void CST::Print(const Node& node, WString prefix, bool isLast, bool moe) const
{
	// �����ǰ׺�����������ߣ�
	wcout << prefix;

	// �����Ƿ������һ���ӽڵ�ѡ��ͬ��������
	if (isLast) {
		wcout << L"������ ";
		// Ϊ�ӽڵ������µ�ǰ׺�����һ���ӽڵ�ķ�֧��Ҫ���̣�
		prefix += L"    ";
	}
	else {
		wcout << L"������ ";
		// Ϊ�ӽڵ������µ�ǰ׺�������һ���ӽڵ�ķ�֧��Ҫ������
		prefix += L"��   ";
	}

	// ��ӡ�ڵ�ֵ
	if (std::holds_alternative<TokenName>(node.value)) {
		if (moe)
		{
			auto name = EnumNameWillainVer(node.value);
			wcout << L"\033[32m";
			SetRGBColor(135, 206, 250);
			WPrint(name.data());
			wcout << L"\033[0m\n";
		}
		else {
			auto tokenName = std::get<TokenName>(node.value);
			auto name = enum_name(tokenName);
			name = name.substr(name.find_last_of("::") + 1);
			cout << "\033[32m" << name << "\033[0m\n";
		}
	}
	else if (std::holds_alternative<SyntaxTemplateName>(node.value)) {
		if (moe)
		{
			auto name = EnumNameWillainVer(node.value);
			wcout << L"\033[32m";
			SetRGBColor(255, 192, 203);
			WPrint(name.data());
			wcout << L"\033[0m\n";
		}
		else {
			auto templateName = std::get<SyntaxTemplateName>(node.value);
			auto name = enum_name(templateName);
			name = name.substr(name.find_last_of("::") + 1);
			cout << "\033[35m" << name << "\033[0m\n";
		}
	}

	// �ݹ��ӡ�ӽڵ�
	for (size_t i = 0; i < node.children.size(); ++i) {
		Print(node.children[i], prefix, i == node.children.size() - 1, moe);
	}
}

// ʵ��EnumNameWillainVer���������ؿ��ַ���
CST::WString CST::EnumNameWillainVer(const ValueType& value) const
{
	if (std::holds_alternative<SyntaxTemplateName>(value)) {
		auto templateName = std::get<SyntaxTemplateName>(value);
		auto it = templateNameMap.find(templateName);
		if (it != templateNameMap.end()) {
			return it->second;
		}
		auto name = enum_name(get<SyntaxTemplateName>(value));
		name = name.substr(name.find_last_of("::") + 1);
		return StringToWString(name);
	}
	else if (std::holds_alternative<TokenName>(value)) {
		auto tokenName = std::get<TokenName>(value);
		auto it = tokenNameMap.find(tokenName);
		if (it != tokenNameMap.end()) {
			return it->second;
		}
		auto name = enum_name(get<TokenName>(value));
		name = name.substr(name.find_last_of("::") + 1);
		return StringToWString(name);
	}
	return L"�ǰ�����֪����������QAQ";
}

// ��Tokens�ж����ս��
void CST::ReadTokenToPosition(const size_t position)
{
	while (m_ReadTokenPosition < position)
	{
		Node node{
			.value = m_Context.tokens[m_ReadTokenPosition],
			.size = 0,
			.parent = m_CurrentNode
		};

		m_CurrentNode->children.push_back(node);
		m_CurrentNode = &(m_CurrentNode->children.back());

		Backtrack();

		m_ReadTokenPosition++;
	}
	m_ReadTokenCondition = false;
}

void CST::Backtrack()
{
	if (m_CurrentNode->size > m_CurrentNode->children.size())
		return;

	do {
		m_CurrentNode = m_CurrentNode->parent;
	} while (m_CurrentNode->parent && m_CurrentNode->size <= m_CurrentNode->children.size());
}

void CST::AddNode(TemplateNameMetaData data)
{
	if (data.name == SyntaxTemplateName::TemplateEnd)
	{
		m_ReadTokenCondition = true;
		return;
	}

	if (m_ReadTokenCondition)
	{
		ReadTokenToPosition(data.position);
	}

	Node node{
		.value = data.name,
		.size = data.size
	};

	node.parent = m_CurrentNode;
	m_CurrentNode->children.push_back(node);
	m_CurrentNode = &(m_CurrentNode->children.back());

	switch (get<SyntaxTemplateName>(m_CurrentNode->value))
	{
	case SyntaxTemplateName::Empty:
	case SyntaxTemplateName::EmptyArgDeclSequence:
	case SyntaxTemplateName::EmptyArgSequence:
	case SyntaxTemplateName::EmptyStatementSequence:
	case SyntaxTemplateName::EmptyPointerTypeSequence:
	case SyntaxTemplateName::EmptyArrayTypeSequence:
	case SyntaxTemplateName::EmptyConstType:
		m_CurrentNode->children.push_back(Node{
			.value = TokenName::Empty,
			.size = 0,
			.parent = m_CurrentNode
			});
		Backtrack();
		m_ReadTokenCondition = true;
	}
}

void CST::ComsumeWithTemplate(SyntaxMetaSequence sequence)
{
	//cout << "Comsume with template " << enum_name(sequence.name) << ":\n";

	//size_t size = sequence.size();

	//// �����size��node��Ϊǰһ��node��children
	//auto it = m_TemplateNodes.end() - size;
	//Vector<Node> nodes = { it, m_TemplateNodes.end() };

	//m_TemplateNodes.erase(it, m_TemplateNodes.end());

	//m_TemplateNodes.back().children = nodes;
}