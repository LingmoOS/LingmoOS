#pragma once

#include "base/signal.h"

#include <memory>

struct pw_context;
struct pw_core;
struct pw_loop;
struct spa_hook;

namespace Lingmo::base {
class EventLoop;
}

// One PipeWire connection for all screen cast streams, iterated from the
// portal's event loop instead of a PipeWire thread so stream callbacks stay
// on the thread that owns the Wayland proxies.
class PipeWireCore
{
public:
    PipeWireCore();
    ~PipeWireCore();

    PipeWireCore(const PipeWireCore &) = delete;
    PipeWireCore &operator=(const PipeWireCore &) = delete;

    // Connects on first use; after a daemon error the next call reconnects.
    bool connectToDaemon();
    pw_core *core() const { return m_core; }

    // Drops the connection after notifying streams; the next connectToDaemon()
    // starts over.
    void fail();
    // From inside a PipeWire callback, where the core cannot go yet.
    void failLater();

    Lingmo::base::Signal<> disconnected;

private:
    void iterate();
    void disconnect();

    Lingmo::base::EventLoop *m_eventLoop;
    pw_loop *m_loop = nullptr;
    pw_context *m_context = nullptr;
    pw_core *m_core = nullptr;
    spa_hook *m_listener = nullptr;
    int m_watch = 0;
    bool m_iterating = false;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};
