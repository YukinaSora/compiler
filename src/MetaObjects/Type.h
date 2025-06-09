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
	String name;				// 类型名称
	size_t width;				// 类型占用的字节宽度
	size_t pointerLevel;		// 指针层数
	bool   isConst;				// 可否修改
	Vector<size_t> arrayDims;   // 数组维度，非数组时为空
};

export Type g_Type;

