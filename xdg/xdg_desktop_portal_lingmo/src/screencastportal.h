#pragma once

#include "base/dbus.h"

#include <map>
#include <memory>
#include <string>

class PipeWireCore;
class ScreenCastSession;
class StartRequest;
class WaylandCapture;

class ScreenCastPortal
{
public:
    explicit ScreenCastPortal(Lingmo::base::dbus::Connection *bus);
    ~ScreenCastPortal();

    bool publish();

    uint32_t version() const { return 4; }
    // Monitors and windows; the compositor exposes no virtual sources.
    uint32_t availableSourceTypes() const { return 1 | 2; }
    // Hidden or embedded; cursor metadata is not produced.
    uint32_t availableCursorModes() const { return 1 | 2; }

private:
    friend class ScreenCastSession;
    friend class StartRequest;

    void createSession(Lingmo::base::dbus::MethodCall &call);
    void selectSources(Lingmo::base::dbus::MethodCall &call);
    void start(Lingmo::base::dbus::MethodCall &call);
    ScreenCastSession *session(const std::string &path, const std::string &sender) const;
    // Destroys the session on the next loop turn.
    void deleteLater(ScreenCastSession *session);

    Lingmo::base::dbus::Connection *m_bus;
    Lingmo::base::dbus::Object m_object;
    std::unique_ptr<WaylandCapture> m_wayland;
    std::unique_ptr<PipeWireCore> m_pipewire;
    // After the connections they use, so they go first.
    std::map<std::string, std::unique_ptr<ScreenCastSession>> m_sessions;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};
