#include "file_hook.h"

#include <fstream>
#include <ranges>
#include <utility>
#include <filesystem>
#include "../utils/utils.h"

namespace Green
{

FileHook::~FileHook()
{
	unhookAll();
}

bool FileHook::hook(const std::wstring& filePath, const std::function<void(const std::wstring&)>& callback)
{
	if (std::filesystem::exists(filePath)) {
		std::lock_guard lock(hooksMutex);

		// Setup the hook
		const auto info = std::make_shared<HookInfo>();
		hooks[filePath] = info;
		info->lastSize = std::filesystem::file_size(filePath);
		info->callback = callback;
		info->monitorThread = std::thread(&FileHook::monitor, this, filePath);
		info->filePath = filePath;

		// Setup the file watcher
		info->watcher = new filewatch::FileWatch<std::wstring>(
			std::wstring(filePath.begin(), filePath.end()),
			[info, this](const std::wstring&, const filewatch::Event changeType) {
				if (changeType == filewatch::Event::modified)
				{
					std::lock_guard lock2(hooksMutex);
					if (hooks.contains(info->filePath))
						hooks[info->filePath]->modifiedSignal.set();
				}
			}
		);
		return true;
	}
	return false;
}

bool FileHook::unhook(const std::wstring& filePath)
{
	std::lock_guard lock(hooksMutex);
	if (hooks.contains(filePath))
	{
		const auto hookInfo = hooks[filePath];
		hookInfo->shouldMonitor = false;
		hookInfo->modifiedSignal.set();
		if (hookInfo->monitorThread.joinable())
			hookInfo->monitorThread.join();
		hooks.erase(filePath);
		return true;
	}
	return false;
}

bool FileHook::unhookAll()
{
	std::lock_guard lock(hooksMutex);
	for (const auto& hookInfo : hooks | std::views::values)
	{
		hookInfo->shouldMonitor = false;
		hookInfo->modifiedSignal.set();
	}

	for (const auto& hookInfo : hooks | std::views::values)
		if (hookInfo->monitorThread.joinable())
			hookInfo->monitorThread.join();
	return true;
}

void FileHook::monitor(const std::wstring& filePath)
{
	const auto hook = hooks[filePath];
	const std::filesystem::path path(filePath);
	while (hooks[filePath]->shouldMonitor) {
		hook->modifiedSignal.wait();
		if (!hook->shouldMonitor) break;

		const std::uintmax_t currentSize = std::filesystem::file_size(path);
		if (currentSize <= hooks[filePath]->lastSize) continue;

		std::ifstream file(filePath, std::ios::in | std::ios::binary);
		file.seekg(hooks[filePath]->lastSize);

		std::string newData;
		std::getline(file, newData, '\0');

		hooks[filePath]->callback(Utils::FromUtf8(newData));
		hooks[filePath]->lastSize = currentSize;
	}
}

}