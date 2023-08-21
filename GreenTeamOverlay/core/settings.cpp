#include "settings.h"
#include <filesystem>
#include <Windows.h>
#include <iostream>

#include "../utils/utils.h""

#define APP_SETTINGS_FILE_NAME "settings.json"

namespace Green
{

bool Settings::createIfMissing(const std::string& path) const
{
	if (std::filesystem::exists(path))
		return false;

	DEBUG_PRINTLN("Creating settings!");

	// Create settings
	Settings settings;

	// Save settings
	nlohmann::json j = settings;
	const std::string data = j.dump(1, '\t');
	return Utils::SaveFileToDisk(path, data.c_str(), data.length());
}

bool Settings::loadSettings(const std::string& path)
{

	if (!std::filesystem::exists(path))
		return false;

	DEBUG_PRINTLN("Loading settings!");

	// Load data
	const std::vector<unsigned char> file = Utils::ReadFileContents(path);
	const auto fileData = std::string(file.begin(), file.end());

	// Parse then replace current settings
	const nlohmann::json j = nlohmann::json::parse(fileData);
	setInstance(j);
	return true;
}

void Settings::load()
{
	if (createIfMissing(APP_SETTINGS_FILE_NAME))
	{
		MessageBox(NULL, L"Settings file created. Please edit it!", L"BrainRot", MB_OK | MB_ICONINFORMATION);
		exit(0);
	}
	loadSettings(APP_SETTINGS_FILE_NAME);
}

void Settings::setInstance(const Settings& settings)
{
	*this = settings;
}

}
