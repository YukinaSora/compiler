#pragma once

#include <iostream>
#include <vector>
#include <variant>
#include <string>
#include <map>

#include "enum_stringify.h"

using std::cout;
using std::wcout;
using std::cerr;
using std::endl;
using std::isalpha;
using std::string;

using String = std::string;
using ConstexprString = std::string_view;

template<typename _Ty>
using Vector = std::vector<_Ty>;

template<class ..._Tys>
using Variant = std::variant<_Tys...>;

template<typename ..._Ty>
using Map = std::map<_Ty...>;

using YukinaSora::enum_name;

class Tokenizer;
class SyntaxParser;
class CST;