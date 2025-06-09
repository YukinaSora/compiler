#include "Utils.h"

#include <iostream>
#include <comdef.h>
#include <string>

bool SetConsoleFont(const wchar_t* fontName, int fontSize) {
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(cfi);
	cfi.nFont = 0;
	cfi.dwFontSize.X = 0;
	cfi.dwFontSize.Y = fontSize;
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;
	wcscpy_s(cfi.FaceName, fontName);
	return SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi) != 0;
}

// 设置控制台字体大小
bool SetConsoleFontSize(int size) {
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(cfi);
	cfi.nFont = 0;
	cfi.dwFontSize.X = 0;  // 宽度设为0表示使用默认值
	cfi.dwFontSize.Y = size;
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;
	wcscpy_s(cfi.FaceName, L"SimSun");  // 设置为宋体
	return SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
}

// 启用真彩色支持
bool EnableTrueColor() {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	if (!GetConsoleMode(hConsole, &dwMode)) return false;
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING; // 启用虚拟终端
	return SetConsoleMode(hConsole, dwMode);
}

void InitConsole() {
	// 设置控制台输出为 UTF-8 编码
	SetConsoleOutputCP(CP_UTF8);

	// 启用控制台的 VT100 转义序列支持（可选，用于颜色显示）
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(hOut, dwMode);

	// 设置为支持 Unicode 的字体（如 SimHei 或 Microsoft YaHei）
	SetConsoleFont(L"宋体");
	SetConsoleFontSize(16);

	if (!EnableTrueColor()) {
		std::cout << "当前运行环境不支持真彩色模式\n";
	}

	std::ios::sync_with_stdio(false);
	std::cin.tie(0);
	std::cout.tie(0);
}

void WPrint(const wchar_t* text) {
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD written;
	WriteConsoleW(hOut, text, static_cast<size_t>(wcslen(text)), &written, NULL);
}

void SetRGBColor(int r, int g, int b, int bgR, int bgG, int bgB) {
	std::wcout << L"\033[38;2;" << r << L";" << g << L";" << b << L"m";    // 前景色
	std::wcout << L"\033[48;2;" << bgR << L";" << bgG << L";" << bgB << L"m"; // 背景色
}

std::wstring StringToWString(std::string_view utf8_sv) {
#ifdef _WIN32
	int wchar_count = MultiByteToWideChar(CP_UTF8, 0, utf8_sv.data(),
		static_cast<int>(utf8_sv.size()), nullptr, 0);
	if (wchar_count == 0) throw std::runtime_error("计算宽字符数失败");

	std::wstring result(wchar_count, L'\0');
	if (MultiByteToWideChar(CP_UTF8, 0, utf8_sv.data(),
		static_cast<int>(utf8_sv.size()), &result[0], wchar_count) == 0) {
		throw std::runtime_error("UTF-8转UTF-16失败");
	}
	return result;
#else
	throw std::runtime_error("此函数仅支持Windows");
#endif
}