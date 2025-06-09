#pragma once

#include "MetaObjects.h"
#include "MetaObject.h"

class Expression: public MetaObject
{
public: 
	struct Args {
		String name;
	};
	Expression(const Args& args);


public:
	String name;

};

