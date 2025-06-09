#pragma once

#include "Type.h"

class Variable: public MetaObject
{
public:
	struct Args {

	};
	Variable(const Args& args) noexcept;

public:
	Type type;
	String identifier;
	void* value;
};

