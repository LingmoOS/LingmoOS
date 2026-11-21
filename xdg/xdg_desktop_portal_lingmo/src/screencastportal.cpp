#include "screencastportal.h"
#include "options.h"
#include "pipewirecore.h"
#include "requestobject.h"
#include "screencaststream.h"
#include "screencasttypes.h"
#include "waylandcapture.h"

#include "base/event_loop.h"
#include "base/logging.h"
#include "base/strings.h"

#include <algorithm>
#include <chrono>
#include <climits>
#include <cmath>
#include <vector>

using namespace Lingmo;
using base::dbus::MethodCall;
using base::dbus::ObjectPath;
using base::dbus::Variant;
using base::dbus::VariantMap;

static constexpr uint32_t sourceMonitor = 1;
static constexpr uint32_t sourceWindow = 2;
static constexpr uint32_t cursorHidden = 1;
static constexpr uint32_t cursorEmbedded = 2;
static constexpr uint32_t persistNone = 0;
static constexpr uint32_t persistUntilRevoked = 2;

static const std::string sessionInterface = "org.freedesktop.impl.portal.Session";

static std::vector<std::string> restoredMonitors(const Variant &value)
{
    const std::optional<RestoreData> data = value.get<RestoreData>();
    if (!data || std::get<0>(*data) != "Lingmo" || std::get<1>(*data) != 1)
        return {};
    const VariantMap map = std::get<2>(*data).value<VariantMap>();
    const auto monitors = map.find("monitors");
    return monitors == map.end() ? std::vector<std::string>() : toStringList(monitors->second);
}

static Variant restoreData(const std::vector<std::string> &monitors)
{
    return Variant(RestoreData{"Lingmo", 1, Variant(VariantMap{{"monitors", Variant(monitors)}})});
}

static int framerateFor(const CaptureOutput &output)
{
    return output.refreshMilliHz > 0 ? int(std::ceil(output.refreshMilliHz / 1000.0)) : 60;
}

static const CaptureOutput *findOutput(const std::vector<CaptureOutput> &outputs, const std::string &name)
{
    const auto it = std::find_if(outputs.begin(), outputs.end(), [&](const CaptureOutput &output) {
        return output.name == name;
    });
    return it != outputs.end() ? &*it : nullptr;
}

static base::dbus::Message permissionsCall(base::dbus::Connection *bus, const std::string &method)
{
    return bus->newMethodCall("org.lingmo.Services", "/org/lingmo/Services/Permissions",
                              "org.lingmo.Services.Permissions", method);
}

class ScreenCastSession
{
public:
    ScreenCastSession(ScreenCastPortal *portal, const std::string &path, const std::string &owner,
                      const std::string &appId)
        : portal(portal)
        , bus(portal->m_bus)
        , path(path)
        , owner(owner)
        , appId(appId)
    {
    }

    bool exportObject()
    {
        object = std::make_unique<base::dbus::Object>(bus, path, sessionInterface);
        object->addProperty("version", [] { return uint32_t(1); });
        object->addMethod("Close", "", "", [this](MethodCall &call) {
            Close(call);
            if (!call.isReplied())
                call.reply();
        });
        if (!object->publish())
            return false;
        ownerWatch = watchRequester(bus, owner, [this] { close(false); });
        return true;
    }

    // Frees the exported path and the streams. The frontend learns about a
    // close it did not ask for through Closed.
    void close(bool notify);

    void deleteRequestLater(StartRequest *finished)
    {
        later(alive, [this, finished] { requests.erase(finished); });
    }

private:
    void Close(MethodCall &call)
    {
        if (call.message().sender() != owner) {
            call.replyError("org.freedesktop.DBus.Error.AccessDenied",
                            "Only the requesting service may close this session");
            return;
        }
        close(false);
    }

public:
    ScreenCastPortal *portal;
    base::dbus::Connection *bus;
    std::string path;
    std::string owner;
    std::string appId;
    std::unique_ptr<base::dbus::Object> object;
    base::dbus::NameWatch ownerWatch;
    uint32_t cursorMode = cursorHidden;
    uint32_t persistMode = persistNone;
    bool started = false;
    bool closed = false;
    uint32_t types = sourceMonitor;
    std::vector<std::string> restore;
    std::vector<std::unique_ptr<ScreenCastStream>> streams;
    StartRequest *request = nullptr;
    std::shared_ptr<bool> alive = std::make_shared<bool>(true);
    // Last, so they go first while the session is whole.
    std::map<StartRequest *, std::unique_ptr<StartRequest>> requests;
};

// The Start request: the permission prompt, then the streams, then one reply.
class StartRequest
{
public:
    StartRequest(ScreenCastPortal *portal, const MethodCall &call, const std::string &path,
                 ScreenCastSession *session)
        : bus(portal->m_bus)
        , call(call)
        , requester(call.message().sender())
        , path(path)
        , session(session)
        , wayland(portal->m_wayland.get())
        , pipewire(portal->m_pipewire.get())
    {
        session->request = this;
    }

    ~StartRequest()
    {
        if (session->request == this)
            session->request = nullptr;
    }

    bool exportObject()
    {
        object = exportRequest(bus, path, [this](MethodCall &close) { Close(close); });
        return object != nullptr;
    }

    void run(const std::string &parentWindow)
    {
        if (!wayland->connectToDisplay() || !wayland->captureAvailable()) {
            LM_WARNING(base::LogCategory::defaultCategory()) << "Screen capture is unavailable on this Wayland display";
            finish(2);
            return;
        }
        const auto outputs = wayland->outputs();
        if (outputs.empty()) {
            finish(2);
            return;
        }
        const auto known = [&](const std::string &name) {
            return findOutput(outputs, name) != nullptr;
        };
        // The only thing that skips the chooser is the application's own
        // restore token naming monitors that still exist. Windows are chosen
        // afresh every time, their identifiers being good for one lifetime.
        if ((session->types & sourceMonitor) && !session->restore.empty()
            && std::all_of(session->restore.begin(), session->restore.end(), known)) {
            std::vector<CaptureTarget> targets;
            for (const std::string &name : session->restore)
                targets.push_back({CaptureTarget::Kind::Monitor, name});
            startStreams(targets, session->persistMode);
            return;
        }
        std::vector<Variant> windows;
        if (session->types & sourceWindow) {
            for (const auto &toplevel : wayland->toplevels()) {
                windows.push_back(Variant(VariantMap{{"identifier", Variant(toplevel.identifier)},
                                                     {"title", Variant(toplevel.title)},
                                                     {"app_id", Variant(toplevel.appId)}}));
            }
        }
        std::vector<Variant> monitors;
        for (const auto &output : outputs) {
            monitors.push_back(Variant(VariantMap{{"name", Variant(output.name)},
                                                  {"description", Variant(output.description)},
                                                  {"x", Variant(int32_t(output.logicalPosition.x()))},
                                                  {"y", Variant(int32_t(output.logicalPosition.y()))},
                                                  {"width", Variant(int32_t(output.logicalSize.width()))},
                                                  {"height", Variant(int32_t(output.logicalSize.height()))}}));
        }
        base::dbus::Message prompt = permissionsCall(bus, "Prompt");
        VariantMap options{{"types", Variant(session->types)}};
        if (session->types & sourceMonitor)
            options["monitors"] = base::dbus::Type<std::vector<Variant>>::toVariant(monitors);
        if (session->types & sourceWindow)
            options["windows"] = base::dbus::Type<std::vector<Variant>>::toVariant(windows);
        prompt.append(path, std::string("screen-recording"), session->appId, parentWindow, options);
        // The user answers in their own time.
        prompting = true;
        pendingPrompt = bus->callAsync(
                prompt,
                [this](const base::dbus::Reply &reply) {
                    prompting = false;
                    if (finished)
                        return;
                    uint32_t response = 0;
                    VariantMap results;
                    if (!reply.read(response, results)) {
                        LM_WARNING(base::LogCategory::defaultCategory())
                                << "Permission prompt failed:" << reply.error().message;
                        finish(2);
                        return;
                    }
                    answered(response, results);
                },
                std::chrono::milliseconds(INT_MAX));
    }

    void finish(uint32_t response, const VariantMap &results = {})
    {
        if (finished)
            return;
        finished = true;
        if (session->request == this)
            session->request = nullptr;
        object.reset();
        call.reply(response, results);
        if (prompting) {
            base::dbus::Message dismiss = permissionsCall(bus, "Dismiss");
            dismiss.append(path);
            bus->send(dismiss);
        }
        if (response != 0)
            pending.clear();
        session->deleteRequestLater(this);
    }

    void Close(MethodCall &close)
    {
        if (close.message().sender() != requester) {
            close.replyError("org.freedesktop.DBus.Error.AccessDenied",
                             "Only the requesting service may close this request");
            return;
        }
        finish(1);
    }

private:
    void answered(uint32_t response, const VariantMap &results)
    {
        if (response != 0) {
            finish(response == 1 ? 1 : 2);
            return;
        }
        const auto list = [&results](const std::string &key) {
            const auto it = results.find(key);
            return it == results.end() ? std::vector<std::string>() : toStringList(it->second);
        };
        const std::vector<std::string> monitors = list("monitors");
        const std::vector<std::string> windows = list("windows");
        std::vector<CaptureTarget> targets;
        for (const std::string &name : monitors)
            targets.push_back({CaptureTarget::Kind::Monitor, name});
        for (const std::string &identifier : windows)
            targets.push_back({CaptureTarget::Kind::Window, identifier});
        // A token can only name monitors, so a window choice gets none.
        startStreams(targets, windows.empty() ? session->persistMode : persistNone);
    }

    void startStreams(const std::vector<CaptureTarget> &targets, uint32_t persist)
    {
        const auto outputs = wayland->outputs();
        const auto toplevels = wayland->toplevels();
        // A window is painted at its output's rate; the fastest is close enough.
        int windowFramerate = 1;
        for (const auto &output : outputs)
            windowFramerate = std::max(windowFramerate, framerateFor(output));
        std::vector<CaptureTarget> selected;
        for (const auto &target : targets) {
            int framerate = windowFramerate;
            if (target.kind == CaptureTarget::Kind::Monitor) {
                const CaptureOutput *output = findOutput(outputs, target.name);
                if (!output)
                    continue;
                framerate = framerateFor(*output);
            } else if (std::none_of(toplevels.begin(), toplevels.end(), [&](const auto &toplevel) {
                           return toplevel.identifier == target.name;
                       })) {
                continue;
            }
            if (std::any_of(selected.begin(), selected.end(), [&](const CaptureTarget &other) {
                    return other.kind == target.kind && other.name == target.name;
                }))
                continue;
            selected.push_back(target);
            pending.push_back(std::make_unique<ScreenCastStream>(wayland, pipewire, target,
                                                                 session->cursorMode == cursorEmbedded, framerate));
            ScreenCastStream *stream = pending.back().get();
            stream->onFailed = [this] {
                finish(2);
            };
            stream->onReady = [this] {
                deliver();
            };
            stream->onStopped = [this] {
                finish(2);
            };
            if (!stream->start()) {
                finish(2);
                return;
            }
        }
        if (selected.empty()) {
            finish(2);
            return;
        }
        granted = persist;
    }

    void deliver()
    {
        if (finished)
            return;
        for (const auto &stream : pending) {
            if (!stream->nodeId())
                return;
        }
        const auto outputs = wayland->outputs();
        std::vector<StreamInfo> streams;
        std::vector<std::string> monitors;
        for (const auto &stream : pending) {
            const CaptureTarget &target = stream->target();
            VariantMap properties;
            if (target.kind == CaptureTarget::Kind::Monitor) {
                monitors.push_back(target.name);
                properties["source_type"] = Variant(sourceMonitor);
                if (const CaptureOutput *output = findOutput(outputs, target.name)) {
                    properties["position"] =
                            Variant(IntPair{output->logicalPosition.x(), output->logicalPosition.y()});
                    properties["size"] = Variant(IntPair{output->logicalSize.width(), output->logicalSize.height()});
                }
            } else {
                properties["source_type"] = Variant(sourceWindow);
                properties["size"] = Variant(IntPair{stream->size().width(), stream->size().height()});
            }
            streams.emplace_back(stream->nodeId(), properties);
        }
        VariantMap results{{"streams", Variant(streams)}};
        results["persist_mode"] = Variant(granted);
        if (granted != persistNone)
            results["restore_data"] = restoreData(monitors);
        // The session owns the streams from here; a stream ending closes it.
        for (const auto &stream : pending) {
            stream->onReady = nullptr;
            stream->onFailed = nullptr;
            stream->onStopped = [session = this->session] {
                session->close(true);
            };
        }
        session->streams = std::move(pending);
        session->started = true;
        pending.clear();
        finish(0, results);
    }

    base::dbus::Connection *bus;
    MethodCall call;
    std::string requester;
    std::string path;
    ScreenCastSession *session;
    WaylandCapture *wayland;
    PipeWireCore *pipewire;
    std::unique_ptr<base::dbus::Object> object;
    base::dbus::PendingCall pendingPrompt;
    std::vector<std::unique_ptr<ScreenCastStream>> pending;
    uint32_t granted = persistNone;
    bool prompting = false;
    bool finished = false;
};

void ScreenCastSession::close(bool notify)
{
    if (closed)
        return;
    closed = true;
    if (request)
        request->finish(1);
    streams.clear();
    if (notify)
        bus->emitSignal(path, sessionInterface, "Closed");
    object.reset();
    portal->deleteLater(this);
}

ScreenCastPortal::ScreenCastPortal(base::dbus::Connection *bus)
    : m_bus(bus)
    , m_object(bus, "/org/freedesktop/portal/desktop", "org.freedesktop.impl.portal.ScreenCast")
    , m_wayland(std::make_unique<WaylandCapture>())
    , m_pipewire(std::make_unique<PipeWireCore>())
{
    m_object.addProperty("version", [this] { return version(); });
    m_object.addProperty("AvailableSourceTypes", [this] { return availableSourceTypes(); });
    m_object.addProperty("AvailableCursorModes", [this] { return availableCursorModes(); });
    m_object.addMethod("CreateSession", "oosa{sv}", "ua{sv}", [this](MethodCall &call) { createSession(call); },
                       {{"handle", "sessionHandle", "appId", "options"}, {"response", "results"}});
    m_object.addMethod("SelectSources", "oosa{sv}", "ua{sv}", [this](MethodCall &call) { selectSources(call); },
                       {{"handle", "sessionHandle", "appId", "options"}, {"response", "results"}});
    m_object.addMethod("Start", "oossa{sv}", "ua{sv}", [this](MethodCall &call) { start(call); },
                       {{"handle", "sessionHandle", "appId", "parentWindow", "options"}, {"response", "results"}});
}

ScreenCastPortal::~ScreenCastPortal()
{
    // Streams go before the connections they use.
    m_sessions.clear();
}

bool ScreenCastPortal::publish()
{
    return m_object.publish();
}

ScreenCastSession *ScreenCastPortal::session(const std::string &path, const std::string &sender) const
{
    const auto it = m_sessions.find(path);
    return it != m_sessions.end() && it->second->owner == sender ? it->second.get() : nullptr;
}

void ScreenCastPortal::deleteLater(ScreenCastSession *session)
{
    later(m_alive, [this, session, path = session->path] {
        const auto it = m_sessions.find(path);
        if (it != m_sessions.end() && it->second.get() == session)
            m_sessions.erase(it);
    });
}

void ScreenCastPortal::createSession(MethodCall &call)
{
    ObjectPath handle;
    ObjectPath sessionHandle;
    std::string appId;
    VariantMap options;
    call.read(handle, sessionHandle, appId, options);

    const std::string &path = sessionHandle.path;
    if (!base::strings::startsWith(path, "/org/freedesktop/portal/desktop/session/") || m_sessions.count(path)) {
        call.reply(uint32_t(2), VariantMap());
        return;
    }
    auto session = std::make_unique<ScreenCastSession>(this, path, call.message().sender(), appId);
    if (!session->exportObject()) {
        call.reply(uint32_t(2), VariantMap());
        return;
    }
    m_sessions.emplace(path, std::move(session));
    call.reply(uint32_t(0), VariantMap{{"session_id", Variant(path.substr(path.rfind('/') + 1))}});
}

void ScreenCastPortal::selectSources(MethodCall &call)
{
    ObjectPath handle;
    ObjectPath sessionHandle;
    std::string appId;
    VariantMap options;
    call.read(handle, sessionHandle, appId, options);

    ScreenCastSession *session = this->session(sessionHandle.path, call.message().sender());
    if (!session || session->started || session->closed || session->request) {
        call.reply(uint32_t(2), VariantMap());
        return;
    }
    const uint32_t types = optionUInt(options, "types", sourceMonitor);
    const uint32_t cursorMode = optionUInt(options, "cursor_mode", cursorHidden);
    const uint32_t persistMode = optionUInt(options, "persist_mode", persistNone);
    if (!(types & availableSourceTypes()) || (cursorMode & ~availableCursorModes()) || cursorMode == 0
        || persistMode > persistUntilRevoked) {
        session->close(true);
        call.reply(uint32_t(2), VariantMap());
        return;
    }
    session->types = types & availableSourceTypes();
    session->cursorMode = cursorMode;
    session->persistMode = persistMode;
    const auto restore = options.find("restore_data");
    session->restore = restore == options.end() ? std::vector<std::string>() : restoredMonitors(restore->second);
    call.reply(uint32_t(0), VariantMap());
}

void ScreenCastPortal::start(MethodCall &call)
{
    ObjectPath handle;
    ObjectPath sessionHandle;
    std::string appId;
    std::string parentWindow;
    VariantMap options;
    call.read(handle, sessionHandle, appId, parentWindow, options);

    ScreenCastSession *session = this->session(sessionHandle.path, call.message().sender());
    // One Start at a time: a second would leave the first without its reply.
    if (!session || session->started || session->closed || session->request
        || !base::strings::startsWith(handle.path, "/org/freedesktop/portal/desktop/request/")) {
        call.reply(uint32_t(2), VariantMap());
        return;
    }
    auto owned = std::make_unique<StartRequest>(this, call, handle.path, session);
    if (!owned->exportObject()) {
        call.reply(uint32_t(2), VariantMap());
        return;
    }
    StartRequest *request = owned.get();
    session->requests.emplace(request, std::move(owned));
    request->run(parentWindow);
}
