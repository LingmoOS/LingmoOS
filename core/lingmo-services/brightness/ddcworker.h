
#include <condition_variable>
#include <cstdint>
#pragma once
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "base/event_loop.h"

struct DdcDisplay {
    std::string connector;
    std::string device;
    int value = 0;
    int maximum = 0;
};

// Blocking DDC/CI I/O; runs on its own thread because each transaction takes tens of ms.
class DdcWorker
{
public:
    using Probed = std::function<void(uint64_t generation, const std::vector<DdcDisplay> &displays)>;

    // `probed` runs on the loop of the thread that made the worker.
    explicit DdcWorker(Probed probed);
    // Waits for the transaction under way; queued work is dropped.
    ~DdcWorker();

    DdcWorker(const DdcWorker &) = delete;
    DdcWorker &operator=(const DdcWorker &) = delete;

    // Only the latest value per device is written.
    void requestValue(const std::string &device, int value);
    void probe(uint64_t generation, std::vector<DdcDisplay> candidates);

private:
    void run();
    void post(std::function<void()> task);
    void flush();

    Probed m_probed;
    Lingmo::Base::LoopHandle m_loop;
    // Lives on the owner's thread, as long as the worker.
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
    std::mutex m_mutex;
    std::condition_variable m_wake;
    std::deque<std::function<void()>> m_tasks;
    std::map<std::string, int> m_pending;
    bool m_flushQueued = false;
    bool m_stopping = false;
    std::thread m_thread;
};

