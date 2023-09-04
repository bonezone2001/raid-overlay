#include "settings.h"
#include <filesystem>
#include <iostream>
#include <Windows.h>
#include "../utils/utils.h"

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
	(void)createIfMissing(APP_SETTINGS_FILE_NAME);
	loadSettings(APP_SETTINGS_FILE_NAME);
}

void Settings::setInstance(const Settings& settings)
{
	*this = settings;
}

void Settings::save() const
{
	const nlohmann::json j = *this;
	const std::string data = j.dump(1, '\t');
	Utils::SaveFileToDisk(APP_SETTINGS_FILE_NAME, data.c_str(), data.length());
}

}
