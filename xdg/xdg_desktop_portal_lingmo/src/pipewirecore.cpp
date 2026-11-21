#include "pipewirecore.h"
#include "requestobject.h"

#include "base/event_loop.h"
#include "base/logging.h"

#include <pipewire/pipewire.h>
#include <sys/epoll.h>

using namespace Lingmo;

static const pw_core_events coreEvents = [] {
    pw_core_events events {};
    events.version = PW_VERSION_CORE_EVENTS;
    events.error = [](void *data, uint32_t id, int, int result, const char *message) {
        if (id != PW_ID_CORE)
            return;
        LM_WARNING(base::LogCategory::defaultCategory()) << "PipeWire core error" << result << message;
        // The connection is unusable; tear it down outside the callback.
        static_cast<PipeWireCore *>(data)->failLater();
    };
    return events;
}();

PipeWireCore::PipeWireCore()
    : m_eventLoop(base::EventLoop::current())
{
    pw_init(nullptr, nullptr);
}

PipeWireCore::~PipeWireCore()
{
    disconnect();
    pw_deinit();
}

bool PipeWireCore::connectToDaemon()
{
    if (m_core)
        return true;
    m_loop = pw_loop_new(nullptr);
    m_context = m_loop ? pw_context_new(m_loop, nullptr, 0) : nullptr;
    m_core = m_context ? pw_context_connect(m_context, nullptr, 0) : nullptr;
    if (!m_core) {
        LM_WARNING(base::LogCategory::defaultCategory()) << "Cannot connect to PipeWire for screen casting";
        disconnect();
        return false;
    }
    m_listener = new spa_hook {};
    pw_core_add_listener(m_core, m_listener, &coreEvents, this);
    pw_loop_enter(m_loop);
    m_watch = m_eventLoop->watch(pw_loop_get_fd(m_loop), EPOLLIN, [this](uint32_t) { iterate(); });
    return true;
}

void PipeWireCore::iterate()
{
    if (!m_loop || m_iterating)
        return;
    m_iterating = true;
    const int result = pw_loop_iterate(m_loop, 0);
    m_iterating = false;
    if (result < 0)
        fail();
}

void PipeWireCore::fail()
{
    if (!m_loop)
        return;
    // Streams destroy themselves against a live core before it goes away.
    disconnected.emit();
    disconnect();
}

void PipeWireCore::failLater()
{
    later(m_alive, [this] { fail(); });
}

void PipeWireCore::disconnect()
{
    if (!m_loop)
        return;
    if (m_watch) {
        m_eventLoop->unwatch(m_watch);
        m_watch = 0;
    }
    if (m_listener) {
        spa_hook_remove(m_listener);
        delete m_listener;
        m_listener = nullptr;
    }
    if (m_core) {
        pw_core_disconnect(m_core);
        m_core = nullptr;
    }
    if (m_context) {
        pw_context_destroy(m_context);
        m_context = nullptr;
    }
    pw_loop_leave(m_loop);
    pw_loop_destroy(m_loop);
    m_loop = nullptr;
}
