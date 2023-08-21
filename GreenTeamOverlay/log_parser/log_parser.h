#pragma once
#include "file_hook.h"

namespace Green
{


struct EventData
{
    std::wstring eventType;
    std::wstring eventData;
};

class LogParser
{
private:
	FileHook fileHooks;
	std::unordered_map<std::wstring, std::vector<std::function<void(const EventData&)>>> eventHandlers;
	std::chrono::time_point<std::chrono::steady_clock> lastUpdateTime;
	std::wstring lastLine;

public:
	LogParser(const std::wstring& logPath);
	~LogParser();

	void subscribe(const std::wstring& eventType, const std::function<void(const EventData&)>& handler);
	void subscribe(const std::vector<std::wstring>& eventTypes, const std::function<void(const EventData&)>& handler);
	void subscribeAll(const std::function<void(const EventData&)>& handler);

	void captureEvent(const std::wstring& eventType, const std::wstring& eventData);
	void callEventHandlers(const std::wstring& eventType, const EventData& eventData);

	bool isStartOfCombatLogEvent(const std::wstring& line) const;
};

}
