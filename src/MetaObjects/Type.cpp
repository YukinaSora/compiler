#include "Type.h"

Type::Type(const Args& args)
	: name{ args.name }, width{ args.width }
	, pointerLevel{ args.pointerLevel }, isConst{ args.isConst }
	, arrayDims{ args.arrayDims }
{

}

Type Type::Make(const TokenName& type)
{
	return Type{ 
		Args {
			.width = c_TypeWidthTable.find(type)->second
		} 
	};
}