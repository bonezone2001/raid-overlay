#define _CRT_SECURE_NO_WARNINGS

#include "utils.h"
#include <fstream>
#include <iostream>
#include <shlwapi.h>
#include <Psapi.h>
#include "../core/settings.h"

#pragma comment(lib, "Shlwapi.lib")

namespace Green::Utils
{
	bool SaveFileToDisk(const std::string& path, const char* data, const size_t length)
	{
		// Open file
		std::ofstream file(path);
		if (!file.is_open()) return false;

		// Write content
		file.write(data, length);
		file.close();
		return true;
	}

	std::vector<unsigned char> ReadFileContents(const std::string& path)
	{
		// Open file
		std::ifstream ifs(path, std::ios::binary | std::ios::ate);
		if (!ifs) throw std::runtime_error(std::string(path) + ": " + std::strerror(errno));

		// Get size of file
		const auto end = ifs.tellg();
		ifs.seekg(0, std::ios::beg);
		const auto size = static_cast<std::size_t>(end - ifs.tellg());

		// Make sure we have a file that isn't empty
		if (size == 0)
			return {};

		// Read into buffer
		std::vector<unsigned char> buffer(size);
		if (!ifs.read(reinterpret_cast<char*>(buffer.data()), buffer.size()))
			throw std::runtime_error(std::string(path) + ": " + std::strerror(errno));

		// Return file contents
		return buffer;
	}

	std::wstring getLogFileLocation(HWND wowWindow)
	{
		const auto& settings = Settings::getInstance();
		std::wstring logFolderLocation;
		std::wstring logFileName;

		if (settings.wow_log_folder_override.empty())
		{
			std::wcout << "Finding WoW log folder..." << std::endl;
			DWORD wowProcessId;
			GetWindowThreadProcessId(wowWindow, &wowProcessId);
			const HANDLE wowProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, wowProcessId);
			if (wowProcess == NULL)
			{
				MessageBox(NULL, L"Failed to query wow process (is it open?)", L"Error", MB_ICONERROR);
				exit(1);
			}

			// Get the path of the wow process
			wchar_t wowProcessPath[MAX_PATH];
			if (GetModuleFileNameEx(wowProcess, NULL, wowProcessPath, MAX_PATH) == 0)
			{
				MessageBox(NULL, L"Failed to get World of Warcraft process path!", L"Error", MB_ICONERROR);
				exit(1);
			}

			// Get the directory of the wow process
			wchar_t wowProcessDir[MAX_PATH];
			wcscpy_s(wowProcessDir, wowProcessPath);
			PathRemoveFileSpec(wowProcessDir);

			// Get log folder location
			logFolderLocation = wowProcessDir;
			logFolderLocation += L"\\Logs\\";
		}
		else
		{
			logFolderLocation = settings.wow_log_folder_override;
			std::ranges::replace(logFolderLocation, L'/', L'\\');
			if (logFolderLocation.back() != L'\\')
				logFolderLocation += L"\\";
		}

		// Find most recent log file based on last modified date
		if (settings.log_file_name.empty())
		{
			WIN32_FIND_DATA findData;
			const HANDLE hFind = FindFirstFile((logFolderLocation + L"\\WoWCombatLog*.txt").c_str(), &findData);
			if (hFind == INVALID_HANDLE_VALUE)
			{
				MessageBox(NULL, L"Failed to find WoWCombatLog*.txt file!", L"Error", MB_ICONERROR);
				exit(1);
			}

			std::wstring mostRecentLogFile = findData.cFileName;
			FILETIME mostRecentLogFileTime = findData.ftLastWriteTime;
			while (FindNextFile(hFind, &findData))
			{
				if (CompareFileTime(&findData.ftLastWriteTime, &mostRecentLogFileTime) > 0)
				{
					mostRecentLogFile = findData.cFileName;
					mostRecentLogFileTime = findData.ftLastWriteTime;
				}
			}
			FindClose(hFind);
			logFileName = mostRecentLogFile;
		}

		return logFolderLocation + logFileName;
	}

	std::string ToUtf8(const std::wstring& wStr) {
		if (wStr.empty()) return {};

		const int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wStr.c_str(), static_cast<int>(wStr.size()), nullptr, 0, nullptr, nullptr);
		if (sizeNeeded == 0)
			throw std::runtime_error("WideCharToMultiByte failed");

		std::string utf8Str(sizeNeeded, 0);
		if (WideCharToMultiByte(CP_UTF8, 0, wStr.c_str(), static_cast<int>(wStr.size()), utf8Str.data(), sizeNeeded, nullptr, nullptr) == 0)
			throw std::runtime_error("WideCharToMultiByte failed");

		return utf8Str;
	}

	std::wstring FromUtf8(const std::string& utf8Str) {
		if (utf8Str.empty()) return {};

		const int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), static_cast<int>(utf8Str.size()), nullptr, 0);
		if (sizeNeeded == 0)
			throw std::runtime_error("MultiByteToWideChar failed");

		std::wstring wStr(sizeNeeded, 0);
		if (MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), static_cast<int>(utf8Str.size()), wStr.data(), sizeNeeded) == 0) {
			throw std::runtime_error("MultiByteToWideChar failed");
		}

		return wStr;
	}

	void RegexSplit(const std::string& input, const std::regex& sepRegex, std::back_insert_iterator<std::vector<std::string>> resultIter)
	{
		std::sregex_token_iterator iter(input.begin(), input.end(), sepRegex, -1);
		std::sregex_token_iterator end;
		for (; iter != end; ++iter) {
			*resultIter = *iter;
			++resultIter;
		}
	}

	// Convert the RegexSplit function to wstring
	void RegexSplit(const std::wstring& input, const std::wregex& sepRegex, std::back_insert_iterator<std::vector<std::wstring>> resultIter)
	{
		std::wsregex_token_iterator iter(input.begin(), input.end(), sepRegex, -1);
		std::wsregex_token_iterator end;
		for (; iter != end; ++iter) {
			*resultIter = *iter;
			++resultIter;
		}
	}

	std::wstring ToLower(const std::wstring& str)
	{
		std::wstring result = str;
		std::ranges::transform(result, result.begin(), ::tolower);
		return result;
	}

	void Restart()
	{
		// Restart the application
		STARTUPINFO info = { sizeof(info) };
		PROCESS_INFORMATION processInfo;
		if (CreateProcess(nullptr, GetCommandLineW(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &info, &processInfo))
		{
			CloseHandle(processInfo.hProcess);
			CloseHandle(processInfo.hThread);
			ExitProcess(0);
		}

		// If we get here, we failed to restart
		throw std::runtime_error("Failed to restart application");
	}
}
