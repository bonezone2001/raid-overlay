#pragma once

#include <windows.h>
#include <string>
#include <map>
#include <functional>
#include <mutex>

#include "../utils/conditional_signal.h"
#include <file_watch.hpp>
#include <thread>

namespace Green
{

constexpr int BUFFER_SIZE = 1024;

struct HookInfo {
    std::thread monitorThread;
    bool shouldMonitor = true;
    std::uintmax_t lastSize = -1;
    std::function<void(const std::wstring&)> callback;
    filewatch::FileWatch<std::wstring>* watcher;
    ConditionalSignal modifiedSignal;
    std::wstring filePath;

    ~HookInfo()
    {
        if (watcher != nullptr)
        {
            delete watcher;
			watcher = nullptr;
        }
    }
};

class FileHook
{
private:
    std::mutex hooksMutex;
    std::map<std::wstring, std::shared_ptr<HookInfo>> hooks;

private:
	[[noreturn]] void monitor(const std::wstring& filePath);

public:
    ~FileHook();

    bool hook(const std::wstring& filePath, const std::function<void(const std::wstring&)>& callback);
    bool unhook(const std::wstring& filePath);
    bool unhookAll();
};

}