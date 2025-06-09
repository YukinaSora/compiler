//
// 可选宏选项：
// 
// 注：宏选项仅在第一次include时有效
// 
// ENUM_STRINGIFY_ENABLE_NAMESPACE
//      是否禁用namespace信息，默认为否。
//
// ENUM_STRINGIFY_ENABLE_NAMESPACE_CONTROL
//      注：仅在ENUM_STRINGIFY_ENABLE_NAMESPACE启用时有效。
//      启用时，将使用disable_enum_namespace变量控制namespace信息的输出。

#ifndef ENUM_STRINGIFY_H
#define ENUM_STRINGIFY_H


#include <iostream>
#include <array>

namespace YukinaSora {

// 编译期通过模板获取枚举对象名称
// 该函数不会产生额外开销
template<auto _Enum>
constexpr auto enum_name();


// 通过函数参数获取枚举对象的名称
// 编译/存储开销注意：
//      该函数会在编译时建立该枚举类型的名称表。
//      对于每个枚举类型仅会建立一次，下列函数同。
template<typename _Enum>
constexpr auto enum_name(_Enum value);


// 打印枚举名(支持一般枚举类型，需要显式指定枚举类型)
template<typename _Enum, typename _Arg>
auto print_enum(_Arg _enum);


// 打印枚举名(支持枚举类类型)
template<typename _Enum>
auto print_enum(_Enum _enum);


// 获取选项枚举对象的名称
// 该枚举对象的值应为2的幂，声明形如：
// enum Enum : int { ZERO = 1, ONE = 2, TWO = 4, THREE = 8 };
template<typename _Enum>
constexpr auto optional_enum_name(_Enum value);


// 打印选项枚举中的所有选项名(支持一般枚举类型)
// _enum应为选项枚举的值，形如：
// _enum = Enum::ONE | Enum::TWO
template<typename _Enum, typename _Arg>
auto print_optional_enum(_Arg _enum);

using std::string_view;
using std::array;
using std::index_sequence;
using std::make_index_sequence;
using std::cout;
using std::endl;
using std::max;


#ifndef ENUM_STRINGIFY_DISABLE_NAMESPACE
#ifdef ENUM_STRINGIFY_ENABLE_NAMESPACE_CONTROL
bool disable_namespace = true;
#endif // ENUM_STRINGIFY_ENABLE_NAMESPACE_CONTROL
#endif // ENUM_STRINGIFY_DISABLE_NAMESPACE


// 核心操作
// 裁剪函数信息以获取枚举名
template<auto _Enum>
inline constexpr auto enum_name()
{
#ifdef _MSC_VER
    constexpr string_view str = __FUNCSIG__;

#ifdef ENUM_STRINGIFY_DISABLE_NAMESPACE
    // 当enum不属于namespace时，last_of_namespace为namespace YukinaSora的位置
    // 形如：auto __cdecl YukinaSora::enum_name<ONE>(void)
    // 此时截取<>之间的内容即可
    // 
    // 当enum属于namespace时，last_of_namespace为namespace的最后一个字符的位置
    // 形如：auto __cdecl YukinaSora::enum_name<Enum::THREE>(void)
    // 此时截取最后一个::与>之间的内容

    constexpr auto start_of_namespace = str.find_last_of("::") + 1;
    constexpr auto start_of_param     = str.find_last_of("<")  + 1;
    constexpr auto start = max(start_of_namespace, start_of_param);
#else
    constexpr auto start = str.find_last_of("<") + 1;
#endif // ENUM_STRINGIFY_DISABLE_NAMESPACE
    constexpr auto end   = str.find_last_of(">");


#elif __GNUC__
    constexpr string_view str = __PRETTY_FUNCTION__;

#ifdef ENUM_STRINGIFY_DISABLE_NAMESPACE
    // GNU下的格式
    // constexpr auto YukinaSora::enum_name() [with auto _Enum = Enum::ZERO]
    // constexpr auto YukinaSora::enum_name() [with auto _Enum = ONE]

    constexpr auto start_of_namespace = str.find_last_of("::") + 1;
    constexpr auto start_of_param     = str.find_last_of("=")  + 2;
    constexpr auto start = max(start_of_namespace, start_of_param);
#else
    constexpr auto start = str.find_last_of("=") + 2;
#endif // ENUM_STRINGIFY_DISABLE_NAMESPACE
    constexpr auto end   = str.length() - 1;

#endif // _MSC_VER
    return string_view{ str.data() + start, end - start };
}


// 对于一般枚举类型的处理

// 获取枚举类型的最大值
//template<typename _Enum, size_t N = 0>
//inline constexpr size_t enum_size()
//{
//    constexpr auto value = static_cast<_Enum>(N);
//    constexpr auto RN = N + 1;
//    if constexpr (enum_name<value>().find("(") == string_view::npos)
//        return RN ? enum_size<_Enum, RN>() : enum_size<_Enum, 1>();
//    return N;
//}


// 获取枚举类型的最大值
#define enum_stringify_enum_size                                                        \
    []<typename _enum_size_Enum, size_t _enum_size_N = 0>(this auto&& self) -> size_t   \
    {                                                                                   \
        constexpr auto value = static_cast<_enum_size_Enum>(_enum_size_N);              \
		constexpr auto RN = _enum_size_N + 1;                                           \
		if constexpr (enum_name<value>().find("(") == string_view::npos)                \
			return RN ?                                                                 \
                self.template operator()<_enum_size_Enum, RN>() :                       \
                self.template operator()<_enum_size_Enum, 1>();                         \
		return _enum_size_N;                                                            \
    }.template operator()


// 编译时生成枚举名表
template<typename _Enum>
inline constexpr auto enum_name(_Enum value)
{
    constexpr auto size = enum_stringify_enum_size<_Enum>();
    constexpr auto names = []<size_t ...Indexs>(index_sequence<Indexs...>) {
        return array<string_view, size>{
            enum_name<static_cast<_Enum>(Indexs)>()...
        };
    }(make_index_sequence<size>());

    return names[static_cast<size_t>(value)];
}


// 打印枚举名(支持一般枚举类型)
template<typename _Enum, typename _Arg>
inline auto print_enum(_Arg _enum)
{
    auto value = static_cast<_Enum>(_enum);
    cout << enum_name(value) << endl;
}


// 打印枚举名(支持枚举类类型)
template<typename _Enum>
inline auto print_enum(_Enum _enum)
{
    cout << enum_name(_enum) << endl;
}


// 对于可选枚举类型的处理

// 获取可选枚举类型的最大值
//template<typename _Enum, size_t N = 0>
//inline constexpr size_t optional_enum_size()
//{
//    constexpr auto value = static_cast<_Enum>(N);
//    constexpr auto RN = N << 1;
//    if constexpr (enum_name<value>().find("(") == string_view::npos)
//        return RN ? optional_enum_size<_Enum, RN>() : optional_enum_size<_Enum, 1>();
//    return N;
//}


// 获取可选枚举类型的最大值
#define enum_stringify_optional_enum_size \
	[]<typename __lambda_Enum, size_t __lambda_N = 0>(this auto&& self) -> size_t\
	{\
		constexpr auto value = static_cast<__lambda_Enum>(__lambda_N);\
		constexpr auto RN = __lambda_N << 1;\
		if constexpr (enum_name<value>().find("(") == string_view::npos)\
			return RN ?                                                 \
				self.template operator()<__lambda_Enum, RN>() :   \
				self.template operator()<__lambda_Enum, 1>();     \
		return __lambda_N;\
	}.template operator()


// 通过右移操作获取枚举值的最大值
//inline constexpr auto lsr_of(auto _value)
//{
//    size_t ret = 0;
//    size_t value = static_cast<size_t>(_value);
//    while (value = (value >> 1)) ret++;
//    return ret;
//}


// 通过右移操作获取枚举值的最大值
#define enum_stringify_lsr_of \
	[](auto _value) -> size_t\
	{\
		size_t ret = 0;\
		size_t value = static_cast<size_t>(_value);\
		while (value = (value >> 1)) ret++;\
		return ret;\
	}


// 编译时生成枚举名表，运行时查表获取枚举名
template<typename _Enum>
inline constexpr auto optional_enum_name(_Enum value)
{
    constexpr auto size = enum_stringify_lsr_of(enum_stringify_optional_enum_size<_Enum>());
    constexpr auto names = []<size_t ...Indexs>(index_sequence<Indexs...>) {
        return array<string_view, size>{
            enum_name<static_cast<_Enum>(1 << Indexs)>()...
        };
    }(make_index_sequence<size>());

    return names[static_cast<size_t>(enum_stringify_lsr_of(value))];
}


// 打印所有可选枚举名
template<typename _Enum, typename _Arg>
inline auto print_optional_enum(_Arg _enum)
{
    auto value = static_cast<_Enum>(_enum);
    bool first = true;
    for (size_t i = 1; i < value; i = i << 1) {
        if ((value & i) != i) continue;
        if (!first) cout << " | ";
        cout << optional_enum_name(static_cast<_Enum>(i));
        first = false;
    }
    cout << endl;
}

} // namespace YukinaSora

// 隐藏不必要向外的方法
#undef YukinaSora_enum_size
#undef YukinaSora_optional_enum_size
#undef YukinaSora_lsr_of

#endif // !ENUM_STRINGIFY_H