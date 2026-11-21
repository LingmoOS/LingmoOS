#pragma once

#include <memory>

#include "base/signal.h"

// ext_idle_notifier_v1 timeouts, on a Wayland connection of their own.
class WaylandIdle
{
public:
    WaylandIdle();
    ~WaylandIdle();

    WaylandIdle(const WaylandIdle &) = delete;
    WaylandIdle &operator=(const WaylandIdle &) = delete;

    int addTimeout(int milliseconds);
    void removeTimeout(int id);

    Lingmo::Base::Signal<int, int> timeoutReached;
    Lingmo::Base::Signal<> resumed;

private:
    class Private;
    std::unique_ptr<Private> d;
};
