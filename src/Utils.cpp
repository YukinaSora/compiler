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

// ���ÿ���̨�����С
bool SetConsoleFontSize(int size) {
	CONSOLE_FONT_INFOEX cfi;
	cfi.cbSize = sizeof(cfi);
	cfi.nFont = 0;
	cfi.dwFontSize.X = 0;  // �����Ϊ0��ʾʹ��Ĭ��ֵ
	cfi.dwFontSize.Y = size;
	cfi.FontFamily = FF_DONTCARE;
	cfi.FontWeight = FW_NORMAL;
	wcscpy_s(cfi.FaceName, L"SimSun");  // ����Ϊ����
	return SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
}

// �������ɫ֧��
bool EnableTrueColor() {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	if (!GetConsoleMode(hConsole, &dwMode)) return false;
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING; // ���������ն�
	return SetConsoleMode(hConsole, dwMode);
}

void InitConsole() {
	// ���ÿ���̨���Ϊ UTF-8 ����
	SetConsoleOutputCP(CP_UTF8);

	// ���ÿ���̨�� VT100 ת������֧�֣���ѡ��������ɫ��ʾ��
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(hOut, dwMode);

	// ����Ϊ֧�� Unicode �����壨�� SimHei �� Microsoft YaHei��
	SetConsoleFont(L"����");
	SetConsoleFontSize(16);

	if (!EnableTrueColor()) {
		std::cout << "��ǰ���л�����֧�����ɫģʽ\n";
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
	std::wcout << L"\033[38;2;" << r << L";" << g << L";" << b << L"m";    // ǰ��ɫ
	std::wcout << L"\033[48;2;" << bgR << L";" << bgG << L";" << bgB << L"m"; // ����ɫ
}

std::wstring StringToWString(std::string_view utf8_sv) {
#ifdef _WIN32
	int wchar_count = MultiByteToWideChar(CP_UTF8, 0, utf8_sv.data(),
		static_cast<int>(utf8_sv.size()), nullptr, 0);
	if (wchar_count == 0) throw std::runtime_error("������ַ���ʧ��");

	std::wstring result(wchar_count, L'\0');
	if (MultiByteToWideChar(CP_UTF8, 0, utf8_sv.data(),
		static_cast<int>(utf8_sv.size()), &result[0], wchar_count) == 0) {
		throw std::runtime_error("UTF-8תUTF-16ʧ��");
	}
	return result;
#else
	throw std::runtime_error("�˺�����֧��Windows");
#endif
}