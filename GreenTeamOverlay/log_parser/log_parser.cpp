#include "log_parser.h"

#include <numeric>

#include "../utils/utils.h"
namespace Green
{

LogParser::LogParser(const std::wstring& logPath)
{
    lastUpdateTime = std::chrono::steady_clock::now();
    fileHooks.hook(logPath, [this](const std::wstring& text) {
        // Show time since last update
        const auto now = std::chrono::steady_clock::now();
        const auto timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdateTime).count();
        DEBUG_PRINTLN("Time since last update: " << timeSinceLastUpdate << "ms");
        lastUpdateTime = now;

        std::vector<std::wstring> lines;
        Utils::RegexSplit(text, std::wregex(L"[\r\n]+"), std::back_inserter(lines));

        lines.erase(std::ranges::remove_if(lines, [](const std::wstring& line) {
	        return line.empty();
        }).begin(), lines.end());

        if (lines.empty()) return;

        if (!isStartOfCombatLogEvent(lines[0]))
            lines[0] = lastLine + lines[0];
        else if (!lastLine.empty())
            lines.insert(lines.begin(), lastLine);

        lastLine = lines.back();
        lines.pop_back();

        for (const std::wstring& line : lines) {
            if (line.empty()) continue;

            std::vector<std::wstring> sections;
            Utils::RegexSplit(line, std::wregex(L" "), std::back_inserter(sections));
            if (sections.size() < 3) continue;

            const std::wstring dateTime = sections[0] + L" " + sections[1];
            const std::wstring csv = std::accumulate(sections.begin() + 2, sections.end(), std::wstring(),
                [](const std::wstring& lhs, const std::wstring& rhs) {
                    return lhs.empty() ? rhs : lhs + L" " + rhs;
                });

            std::vector<std::wstring> tokens;
            Utils::RegexSplit(csv, std::wregex(L","), std::back_inserter(tokens));

            const std::wstring eventType = tokens[0];
            captureEvent(eventType, csv);
        }
	});
}

LogParser::~LogParser()
{
    fileHooks.unhookAll();
}

void LogParser::subscribe(const std::wstring& eventType, const std::function<void(const EventData&)>& handler)
{
	eventHandlers[eventType].emplace_back(handler);
}

void LogParser::subscribe(const std::vector<std::wstring>& eventTypes, const std::function<void(const EventData&)>& handler)
{
    for (const std::wstring& eventType : eventTypes)
		eventHandlers[eventType].emplace_back(handler);
}

void LogParser::subscribeAll(const std::function<void(const EventData&)>& handler)
{
    eventHandlers[L"ALL"].emplace_back(handler);
}

void LogParser::captureEvent(const std::wstring& eventType, const std::wstring& eventData)
{
	const EventData event = { eventType, eventData };
    callEventHandlers(eventType, event);
}

void LogParser::callEventHandlers(const std::wstring& eventType, const EventData& eventData)
{
    const auto& allHandlers = eventHandlers[L"ALL"];
    const auto& specificHandlers = eventHandlers[eventType];

    for (const auto& handler : allHandlers)
        handler(eventData);

    for (const auto& handler : specificHandlers)
        handler(eventData);
}

bool LogParser::isStartOfCombatLogEvent(const std::wstring& line) const
{
    return std::regex_search(line, std::wregex(LR"(^(?=.*/)[^ ]*)"));
}

}
