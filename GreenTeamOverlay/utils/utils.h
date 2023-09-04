#pragma once

#include <regex>
#include <string>
#include <vector>
#include <windows.h>

#ifdef _DEBUG
	#define ALLOC_DEBUG_CONSOLE() \
	    AllocConsole(); \
	    FILE* fDummy; \
	    freopen_s(&fDummy, "CONIN$", "r", stdin); \
	    freopen_s(&fDummy, "CONOUT$", "w", stderr); \
	    freopen_s(&fDummy, "CONOUT$", "w", stdout);
#else
	#define ALLOC_DEBUG_CONSOLE()
#endif

#ifdef _DEBUG
	#define DEBUG_PRINT(x) std::cout << x
	#define DEBUG_PRINTLN(x) std::cout << x << std::endl
	#define WDEBUG_PRINT(x) std::wcout << x
	#define WDEBUG_PRINTLN(x) std::wcout << x << std::endl
#else
	#define DEBUG_PRINT(x)
	#define DEBUG_PRINTLN(x)
	#define WDEBUG_PRINT(x)
	#define WDEBUG_PRINTLN(x)
#endif

namespace Green::Utils
{
	bool SaveFileToDisk(const std::string& path, const char* data, size_t length);
	std::vector<unsigned char> ReadFileContents(const std::string& path);

	std::wstring getLogFileLocation(HWND wowWindow);
	std::string ToUtf8(const std::wstring& wStr);
	std::wstring FromUtf8(const std::string& utf8Str);
	void RegexSplit(const std::string& input, const std::regex& sepRegex, std::back_insert_iterator<std::vector<std::string>> resultIter);
	void RegexSplit(const std::wstring& input, const std::wregex& sepRegex, std::back_insert_iterator<std::vector<std::wstring>> resultIter);
	std::wstring ToLower(const std::wstring& str);
	std::wstring GetClipboardText();
	void Restart();
}
