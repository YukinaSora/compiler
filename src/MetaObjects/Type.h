#pragma once

#include "MetaObject.h"

class Type: public MetaObject
{
private:
	const Map<TokenName, size_t> c_TypeWidthTable = {
		{ TokenName::Int, 8 },
	};

public:
	struct Args {
		String name;
		size_t width;
		size_t pointerLevel;
		bool   isConst;
		Vector<size_t> arrayDims;
	};
	Type(const Args& args) noexcept;

	Type Make(const TokenName& type);

public:
	String name;				// ��������
	size_t width;				// ����ռ�õ��ֽڿ��
	size_t pointerLevel;		// ָ�����
	bool   isConst;				// �ɷ��޸�
	Vector<size_t> arrayDims;   // ����ά�ȣ�������ʱΪ��
};

export Type g_Type;

