#pragma once

#include <iostream>
#include <vector>

#define __BuiltInErrorInTrue(condition) \
	([](){ struct _{ int: -!!(condition); }; }())

#define IsSameType(a, b) \
	(typeid(a) == typeid(b))

#define ArrayLength(array) \
	[]() -> size_t {\
		__BuiltInErrorInTrue(IsSameType((array), &(array)[0]));\
		return sizeof((array)) / sizeof((array)[0]);\
	}()

template<class _Ty0, class _Ty1>
constexpr inline bool ArrayContains(const _Ty0* array, size_t length, const _Ty1& value)
{
	for (size_t i = 0; i < length; i++)
		if (array[i] == value) return true;

	return false;
}

template<class _Ty0, class _Ty1>
constexpr inline bool ArrayContains(const std::vector<_Ty0> array, const _Ty1& value)
{
	for (auto& item : array)
		if (item == value) return true;

	return false;
}

constexpr size_t operator "" _KB(unsigned long long size) { return size << 10; }
constexpr size_t operator "" _MB(unsigned long long size) { return size << 20; }


#include <Windows.h>

bool SetConsoleFont(const wchar_t* fontName, int fontSize = 10);
bool SetConsoleFontSize(int size);
bool EnableTrueColor();
void WPrint(const wchar_t* text);
void SetRGBColor(int r, int g, int b, int bgR = 0, int bgG = 0, int bgB = 0);

std::wstring StringToWString(std::string_view utf8_sv);

void InitConsole();