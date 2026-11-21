#include "inhibitportal.h"
#include "options.h"
#include "requestobject.h"

#include "base/strings.h"

using namespace Lingmo;
using base::dbus::MethodCall;
using base::dbus::Reply;

class InhibitRequest
{
public:
    InhibitRequest(InhibitPortal *portal, const MethodCall &call, const std::string &path)
        : m_portal(portal)
        , m_bus(portal->m_bus)
        , m_call(call)
        , m_requester(call.message().sender())
        , m_path(path)
    {
    }

    ~InhibitRequest() { release(); }

    // Exported before the reply, as the frontend may close it at once.
    bool exportObject()
    {
        m_object = exportRequest(m_bus, m_path, [this](MethodCall &call) { Close(call); });
        if (!m_object)
            return false;
        m_requesterWatch = watchRequester(m_bus, m_requester, [this] { close(); });
        return true;
    }

    void start(const std::string &appId, const std::string &reason)
    {
        // Asked without blocking, so a busy bus never stalls the other portals; the
        // reply waits for the answer.
        base::dbus::Message ownerCall = m_bus->newMethodCall("org.freedesktop.DBus", "/org/freedesktop/DBus",
                                                             "org.freedesktop.DBus", "GetNameOwner");
        ownerCall.append(std::string("org.freedesktop.ScreenSaver"));
        m_lookup = m_bus->callAsync(ownerCall, [this, appId, reason](const Reply &owner) {
            if (m_closed) {
                m_portal->deleteLater(this);
                return;
            }
            if (owner.isError() || !owner.value<std::string>()) {
                m_call.replyError("org.freedesktop.portal.Error.Failed",
                                  "ScreenSaver inhibition service is unavailable");
                m_object.reset();
                m_portal->deleteLater(this);
                return;
            }
            m_owner = *owner.value<std::string>();
            inhibit(appId, reason);
        });
    }

    void Close(MethodCall &call)
    {
        if (call.message().sender() != m_requester) {
            call.replyError("org.freedesktop.DBus.Error.AccessDenied",
                            "Only the requesting service may close this inhibition");
            return;
        }
        close();
    }

private:
    void inhibit(const std::string &appId, const std::string &reason)
    {
        base::dbus::Message call = m_bus->newMethodCall(m_owner, "/org/freedesktop/ScreenSaver",
                                                        "org.freedesktop.ScreenSaver", "Inhibit");
        call.append(appId, reason);
        m_inhibit = m_bus->callAsync(call, [this](const Reply &reply) {
            m_pending = false;
            const std::optional<uint32_t> cookie = reply.value<uint32_t>();
            if (cookie) {
                m_cookie = *cookie;
                m_acquired = true;
            }
            if (m_closed) {
                release();
                m_portal->deleteLater(this);
                return;
            }
            if (!cookie) {
                if (reply.isError())
                    m_call.replyError(reply.error().name, reply.error().message);
                else
                    m_call.replyError("org.freedesktop.DBus.Error.InvalidSignature",
                                      "Unexpected reply signature: got \"" + reply.message().signature()
                                              + "\", expected \"u\"");
                m_object.reset();
                m_portal->deleteLater(this);
                return;
            }
            m_call.reply();
        });
    }

    void release()
    {
        if (!m_acquired)
            return;
        m_acquired = false;
        // Pin the unique owner so a restarted service cannot receive an old cookie.
        base::dbus::Message call = m_bus->newMethodCall(m_owner, "/org/freedesktop/ScreenSaver",
                                                        "org.freedesktop.ScreenSaver", "UnInhibit");
        call.append(m_cookie);
        m_bus->send(call);
    }

    void close()
    {
        if (m_closed)
            return;
        m_closed = true;
        m_object.reset();
        if (m_pending) {
            m_call.replyError("org.freedesktop.portal.Error.Cancelled", "Inhibition was closed");
            // Kept alive to release a cookie returned after Close.
            return;
        }
        release();
        m_portal->deleteLater(this);
    }

    InhibitPortal *m_portal;
    base::dbus::Connection *m_bus;
    MethodCall m_call;
    std::string m_requester;
    std::string m_path;
    std::string m_owner;
    std::unique_ptr<base::dbus::Object> m_object;
    base::dbus::NameWatch m_requesterWatch;
    base::dbus::PendingCall m_lookup;
    base::dbus::PendingCall m_inhibit;
    uint32_t m_cookie = 0;
    bool m_acquired = false;
    bool m_pending = true;
    bool m_closed = false;
};

InhibitPortal::InhibitPortal(base::dbus::Connection *bus)
    : m_bus(bus)
    , m_object(bus, "/org/freedesktop/portal/desktop", "org.freedesktop.impl.portal.Inhibit")
{
    m_object.addMethod("Inhibit", "ossua{sv}", "", [this](MethodCall &call) { inhibit(call); },
                       {{"handle", "appId", "window", "flags", "options"}, {}});
}

InhibitPortal::~InhibitPortal() = default;

bool InhibitPortal::publish()
{
    return m_object.publish();
}

void InhibitPortal::inhibit(MethodCall &call)
{
    base::dbus::ObjectPath handle;
    std::string appId;
    std::string window;
    uint32_t flags = 0;
    base::dbus::VariantMap options;
    call.read(handle, appId, window, flags, options);

    // Only the idle bit is honoured; the others ride along, e.g. suspend|idle from players.
    if (!(flags & 8)) {
        call.replyError("org.freedesktop.portal.Error.NotSupported", "Only idle inhibition (flag 8) is supported");
        return;
    }
    if (!base::strings::startsWith(handle.path, "/org/freedesktop/portal/desktop/request/")) {
        call.replyError("org.freedesktop.portal.Error.InvalidArgument", "Invalid request path");
        return;
    }
    auto request = std::make_unique<InhibitRequest>(this, call, handle.path);
    if (!request->exportObject()) {
        call.replyError("org.freedesktop.portal.Error.Failed", "Cannot export inhibition request");
        return;
    }
    InhibitRequest *raw = request.get();
    m_requests.emplace(raw, std::move(request));
    raw->start(appId, optionString(options, "reason"));
}

void InhibitPortal::deleteLater(InhibitRequest *request)
{
    later(m_alive, [this, request] { m_requests.erase(request); });
}
