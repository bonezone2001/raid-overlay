#define _CRT_SECURE_NO_WARNINGS

// This has to be some of the worst code ever made, however, I don't care.
// I'll maybe refactor this later, MAYBE.

#include <iostream>
#include <fstream>
#include <string>
#include "auto_update.h"

#include <sstream>
#include <curl/curl.h>

#include "utils.h"
#include "../resource.h"
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "version.lib")

namespace Green::Utils
{

std::string FetchVersionFromServer();
bool ReplaceExecutableWithUpdate();
bool DownloadUpdateFile();

const std::string CHECK_UPDATE_URL = "http://brainrot.lootlust.com/check_update";
const std::string UPDATE_FILE_URL = "http://brainrot.lootlust.com/GreenTeamOverlay.exe";

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	const size_t totalSize = size * nmemb;
	auto* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

static size_t WriteCallbackDownload(void* contents, size_t size, size_t nmemb, void* userp) {
	const size_t totalSize = size * nmemb;
    const auto fp = static_cast<FILE*>(userp);
    return fwrite(contents, 1, totalSize, fp);
}

std::string GetCurrentVersion()
{
    // get the filename of the executable containing the version resource
    TCHAR szFilename[MAX_PATH + 1] = { 0 };
    if (GetModuleFileName(NULL, szFilename, MAX_PATH) == 0)
    {
        DEBUG_PRINTLN("GetModuleFileName failed with error " << GetLastError());
        return {};
    }

    // allocate a block of memory for the version info
    DWORD dummy;
    const DWORD dwSize = GetFileVersionInfoSize(szFilename, &dummy);
    if (dwSize == 0)
    {
        DEBUG_PRINTLN("GetFileVersionInfoSize failed with error " << GetLastError());
        return {};
    }

    // load the version info
    std::vector<BYTE> data(dwSize);
    if (!GetFileVersionInfo(szFilename, NULL, dwSize, data.data()))
    {
        DEBUG_PRINTLN("GetFileVersionInfo failed with error " << GetLastError());
        return {};
    }

    // Print file version
    VS_FIXEDFILEINFO* pFileInfo;
    UINT uLen;
    if (!VerQueryValue(data.data(), TEXT("\\"), reinterpret_cast<LPVOID*>(&pFileInfo), &uLen))
	{
		DEBUG_PRINTLN("VerQueryValue failed with error " << GetLastError());
		return {};
	}

    const DWORD dwFileVersionMs = pFileInfo->dwFileVersionMS;
	const DWORD dwFileVersionLs = pFileInfo->dwFileVersionLS;
	const DWORD dwLeftMost = HIWORD(dwFileVersionMs);
	const DWORD dwSecondLeft = LOWORD(dwFileVersionMs);
	const DWORD dwSecondRight = HIWORD(dwFileVersionLs);
	const DWORD dwRightMost = LOWORD(dwFileVersionLs);

	std::stringstream ss;
	ss << dwLeftMost << "." << dwSecondLeft << "." << dwSecondRight << "." << dwRightMost;
	return ss.str();
}

void DeleteOldFiles()
{
    // Delete any .old files in the current directory
    WIN32_FIND_DATA FindFileData;
    const HANDLE hFind = FindFirstFile(L"*.old", &FindFileData);
    if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			DeleteFile(FindFileData.cFileName);
            std::wcout << "Deleted old file: " << FindFileData.cFileName << std::endl;
		} while (FindNextFile(hFind, &FindFileData));
		FindClose(hFind);
	}
}

bool CheckForUpdates()
{
    DeleteOldFiles();
	const std::string currentVersion = GetCurrentVersion();
    DEBUG_PRINTLN("BrainRot v" << currentVersion);

	if (CURL* curl = curl_easy_init())
    {
        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, CHECK_UPDATE_URL.c_str());

        // Provide a callback function to process the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2.5L);

        // Perform the HTTP request
        const CURLcode res = curl_easy_perform(curl);

        // Cleanup / Sanity checks
        curl_easy_cleanup(curl);
        if (res != CURLE_OK)
        {
            DEBUG_PRINTLN("Failed to perform HTTP request: " << curl_easy_strerror(res));
            return false;
        }

        // Remove any trailing newlines or spaces
        response = response.substr(0, response.find_last_not_of(" \n\r\t") + 1);

        DEBUG_PRINTLN("Latest v" << response);
        DEBUG_PRINTLN("Status: " << (response != currentVersion ? "Needs update" : "No update"));
        return response != currentVersion;
    }

    DEBUG_PRINTLN("Failed to initialize CURL");
    return false;
}

bool UpdateInPlace()
{
    // Download the updated executable
    if (!DownloadUpdateFile())
    {
        DEBUG_PRINTLN("Failed to download the update.");
        return false;
    }

    // Replace the current executable with the downloaded update
    if (!ReplaceExecutableWithUpdate())
    {
        DEBUG_PRINTLN("Failed to replace the executable with the update.");
        return false;
    }

    return true;
}

bool DownloadUpdateFile()
{
    if (CURL* curl = curl_easy_init())
    {
	    const std::string outputFilePath = "update.exe";
        FILE* fp = fopen(outputFilePath.c_str(), "wb");
        if (!fp) {
            DEBUG_PRINTLN("Error opening the output file.");
            curl_easy_cleanup(curl);
            return 1;
        }

        curl_easy_setopt(curl, CURLOPT_URL, UPDATE_FILE_URL.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackDownload);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

        const CURLcode res = curl_easy_perform(curl);
        fclose(fp);
        curl_easy_cleanup(curl);
        if (res != CURLE_OK) {
            DEBUG_PRINTLN("Error while downloading update: " << curl_easy_strerror(res));
            return false;
        }

        return true;
    }

    DEBUG_PRINTLN("Failed to initialize CURL");
    return false;
}

bool ReplaceExecutableWithUpdate()
{
    // Get current executable path
    TCHAR szFilename[MAX_PATH + 1] = { 0 };
    if (GetModuleFileName(NULL, szFilename, MAX_PATH) == 0)
	{
		DEBUG_PRINTLN("GetModuleFileName failed with error " << GetLastError());
		return false;
	}

    // Rename current executable to .old
    const std::wstring oldExecutablePath = std::wstring(szFilename) + L".old";
    if (!MoveFileEx(szFilename, oldExecutablePath.c_str(), MOVEFILE_REPLACE_EXISTING))
	{
		DEBUG_PRINTLN("Failed to rename current executable to .old");
		return false;
	}

    // Set file as hidden
    if (!SetFileAttributes(oldExecutablePath.c_str(), FILE_ATTRIBUTE_HIDDEN))
	{
        DEBUG_PRINTLN("Failed to set file as hidden");
        return false;
    }

    // Rename update to current executable
    const std::wstring updateFilePath = L"update.exe";
    if (!MoveFileEx(updateFilePath.c_str(), szFilename, MOVEFILE_REPLACE_EXISTING))
    {
	    DEBUG_PRINTLN("Failed to rename update to current executable");
		return false;
    }

    return true;
}

}