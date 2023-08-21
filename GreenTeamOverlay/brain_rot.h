#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <windows.h>

namespace Green
{

class LogParser;
class EchoFight;

class BrainRot
{
private:
    std::shared_ptr <EchoFight> curFight;
    std::shared_ptr<LogParser> logParser;
    std::wstring logFilePath;

    std::atomic<bool> running;
    HWND wowWindow;

    // Private constructor and destructor to prevent external instantiation and deletion
    BrainRot();

public:
    // Delete the copy constructor and copy assignment operator
    BrainRot(const BrainRot&) = delete;
    BrainRot& operator=(const BrainRot&) = delete;

    // Get the singleton instance
    static BrainRot& getInstance()
    {
        static BrainRot instance;  // This is thread-safe in C++11 and later
        return instance;
    }

    void init();
    void start();
    void stop();

public:
    std::shared_ptr<LogParser> getLogParser() const { return logParser; }
    std::shared_ptr<EchoFight> getCurFight() const { return curFight; }
    void setCurFight(const std::shared_ptr<EchoFight>& fight) { curFight = fight; }
};

}
