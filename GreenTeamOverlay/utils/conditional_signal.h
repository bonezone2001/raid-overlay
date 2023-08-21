#pragma once
#include <mutex>

class ConditionalSignal {
public:
    ConditionalSignal() : signaled(false) {}

    void wait() {
        std::unique_lock lock(mutex);
        while (!signaled) {
            condition.wait(lock);
        }
        signaled = false;
    }

    void set() {
        {
            std::lock_guard lock(mutex);
            signaled = true;
        }
        condition.notify_one();
    }

private:
    std::mutex mutex;
    std::condition_variable condition;
    bool signaled;
};
