//
// ��ѡ��ѡ�
// 
// ע����ѡ����ڵ�һ��includeʱ��Ч
// 
// ENUM_STRINGIFY_ENABLE_NAMESPACE
//      �Ƿ����namespace��Ϣ��Ĭ��Ϊ��
//
// ENUM_STRINGIFY_ENABLE_NAMESPACE_CONTROL
//      ע������ENUM_STRINGIFY_ENABLE_NAMESPACE����ʱ��Ч��
//      ����ʱ����ʹ��disable_enum_namespace��������namespace��Ϣ�������

#ifndef ENUM_STRINGIFY_H
#define ENUM_STRINGIFY_H


#include <iostream>
#include <array>

namespace YukinaSora {

// ������ͨ��ģ���ȡö�ٶ�������
// �ú�������������⿪��
template<auto _Enum>
constexpr auto enum_name();


// ͨ������������ȡö�ٶ��������
// ����/�洢����ע�⣺
//      �ú������ڱ���ʱ������ö�����͵����Ʊ�
//      ����ÿ��ö�����ͽ��Ὠ��һ�Σ����к���ͬ��
template<typename _Enum>
constexpr auto enum_name(_Enum value);


// ��ӡö����(֧��һ��ö�����ͣ���Ҫ��ʽָ��ö������)
template<typename _Enum, typename _Arg>
auto print_enum(_Arg _enum);


// ��ӡö����(֧��ö��������)
template<typename _Enum>
auto print_enum(_Enum _enum);


// ��ȡѡ��ö�ٶ��������
// ��ö�ٶ����ֵӦΪ2���ݣ��������磺
// enum Enum : int { ZERO = 1, ONE = 2, TWO = 4, THREE = 8 };
template<typename _Enum>
constexpr auto optional_enum_name(_Enum value);


// ��ӡѡ��ö���е�����ѡ����(֧��һ��ö������)
// _enumӦΪѡ��ö�ٵ�ֵ�����磺
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


// ���Ĳ���
// �ü�������Ϣ�Ի�ȡö����
template<auto _Enum>
inline constexpr auto enum_name()
{
#ifdef _MSC_VER
    constexpr string_view str = __FUNCSIG__;

#ifdef ENUM_STRINGIFY_DISABLE_NAMESPACE
    // ��enum������namespaceʱ��last_of_namespaceΪnamespace YukinaSora��λ��
    // ���磺auto __cdecl YukinaSora::enum_name<ONE>(void)
    // ��ʱ��ȡ<>֮������ݼ���
    // 
    // ��enum����namespaceʱ��last_of_namespaceΪnamespace�����һ���ַ���λ��
    // ���磺auto __cdecl YukinaSora::enum_name<Enum::THREE>(void)
    // ��ʱ��ȡ���һ��::��>֮�������

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
    // GNU�µĸ�ʽ
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


// ����һ��ö�����͵Ĵ���

// ��ȡö�����͵����ֵ
//template<typename _Enum, size_t N = 0>
//inline constexpr size_t enum_size()
//{
//    constexpr auto value = static_cast<_Enum>(N);
//    constexpr auto RN = N + 1;
//    if constexpr (enum_name<value>().find("(") == string_view::npos)
//        return RN ? enum_size<_Enum, RN>() : enum_size<_Enum, 1>();
//    return N;
//}


// ��ȡö�����͵����ֵ
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


// ����ʱ����ö������
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


// ��ӡö����(֧��һ��ö������)
template<typename _Enum, typename _Arg>
inline auto print_enum(_Arg _enum)
{
    auto value = static_cast<_Enum>(_enum);
    cout << enum_name(value) << endl;
}


// ��ӡö����(֧��ö��������)
template<typename _Enum>
inline auto print_enum(_Enum _enum)
{
    cout << enum_name(_enum) << endl;
}


// ���ڿ�ѡö�����͵Ĵ���

// ��ȡ��ѡö�����͵����ֵ
//template<typename _Enum, size_t N = 0>
//inline constexpr size_t optional_enum_size()
//{
//    constexpr auto value = static_cast<_Enum>(N);
//    constexpr auto RN = N << 1;
//    if constexpr (enum_name<value>().find("(") == string_view::npos)
//        return RN ? optional_enum_size<_Enum, RN>() : optional_enum_size<_Enum, 1>();
//    return N;
//}


// ��ȡ��ѡö�����͵����ֵ
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


// ͨ�����Ʋ�����ȡö��ֵ�����ֵ
//inline constexpr auto lsr_of(auto _value)
//{
//    size_t ret = 0;
//    size_t value = static_cast<size_t>(_value);
//    while (value = (value >> 1)) ret++;
//    return ret;
//}


// ͨ�����Ʋ�����ȡö��ֵ�����ֵ
#define enum_stringify_lsr_of \
	[](auto _value) -> size_t\
	{\
		size_t ret = 0;\
		size_t value = static_cast<size_t>(_value);\
		while (value = (value >> 1)) ret++;\
		return ret;\
	}


// ����ʱ����ö����������ʱ����ȡö����
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


// ��ӡ���п�ѡö����
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

// ���ز���Ҫ����ķ���
#undef YukinaSora_enum_size
#undef YukinaSora_optional_enum_size
#undef YukinaSora_lsr_of

#endif // !ENUM_STRINGIFY_H