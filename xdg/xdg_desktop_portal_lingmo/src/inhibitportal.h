#pragma once

#include "base/dbus.h"

#include <map>
#include <memory>

class InhibitRequest;

class InhibitPortal
{
public:
    explicit InhibitPortal(Lingmo::base::dbus::Connection *bus);
    ~InhibitPortal();

    bool publish();

private:
    friend class InhibitRequest;

    void inhibit(Lingmo::base::dbus::MethodCall &call);
    // Destroys the request on the next loop turn.
    void deleteLater(InhibitRequest *request);

    Lingmo::base::dbus::Connection *m_bus;
    Lingmo::base::dbus::Object m_object;
    std::map<InhibitRequest *, std::unique_ptr<InhibitRequest>> m_requests;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};
