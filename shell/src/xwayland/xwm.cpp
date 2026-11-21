// Copyright 2017 The ChromiumOS Authors
// Copyright 2021 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.sommelier file.
//
// Upstream: chromiumos/platform2/vm_tools/sommelier/{sommelier,sommelier-window}.cc
// Revision: aa8a57908bdcff297e229679dc3240b1b108f57e
//
// Adapted for Lingmo: X11 metadata, configure and focus messages map to the
// native window manager, including translated negative desktop coordinates.
#include "xwm.h"

#include "scene/surface_node.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/socket.h>
#include <bit>
#include <climits>
#include <csignal>
#include <unistd.h>
#include <xcb/composite.h>
#include <xcb/shape.h>
#include <xcb/sync.h>
#include <xcb/xcbext.h>
#include <xcb/xfixes.h>

#include "base/event_loop.h"
#include "base/logging.h"
#include "base/strings.h"
#include "compositor/surface.h"
#include "core/main_loop.h"
#include "dnd_bridge.h"
#include "fd_watch.h"
#include "input/seat.h"
#include "selection_bridge.h"
#include "surface_association.h"
#include "wm/toplevel_event_handler.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_state.h"

namespace Lingmo::XWayland {
namespace {

const base::LogCategory &logs()
{
    return base::LogCategory::defaultCategory();
}

// A property's value as bytes.
std::string_view propertyBytes(xcb_get_property_reply_t *reply)
{
    return std::string_view(static_cast<const char *>(xcb_get_property_value(reply)),
                            size_t(xcb_get_property_value_length(reply)));
}

int constrainedExtent(int value, int base, int increment, int minimum, int maximum)
{
    const int low = std::clamp(minimum, 1, 65535);
    const int high = maximum > 0 ? std::clamp(maximum, low, 65535) : 65535;
    value = std::clamp(value, low, high);
    int fitted = base + (value - base) / increment * increment;
    if (fitted < low)
        fitted += (low - fitted + increment - 1) / increment * increment;
    if (fitted > high)
        fitted -= (fitted - high + increment - 1) / increment * increment;
    return std::clamp(fitted, low, high);
}

const char *const atomNames[] = {"WL_SURFACE_SERIAL",
                                 "_LINGMO_FOCUS_TIMESTAMP",
                                 "WM_PROTOCOLS",
                                 "WM_DELETE_WINDOW",
                                 "WM_TAKE_FOCUS",
                                 "WM_STATE",
                                 "WM_CHANGE_STATE",
                                 "WM_S0",
                                 "_NET_WM_CM_S0",
                                 "MANAGER",
                                 "UTF8_STRING",
                                 "_NET_WM_NAME",
                                 "_NET_SUPPORTING_WM_CHECK",
                                 "_NET_SUPPORTED",
                                 "_NET_CLIENT_LIST",
                                 "_NET_CLIENT_LIST_STACKING",
                                 "_NET_ACTIVE_WINDOW",
                                 "_NET_CLOSE_WINDOW",
                                 "_NET_WM_STATE",
                                 "_NET_WM_STATE_FULLSCREEN",
                                 "_NET_WM_STATE_MAXIMIZED_HORZ",
                                 "_NET_WM_STATE_MAXIMIZED_VERT",
                                 "_NET_WM_STATE_HIDDEN",
                                 "_NET_WM_STATE_FOCUSED",
                                 "_NET_WM_STATE_DEMANDS_ATTENTION",
                                 "_NET_WM_STATE_MODAL",
                                 "WM_CLIENT_LEADER",
                                 "_NET_STARTUP_ID",
                                 "_NET_WM_WINDOW_TYPE",
                                 "_NET_WM_WINDOW_TYPE_NORMAL",
                                 "_NET_WM_WINDOW_TYPE_DIALOG",
                                 "_NET_WM_WINDOW_TYPE_MENU",
                                 "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU",
                                 "_NET_WM_WINDOW_TYPE_POPUP_MENU",
                                 "_NET_WM_WINDOW_TYPE_TOOLTIP",
                                 "_NET_WM_WINDOW_TYPE_COMBO",
                                 "_NET_WM_WINDOW_TYPE_DND",
                                 "_NET_WM_WINDOW_TYPE_NOTIFICATION",
                                 "_MOTIF_WM_HINTS",
                                 "_NET_FRAME_EXTENTS",
                                 "_NET_REQUEST_FRAME_EXTENTS",
                                 "_NET_WM_MOVERESIZE",
                                 "_NET_DESKTOP_GEOMETRY",
                                 "_NET_WORKAREA",
                                 "_NET_CURRENT_DESKTOP",
                                 "_NET_NUMBER_OF_DESKTOPS",
                                 "_NET_WM_DESKTOP",
                                 "_XSETTINGS_S0",
                                 "_XSETTINGS_SETTINGS",
                                 "_NET_WM_PING",
                                 "_NET_WM_STATE_ABOVE",
                                 "_NET_WM_STATE_SKIP_TASKBAR",
                                 "_NET_WM_STATE_SKIP_PAGER",
                                 "WM_CLIENT_MACHINE",
                                 "_NET_WM_SYNC_REQUEST",
                                 "_NET_WM_SYNC_REQUEST_COUNTER",
                                 "_NET_WM_WINDOW_OPACITY"};

// How long a resize waits for the client's sync counter before going ahead.
constexpr int kSyncTimeoutMs = 1000;

// The two X-Resource requests used, sent through libxcb's extension API rather
// than a build dependency on libxcb-res. Layouts follow resproto.
xcb_extension_t resExtension = {"X-Resource", 0};
constexpr uint8_t kResQueryVersion = 0;
constexpr uint8_t kResQueryClientIds = 4;
constexpr uint32_t kResLocalClientPid = 2;

unsigned sendResRequest(xcb_connection_t *connection, uint8_t opcode, void *request,
                        size_t size)
{
    // xcb_send_request writes into the two iovecs before the ones it is given.
    iovec parts[3] = {};
    parts[2].iov_base = request;
    parts[2].iov_len = size;
    const xcb_protocol_request_t protocol {1, &resExtension, opcode, 0};
    return xcb_send_request(connection, XCB_REQUEST_CHECKED, parts + 2, &protocol);
}

unsigned queryResVersion(xcb_connection_t *connection)
{
    struct {
        uint8_t majorOpcode, minorOpcode;
        uint16_t length;
        uint8_t clientMajor, clientMinor;
        uint8_t pad[2];
    } request {0, 0, 0, 1, 2, {}};
    return sendResRequest(connection, kResQueryVersion, &request, sizeof(request));
}

unsigned queryClientPid(xcb_connection_t *connection, uint32_t window)
{
    struct {
        uint8_t majorOpcode, minorOpcode;
        uint16_t length;
        uint32_t specCount, client, mask;
    } request {0, 0, 0, 1, window, kResLocalClientPid};
    return sendResRequest(connection, kResQueryClientIds, &request, sizeof(request));
}

// Whether WM_CLIENT_MACHINE names this machine. Unset counts as local: the
// socket is. Toolkits write gethostname().
bool runsOnThisMachine(const std::string &machine)
{
    char host[HOST_NAME_MAX + 1] = {};
    gethostname(host, sizeof(host) - 1);
    return machine.empty() || machine == host;
}

// Latin-1 text lower-cased as Unicode does it, then as UTF-8.
std::string lowerLatin1ToUtf8(std::string latin1)
{
    for (char &c : latin1) {
        const auto byte = uint8_t(c);
        if ((byte >= 'A' && byte <= 'Z') || (byte >= 0xc0 && byte <= 0xde && byte != 0xd7))
            c = char(byte + 0x20);
    }
    return base::strings::latin1ToUtf8(latin1);
}

// The pid in a QueryClientIds reply, or zero when it has none.
int64_t clientPidFromReply(const void *reply)
{
    const auto *bytes = static_cast<const uint8_t *>(reply);
    uint32_t words = 0, count = 0;
    memcpy(&words, bytes + 4, 4);
    memcpy(&count, bytes + 8, 4);
    const size_t end = 32 + size_t(words) * 4;
    size_t offset = 32;
    for (uint32_t i = 0; i < count && offset + 12 <= end; ++i) {
        uint32_t mask = 0, length = 0;
        memcpy(&mask, bytes + offset + 4, 4);
        memcpy(&length, bytes + offset + 8, 4);
        offset += 12;
        if (length > end - offset)
            break;
        if (mask == kResLocalClientPid && length == 4) {
            uint32_t pid = 0;
            memcpy(&pid, bytes + offset, 4);
            return pid;
        }
        offset += length;
    }
    return 0;
}

} // namespace

struct Xwm::X11Window : compositor::SurfaceObserver, wm::WindowObserver {
    Xwm *xwm;
    uint32_t id;
    uint64_t generation;
    compositor::Surface *surface = nullptr;
    wm::Window *window = nullptr;
    // Logical geometry, and the X11 geometry it came from.
    base::Rect geometry, xGeometry;
    // Adopted from a ReparentNotify, which has no size; xGeometry holds a
    // placeholder one until the geometry is read or configured.
    bool sizeUnknown = false;
    // In X11 pixels, relative to the window.
    std::optional<base::Region> boundingShape, inputShape;
    uint64_t shapeGeneration = 0;
    // UTF-8; WM_CLASS's parts stay Latin-1, as the property holds them.
    std::string title, legacyTitle, instance, clazz, leaderClass;
    bool hasTitle = false, popup = false, roleLocked = false;
    uint64_t mapOrder = 0;
    uint32_t transient = 0;
    // The process on the X server's end of the client's socket; zero for the
    // server itself or this compositor. See ownerPid().
    uint32_t pid = 0;
    bool dialog = false, urgent = false, demandsAttention = false, initiallyMinimized = false;
    bool dragIcon = false;
    uint32_t leader = 0;
    std::string startupId;
    // No maximum: an empty size.
    base::Size minimum {1, 1}, maximum, increment {1, 1}, base {0, 0};
    bool mapped = false, overrideRedirect = false, decorated = true;
    bool input = true, takeFocus = false, deleteWindow = false, pingable = false;
    // Requested through _NET_WM_STATE; there is no pager, but the state is kept.
    bool above = false, skipTaskbar = false, skipPager = false;
    // _NET_WM_WINDOW_OPACITY, which KWin, Mutter and picom honour; some
    // clients set zero to stay mapped but unseen.
    float opacity = 1;
    uint32_t pingSerial = 0;
    std::string clientMachine;
    // _NET_WM_SYNC_REQUEST: the client's counter, our alarm on it, the last
    // value requested, and a size change held back until it is reached.
    bool syncable = false, syncPending = false, syncDeferred = false;
    xcb_sync_counter_t syncCounter = 0;
    xcb_sync_alarm_t syncAlarm = 0;
    int64_t syncValue = 0;
    bool metadataReady = false, syncing = false;
    bool propertiesPending = false;
    // Changed while a read was on its way, to be read again once it lands.
    bool allPropertiesDirty = false;
    std::vector<xcb_atom_t> dirtyProperties;
    // A read since the last full apply changed more than the window's labels.
    bool applyPending = false;
    // What synchronize() last wrote, so that only a change is written again.
    std::optional<std::vector<uint32_t>> writtenExtents, writtenState, writtenWmState;
    bool fullscreen = false, maximized = false, modal = false;
    // _NET_WM_STATE client messages received while mapped but not yet managed,
    // replayed over every read of the property until the window exists.
    std::vector<std::array<uint32_t, 3>> stateRequests;

    X11Window(Xwm *owner, uint32_t xid, uint64_t epoch)
        : xwm(owner)
        , id(xid)
        , generation(epoch)
    {
    }

    ~X11Window() override
    {
        xwm->detach(*this);
        if (syncAlarm)
            xcb_sync_destroy_alarm(xwm->m_connection, syncAlarm);
    }

    void onSurfaceDestroying(compositor::Surface *) override { xwm->detach(*this); }

    void onSurfaceCommit(compositor::Surface *) override { xwm->present(*this); }

    void onWindowDestroying(wm::Window *) override { window = nullptr; }

    void onWindowBoundsChanged(wm::Window *, const base::Rect &) override { xwm->synchronize(*this); }

    void onWindowStateTypeChanged(wm::Window *, wm::WindowStateType) override
    {
        xwm->synchronize(*this);
    }

    void onWindowFrameChanged(wm::Window *) override { xwm->synchronize(*this); }

    void onWindowActivationChanged(wm::Window *, bool active) override
    {
        xwm->focus(*this, active);
    }

    void onWindowVisibilityChanged(wm::Window *, bool) override { xwm->synchronize(*this); }
};

std::string Xwm::displayTitle(const X11Window &w)
{
    return w.hasTitle ? w.title : w.legacyTitle;
}

std::string Xwm::applicationId(const X11Window &w)
{
    return lowerLatin1ToUtf8(w.clazz.empty() ? w.leaderClass : w.clazz);
}

Xwm::Xwm(wm::WindowManager *manager, SurfaceAssociation *association)
    : m_manager(manager)
    , m_association(association)
{
    association->associated = [this](uint32_t id, compositor::Surface *surface) {
        auto it = m_windows.find(id);
        if (it == m_windows.end())
            return;
        auto &w = *it->second;
        detach(w);
        w.surface = surface;
        surface->addSurfaceObserver(&w);
        present(w);
    };
    m_activeWindowChanged = manager->activeWindowChanged.connect([this](wm::Window *) {
        if (m_ready)
            roots();
    });
}

Xwm::~Xwm()
{
    m_association->associated = {};
    if (m_drainHook)
        core::mainLoop()->removePrepareHook(m_drainHook);
    if (m_notifier)
        m_notifier->setEnabled(false);
    if (m_shutdownFd >= 0)
        shutdown(m_shutdownFd, SHUT_RDWR);
    if (m_setupThread.joinable())
        m_setupThread.join();
    auto windows = std::move(m_windows);
    windows.clear();
    m_dnd.reset();
    m_primarySelection.reset();
    m_selection.reset();
    if (m_connection)
        xcb_disconnect(m_connection);
    if (m_shutdownFd >= 0)
        close(m_shutdownFd);
}

void Xwm::start(int fd)
{
    m_fd = fd;
    m_shutdownFd = fcntl(fd, F_DUPFD_CLOEXEC, 3);
    if (m_shutdownFd < 0) {
        close(fd);
        if (initialized)
            initialized(false);
        return;
    }
    // XCB's setup handshake can wait for Wayland. Only this worker blocks;
    // ownership transfers to the compositor thread after setup is complete.
    m_setupThread = std::thread([this, fd, loop = core::mainLoop()->handle(),
                                 weak = m_weakFactory.getWeakPtr()] {
        m_connection = xcb_connect_to_fd(fd, nullptr);
        const auto *setup = xcb_get_setup(m_connection);
        bool ok = !xcb_connection_has_error(m_connection) && setup;
        if (ok) {
            auto roots = xcb_setup_roots_iterator(setup);
            ok = roots.rem > 0;
            if (ok)
                m_root = roots.data->root;
            const auto *composite = xcb_get_extension_data(m_connection, &xcb_composite_id);
            ok = ok && composite && composite->present;
            const auto *shape = xcb_get_extension_data(m_connection, &xcb_shape_id);
            if (shape && shape->present)
                m_shapeEvent = shape->first_event;
            const auto *sync = xcb_get_extension_data(m_connection, &xcb_sync_id);
            if (sync && sync->present) {
                free(xcb_sync_initialize_reply(
                        m_connection, xcb_sync_initialize(m_connection, 3, 1), nullptr));
                m_syncEvent = sync->first_event;
            }
            // QueryClientIds, which reports a client's pid, needs X-Resource 1.2.
            const auto *res = xcb_get_extension_data(m_connection, &resExtension);
            if (res && res->present) {
                xcb_generic_error_t *error = nullptr;
                auto *version = static_cast<uint8_t *>(xcb_wait_for_reply(
                        m_connection, queryResVersion(m_connection), &error));
                uint16_t major = 0, minor = 0;
                if (version) {
                    memcpy(&major, version + 8, 2);
                    memcpy(&minor, version + 10, 2);
                }
                m_clientPids = major > 1 || (major == 1 && minor >= 2);
                free(version);
                free(error);
            }
            const auto *xfixes = xcb_get_extension_data(m_connection, &xcb_xfixes_id);
            ok = ok && xfixes && xfixes->present;
            if (ok) {
                m_xfixesEvent = xfixes->first_event;
                auto *version = xcb_xfixes_query_version_reply(
                        m_connection, xcb_xfixes_query_version(m_connection, 5, 0), nullptr);
                ok = version && version->major_version >= 2;
                free(version);
            }
        }
        // The weak pointer is only read back on the loop's thread.
        loop.post([weak, ok] {
            Xwm *self = weak.get();
            if (!self)
                return;
            self->m_setupThread.join();
            if (!ok) {
                if (self->initialized)
                    self->initialized(false);
                return;
            }
            self->setup();
        });
    });
}

xcb_atom_t Xwm::atom(const char *name) const
{
    auto it = m_atoms.find(name);
    return it == m_atoms.end() ? XCB_ATOM_NONE : it->second;
}

// Sommelier's direct scale does the same between host and guest; see
// sommelier-transform.cc.
base::Point Xwm::fromX(const base::Point &point) const
{
    return base::Point(base::roundToInt(point.x() / m_scale), base::roundToInt(point.y() / m_scale))
            + m_manager->outputBounds().topLeft();
}

base::Point Xwm::toX(const base::Point &point) const
{
    const base::Point local = point - m_manager->outputBounds().topLeft();
    return base::Point(base::roundToInt(local.x() * m_scale), base::roundToInt(local.y() * m_scale));
}

base::Size Xwm::sizeFromX(const base::Size &size) const
{
    return base::Size(std::max(1, base::roundToInt(size.width() / m_scale)),
                      std::max(1, base::roundToInt(size.height() / m_scale)));
}

base::Size Xwm::sizeToX(const base::Size &size) const
{
    return base::Size(std::max(1, base::roundToInt(size.width() * m_scale)),
                      std::max(1, base::roundToInt(size.height() * m_scale)));
}

base::Region Xwm::regionFromX(const base::Region &region) const
{
    if (m_scale == 1)
        return region;
    base::Region result;
    for (const base::Rect &rect : region) {
        const base::Point topLeft(int(std::floor(rect.x() / m_scale)),
                                  int(std::floor(rect.y() / m_scale)));
        const base::Point bottomRight(int(std::ceil((rect.x() + rect.width()) / m_scale)),
                                      int(std::ceil((rect.y() + rect.height()) / m_scale)));
        result += base::Rect::fromEdges(topLeft.x(), topLeft.y(), bottomRight.x(), bottomRight.y());
    }
    return result;
}

// X11 clients scale themselves: Qt and Xft read Xft.dpi, GTK reads XSETTINGS.
// The values follow gsd-xsettings: GTK takes an integer window scale and keeps
// any fraction as a larger font DPI.
void Xwm::publishDpi()
{
    const int dpi = base::roundToInt(96 * m_scale);
    reply(xcb_get_property(m_connection, 0, m_root, XCB_ATOM_RESOURCE_MANAGER, XCB_ATOM_STRING, 0,
                           65536)
                  .sequence,
          [this, dpi](void *p) {
              auto *r = static_cast<xcb_get_property_reply_t *>(p);
              std::vector<std::string> lines;
              if (r && r->type == XCB_ATOM_STRING && r->format == 8) {
                  const std::string_view resources(
                          static_cast<const char *>(xcb_get_property_value(r)),
                          size_t(xcb_get_property_value_length(r)));
                  for (const std::string_view line : base::strings::splitViews(resources, '\n')) {
                      if (!line.empty() && !line.starts_with("Xft.dpi:"))
                          lines.emplace_back(line);
                  }
              }
              lines.push_back("Xft.dpi:\t" + std::to_string(dpi));
              const std::string value = base::strings::join(lines, "\n") + '\n';
              xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, m_root,
                                  XCB_ATOM_RESOURCE_MANAGER, XCB_ATOM_STRING, 8, value.size(),
                                  value.data());
              xcb_flush(m_connection);
          });
    if (!m_xsettings)
        return;
    const int windowScale = std::max(1, int(std::floor(m_scale)));
    const std::pair<const char *, int32_t> settings[] = {
            {"Gdk/UnscaledDPI", dpi * 1024 / windowScale},
            {"Gdk/WindowScalingFactor", windowScale},
            {"Xft/DPI", dpi * 1024}};
    std::string data;
    auto append = [&data](auto value) {
        data.append(reinterpret_cast<const char *>(&value), sizeof(value));
    };
    append(uint8_t(std::endian::native == std::endian::little ? 0 : 1));
    data.append(3, '\0');
    append(uint32_t(++m_xsettingsSerial));
    append(uint32_t(std::size(settings)));
    for (const auto &[name, value] : settings) {
        const std::string_view key(name);
        append(uint8_t(0));
        data.append(1, '\0');
        append(uint16_t(key.size()));
        data.append(key);
        data.append((4 - key.size() % 4) % 4, '\0');
        append(uint32_t(m_xsettingsSerial));
        append(value);
    }
    xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, m_support,
                        atom("_XSETTINGS_SETTINGS"), atom("_XSETTINGS_SETTINGS"), 8, data.size(),
                        data.data());
    xcb_flush(m_connection);
}

// The EWMH counterpart of xdg_wm_base.ping. The client echoes the message to
// the root window, with the timestamp it was sent.
void Xwm::ping(uint32_t window)
{
    auto it = m_windows.find(window);
    if (it == m_windows.end() || !it->second->pingable || it->second->pingSerial)
        return;
    auto &w = *it->second;
    // Not an X server time, only matched against the echo; 0 means none pending.
    if (++m_pingSerial == 0)
        ++m_pingSerial;
    w.pingSerial = m_pingSerial;
    xcb_client_message_event_t e {};
    e.response_type = XCB_CLIENT_MESSAGE;
    e.format = 32;
    e.window = window;
    e.type = atom("WM_PROTOCOLS");
    e.data.data32[0] = atom("_NET_WM_PING");
    e.data.data32[1] = w.pingSerial;
    e.data.data32[2] = window;
    xcb_send_event(m_connection, 0, window, XCB_EVENT_MASK_NO_EVENT,
                   reinterpret_cast<const char *>(&e));
    xcb_flush(m_connection);
    after(std::chrono::milliseconds(m_pingTimeoutMs),
          [this, window, generation = w.generation, serial = w.pingSerial] {
              auto it = m_windows.find(window);
              if (it == m_windows.end() || it->second->generation != generation
                  || it->second->pingSerial != serial || !it->second->window)
                  return;
              it->second->window->setResponsive(false);
          });
}

// A hung client cannot act on WM_DELETE_WINDOW. As in KWin, a local client's
// process is killed, and its connection closed whatever it runs on.
void Xwm::kill(uint32_t window)
{
    auto it = m_windows.find(window);
    if (it == m_windows.end())
        return;
    const auto &w = *it->second;
    if (const uint32_t pid = ownerPid(w)) {
        LM_INFO(logs()) << "Killing unresponsive X11 client" << pid;
        ::kill(pid_t(pid), SIGKILL);
    }
    xcb_kill_client(m_connection, window);
    xcb_flush(m_connection);
}

// A client forwarded from another machine, as by ssh -X, reaches the server
// through a local proxy; the socket's pid is the proxy's, never the client's.
uint32_t Xwm::ownerPid(const X11Window &w)
{
    return runsOnThisMachine(w.clientMachine) ? w.pid : 0;
}

// EWMH _NET_WM_SYNC_REQUEST: the client sets its counter to the requested
// value once it has handled the configure that follows and painted at the new
// size. The alarm on the counter reports that.
bool Xwm::requestSync(X11Window &w)
{
    if (!m_syncEvent || !w.syncable || !w.syncCounter)
        return false;
    ++w.syncValue;
    const xcb_sync_int64_t value {int32_t(w.syncValue >> 32), uint32_t(w.syncValue)};
    if (!w.syncAlarm) {
        w.syncAlarm = xcb_generate_id(m_connection);
        xcb_sync_create_alarm_value_list_t alarm {};
        alarm.counter = w.syncCounter;
        alarm.valueType = XCB_SYNC_VALUETYPE_ABSOLUTE;
        alarm.value = value;
        alarm.testType = XCB_SYNC_TESTTYPE_POSITIVE_COMPARISON;
        alarm.delta = {0, 1};
        alarm.events = 1;
        xcb_sync_create_alarm_aux(m_connection, w.syncAlarm,
                                  XCB_SYNC_CA_COUNTER | XCB_SYNC_CA_VALUE_TYPE | XCB_SYNC_CA_VALUE
                                          | XCB_SYNC_CA_TEST_TYPE | XCB_SYNC_CA_DELTA
                                          | XCB_SYNC_CA_EVENTS,
                                  &alarm);
    } else {
        xcb_sync_change_alarm_value_list_t alarm {};
        alarm.value = value;
        xcb_sync_change_alarm_aux(m_connection, w.syncAlarm, XCB_SYNC_CA_VALUE, &alarm);
    }
    xcb_client_message_event_t e {};
    e.response_type = XCB_CLIENT_MESSAGE;
    e.format = 32;
    e.window = w.id;
    e.type = atom("WM_PROTOCOLS");
    e.data.data32[0] = atom("_NET_WM_SYNC_REQUEST");
    e.data.data32[1] = XCB_CURRENT_TIME;
    e.data.data32[2] = value.lo;
    e.data.data32[3] = uint32_t(value.hi);
    xcb_send_event(m_connection, 0, w.id, XCB_EVENT_MASK_NO_EVENT,
                   reinterpret_cast<const char *>(&e));
    w.syncPending = true;
    w.syncDeferred = false;
    after(std::chrono::milliseconds(kSyncTimeoutMs),
          [this, id = w.id, generation = w.generation, requested = w.syncValue] {
              auto it = m_windows.find(id);
              if (it == m_windows.end() || it->second->generation != generation
                  || !it->second->syncPending || it->second->syncValue != requested)
                  return;
              auto &w = *it->second;
              w.syncPending = false;
              if (std::exchange(w.syncDeferred, false))
                  synchronize(w);
          });
    return true;
}

void Xwm::applyStateRequest(X11Window &w, const std::array<uint32_t, 3> &request)
{
    const auto apply = [&request](bool &state) {
        state = request[0] == 1 || (request[0] == 2 && !state);
    };
    bool maximize = false;
    for (int i = 1; i <= 2; ++i) {
        if (i == 2 && request[1] == request[2])
            continue;
        const xcb_atom_t state = request[i];
        if (state == atom("_NET_WM_STATE_FULLSCREEN"))
            apply(w.fullscreen);
        else if (state == atom("_NET_WM_STATE_MAXIMIZED_HORZ")
                 || state == atom("_NET_WM_STATE_MAXIMIZED_VERT"))
            maximize = true;
        else if (state == atom("_NET_WM_STATE_ABOVE"))
            apply(w.above);
        else if (state == atom("_NET_WM_STATE_SKIP_TASKBAR"))
            apply(w.skipTaskbar);
        else if (state == atom("_NET_WM_STATE_SKIP_PAGER"))
            apply(w.skipPager);
        else if (state == atom("_NET_WM_STATE_MODAL"))
            apply(w.modal);
        else if (state == atom("_NET_WM_STATE_DEMANDS_ATTENTION"))
            apply(w.demandsAttention);
    }
    // As for a managed window, either half maximizes or restores both.
    if (maximize)
        apply(w.maximized);
    updateAttention(w);
}

void Xwm::applyHints(X11Window &w)
{
    if (!w.surface)
        return;
    w.surface->setAlwaysOnTop(w.above);
    w.surface->setSkipTaskbar(w.skipTaskbar);
}

void Xwm::updateAttention(X11Window &w)
{
    // WM_HINTS' urgency bit and _NET_WM_STATE_DEMANDS_ATTENTION are one thing
    // to the shell: the application asks for the user's attention.
    if (w.window)
        w.window->setDemandsAttention(w.urgent || w.demandsAttention);
}

void Xwm::setScale(double scale)
{
    if (scale <= 0 || scale == m_scale)
        return;
    m_scale = scale;
    if (!m_ready)
        return;
    roots();
    publishDpi();
    for (auto &[id, w] : m_windows) {
        if (w->window) {
            synchronize(*w);
        } else if (!w->xGeometry.isEmpty()) {
            w->geometry = base::Rect(fromX(w->xGeometry.topLeft()), sizeFromX(w->xGeometry.size()));
            present(*w);
        }
    }
    xcb_flush(m_connection);
}

void Xwm::reply(unsigned sequence, std::function<void(void *)> callback)
{
    m_replies.push_back({sequence, std::move(callback)});
}

void Xwm::after(std::chrono::milliseconds delay, std::function<void()> task)
{
    auto timer = std::make_unique<base::Timer>(core::mainLoop());
    base::Timer *raw = timer.get();
    const auto position = m_delayed.insert(m_delayed.end(), std::move(timer));
    raw->setSingleShot(true);
    raw->onTimeout = [this, position, task = std::move(task)] {
        // The timer goes first; the task may schedule another.
        const std::function<void()> run = task;
        m_delayed.erase(position);
        run();
    };
    raw->start(delay);
}

void Xwm::setup()
{
    m_notifier = std::make_unique<FdWatch>(m_fd, EPOLLIN, [this] { drain(); });
    m_notifier->setEnabled(true);
    // Other work queues X requests too, and a flush outside drain(), here or
    // in a bridge, can leave events in XCB's queue; the loop runs this before
    // every wait, whether or not it then blocks.
    m_drainHook = core::mainLoop()->addPrepareHook([this] { drain(); });
    for (auto *name : atomNames) {
        reply(xcb_intern_atom(m_connection, 0, strlen(name), name).sequence, [this, name](void *p) {
            if (p)
                m_atoms[name] = static_cast<xcb_intern_atom_reply_t *>(p)->atom;
        });
    }
    reply(xcb_get_input_focus(m_connection).sequence, [this](void *p) {
        if (!p || m_atoms.size() != std::size(atomNames)) {
            if (initialized)
                initialized(false);
            return;
        }
        uint32_t mask = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
                | XCB_EVENT_MASK_STRUCTURE_NOTIFY;
        m_rootSubscription = xcb_change_window_attributes_checked(m_connection, m_root,
                                                                  XCB_CW_EVENT_MASK, &mask);
        m_redirection = xcb_composite_redirect_subwindows_checked(m_connection, m_root,
                                                                  XCB_COMPOSITE_REDIRECT_MANUAL);
        m_support = xcb_generate_id(m_connection);
        uint32_t supportMask = XCB_EVENT_MASK_PROPERTY_CHANGE;
        xcb_create_window(m_connection, 0, m_support, m_root, 0, 0, 1, 1, 0,
                          XCB_WINDOW_CLASS_INPUT_ONLY, XCB_COPY_FROM_PARENT, XCB_CW_EVENT_MASK,
                          &supportMask);
        property(m_root, "_NET_SUPPORTING_WM_CHECK", XCB_ATOM_WINDOW, {m_support});
        property(m_support, "_NET_SUPPORTING_WM_CHECK", XCB_ATOM_WINDOW, {m_support});
        const char name[] = "Lingmo";
        xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, m_support, atom("_NET_WM_NAME"),
                            atom("UTF8_STRING"), 8, sizeof(name) - 1, name);
        for (const char *selection : {"WM_S0", "_NET_WM_CM_S0"}) {
            xcb_set_selection_owner(m_connection, m_support, atom(selection), XCB_CURRENT_TIME);
            xcb_client_message_event_t e {};
            e.response_type = XCB_CLIENT_MESSAGE;
            e.format = 32;
            e.window = m_root;
            e.type = atom("MANAGER");
            e.data.data32[1] = atom(selection);
            e.data.data32[2] = m_support;
            xcb_send_event(m_connection, 0, m_root, XCB_EVENT_MASK_STRUCTURE_NOTIFY,
                           reinterpret_cast<const char *>(&e));
        }
        std::vector<uint32_t> supported;
        for (auto *n : {"_NET_SUPPORTING_WM_CHECK",
                        "_NET_WM_NAME",
                        "_NET_CLIENT_LIST",
                        "_NET_CLIENT_LIST_STACKING",
                        "_NET_ACTIVE_WINDOW",
                        "_NET_CLOSE_WINDOW",
                        "_NET_WM_STATE",
                        "_NET_WM_STATE_FULLSCREEN",
                        "_NET_WM_STATE_MAXIMIZED_HORZ",
                        "_NET_WM_STATE_MAXIMIZED_VERT",
                        "_NET_WM_STATE_HIDDEN",
                        "_NET_WM_STATE_FOCUSED",
                        "_NET_WM_STATE_DEMANDS_ATTENTION",
                        "_NET_WM_STATE_MODAL",
                        "_NET_WM_STATE_ABOVE",
                        "_NET_WM_STATE_SKIP_TASKBAR",
                        "_NET_WM_STATE_SKIP_PAGER",
                        "_NET_WM_PING",
                        "_NET_FRAME_EXTENTS",
                        "_NET_REQUEST_FRAME_EXTENTS",
                        "_NET_WM_MOVERESIZE",
                        "_NET_DESKTOP_GEOMETRY",
                        "_NET_WORKAREA",
                        "_NET_NUMBER_OF_DESKTOPS",
                        "_NET_CURRENT_DESKTOP",
                        "_NET_WM_DESKTOP"})
            supported.push_back(atom(n));
        property(m_root, "_NET_SUPPORTED", XCB_ATOM_ATOM, supported);
        reply(xcb_get_selection_owner(m_connection, atom("WM_S0")).sequence, [this](void *p) {
            m_initializationFailed |=
                    !p || static_cast<xcb_get_selection_owner_reply_t *>(p)->owner != m_support;
        });
        reply(xcb_get_selection_owner(m_connection, atom("_NET_WM_CM_S0")).sequence,
              [this](void *p) {
                  // A later reply has arrived, so these checked requests cannot block.
                  auto *subscriptionError = xcb_request_check(m_connection, m_rootSubscription);
                  auto *redirectionError = xcb_request_check(m_connection, m_redirection);
                  m_initializationFailed |= subscriptionError || redirectionError;
                  free(subscriptionError);
                  free(redirectionError);
                  m_ready = !m_initializationFailed && p
                          && static_cast<xcb_get_selection_owner_reply_t *>(p)->owner == m_support;
                  if (m_ready && m_manager->seat()) {
                      m_selection = std::make_unique<SelectionBridge>(
                              m_connection, m_root, m_manager->seat()->selection(),
                              [this](unsigned sequence, std::function<void(void *)> callback) {
                                  reply(sequence, std::move(callback));
                              });
                      m_selection->start(m_xfixesEvent);
                      m_primarySelection = std::make_unique<SelectionBridge>(
                              m_connection, m_root, m_manager->seat()->primarySelection(),
                              [this](unsigned sequence, std::function<void(void *)> callback) {
                                  reply(sequence, std::move(callback));
                              },
                              SelectionBridge::Kind::Primary);
                      m_primarySelection->start(m_xfixesEvent);
                      m_dnd = std::make_unique<DndBridge>(
                              m_connection, m_root, m_manager,
                              [this](unsigned sequence, std::function<void(void *)> callback) {
                                  reply(sequence, std::move(callback));
                              });
                      m_dnd->windowForSurface = [this](compositor::Surface *surface) {
                          for (auto &[id, w] : m_windows) {
                              if (surface && w->surface == surface)
                                  return xcb_window_t(id);
                          }
                          return xcb_window_t(XCB_NONE);
                      };
                      m_dnd->toRoot = [this](const base::Point &point) {
                          return toX(point);
                      };
                      m_dnd->fromRoot = [this](const base::Point &point) {
                          return fromX(point);
                      };
                      m_dnd->rootSize = [this] {
                          return sizeToX(m_manager->outputBounds().size());
                      };
                      m_dnd->start(m_xfixesEvent);
                  }
                  roots();
                  if (m_ready) {
                      // GTK only learns a scale through XSETTINGS. Nothing in
                      // the session runs a manager, but one started later wins.
                      reply(xcb_get_selection_owner(m_connection, atom("_XSETTINGS_S0")).sequence,
                            [this](void *p) {
                                auto *r = static_cast<xcb_get_selection_owner_reply_t *>(p);
                                if (!r || r->owner)
                                    return;
                                xcb_set_selection_owner(m_connection, m_support,
                                                        atom("_XSETTINGS_S0"), XCB_CURRENT_TIME);
                                xcb_client_message_event_t e {};
                                e.response_type = XCB_CLIENT_MESSAGE;
                                e.format = 32;
                                e.window = m_root;
                                e.type = atom("MANAGER");
                                e.data.data32[1] = atom("_XSETTINGS_S0");
                                e.data.data32[2] = m_support;
                                xcb_send_event(m_connection, 0, m_root,
                                               XCB_EVENT_MASK_STRUCTURE_NOTIFY,
                                               reinterpret_cast<const char *>(&e));
                                m_xsettings = true;
                                publishDpi();
                            });
                      publishDpi();
                  }
                  if (initialized)
                      initialized(m_ready);
              });
    });
    xcb_flush(m_connection);
}

void Xwm::drain()
{
    if (m_draining || !m_connection)
        return;
    m_draining = true;
    int budget = 256;
    while (budget-- > 0) {
        auto *e = xcb_poll_for_event(m_connection);
        if (!e)
            break;
        event(e);
        free(e);
    }
    int replyBudget = 256;
    while (!m_replies.empty() && replyBudget-- > 0) {
        void *value = nullptr;
        xcb_generic_error_t *error = nullptr;
        if (!xcb_poll_for_reply(m_connection, m_replies.front().sequence, &value, &error)) {
            break;
        }
        auto callback = std::move(m_replies.front().callback);
        m_replies.pop_front();
        callback(value);
        free(value);
        free(error);
    }
    if (m_ready
        && (m_lastOutput != m_manager->outputBounds()
            || m_lastWorkArea != m_manager->workAreaFor(nullptr)))
        roots();
    // Reply polling, and a flush too, since it reads while it writes, pull
    // events off the fd into XCB's own queue, where the notifier never sees them.
    int queuedBudget = 256;
    while (queuedBudget-- > 0) {
        xcb_flush(m_connection);
        auto *e = xcb_poll_for_queued_event(m_connection);
        if (!e)
            break;
        event(e);
        free(e);
    }
    m_draining = false;
    if (xcb_connection_has_error(m_connection)) {
        m_notifier->setEnabled(false);
        if (failed)
            failed();
    } else if (budget < 0 || replyBudget < 0 || queuedBudget < 0) {
        after(std::chrono::milliseconds(0), [this] { drain(); });
    }
}

void Xwm::property(uint32_t window, const char *name, xcb_atom_t type,
                   const std::vector<uint32_t> &values)
{
    xcb_change_property(m_connection, XCB_PROP_MODE_REPLACE, window, atom(name), type, 32,
                        values.size(), values.data());
}

void Xwm::property(uint32_t window, const char *name, xcb_atom_t type,
                   std::vector<uint32_t> values, std::optional<std::vector<uint32_t>> &written)
{
    if (written == values)
        return;
    property(window, name, type, values);
    written = std::move(values);
}

void Xwm::message(uint32_t window, xcb_atom_t protocol, xcb_timestamp_t time)
{
    xcb_client_message_event_t e {};
    e.response_type = XCB_CLIENT_MESSAGE;
    e.format = 32;
    e.window = window;
    e.type = atom("WM_PROTOCOLS");
    e.data.data32[0] = protocol;
    e.data.data32[1] = time;
    xcb_send_event(m_connection, 0, window, XCB_EVENT_MASK_NO_EVENT, reinterpret_cast<char *>(&e));
    xcb_flush(m_connection);
}

// Adapted from Sommelier sommelier-xshape.cc; keep regions in the native scene.
void Xwm::shape(X11Window &w)
{
    if (!m_shapeEvent)
        return;
    const auto id = w.id;
    const auto generation = w.generation;
    const auto revision = ++w.shapeGeneration;
    reply(xcb_shape_query_extents(m_connection, id).sequence,
          [this, id, generation, revision](void *p) {
              auto it = m_windows.find(id);
              if (!p || it == m_windows.end() || it->second->generation != generation
                  || it->second->shapeGeneration != revision)
                  return;
              auto &w = *it->second;
              auto *r = static_cast<xcb_shape_query_extents_reply_t *>(p);
              if (!r->bounding_shaped)
                  w.boundingShape.reset();
              for (auto kind : {XCB_SHAPE_SK_BOUNDING, XCB_SHAPE_SK_INPUT}) {
                  if (kind == XCB_SHAPE_SK_BOUNDING && !r->bounding_shaped)
                      continue;
                  reply(xcb_shape_get_rectangles(m_connection, id, kind).sequence,
                        [this, id, generation, revision, kind](void *p) {
                            auto it = m_windows.find(id);
                            if (!p || it == m_windows.end() || it->second->generation != generation
                                || it->second->shapeGeneration != revision)
                                return;
                            auto *r = static_cast<xcb_shape_get_rectangles_reply_t *>(p);
                            auto *rects = xcb_shape_get_rectangles_rectangles(r);
                            base::Region region;
                            for (int i = 0; i < xcb_shape_get_rectangles_rectangles_length(r); ++i)
                                region += base::Rect(rects[i].x, rects[i].y, rects[i].width,
                                                rects[i].height);
                            auto &w = *it->second;
                            if (kind == XCB_SHAPE_SK_BOUNDING)
                                w.boundingShape = region;
                            else
                                w.inputShape = region;
                            present(w);
                        });
              }
              present(w);
          });
}

std::vector<xcb_atom_t> Xwm::followedProperties()
{
    return {XCB_ATOM_WM_NAME,          atom("_NET_WM_NAME"),        XCB_ATOM_WM_CLASS,
            XCB_ATOM_WM_TRANSIENT_FOR, XCB_ATOM_WM_HINTS,           XCB_ATOM_WM_NORMAL_HINTS,
            atom("WM_PROTOCOLS"),      atom("_MOTIF_WM_HINTS"),     atom("_NET_WM_STATE"),
            atom("_NET_WM_WINDOW_TYPE"), atom("WM_CLIENT_LEADER"), atom("_NET_STARTUP_ID"),
            atom("WM_CLIENT_MACHINE"),   atom("_NET_WM_SYNC_REQUEST_COUNTER"),
            atom("_NET_WM_WINDOW_OPACITY")};
}

void Xwm::properties(X11Window &w, std::vector<xcb_atom_t> atoms)
{
    if (w.propertiesPending) {
        if (atoms.empty())
            w.allPropertiesDirty = true;
        else
            w.dirtyProperties.insert(w.dirtyProperties.end(), atoms.begin(), atoms.end());
        return;
    }
    w.propertiesPending = true;
    const uint32_t id = w.id;
    const auto generation = w.generation;
    // Names and ids label the window; they never move, resize or restate it.
    const std::vector<xcb_atom_t> labels = {
            XCB_ATOM_WM_NAME,         atom("_NET_WM_NAME"),    XCB_ATOM_WM_CLASS,
            atom("WM_CLIENT_LEADER"), atom("_NET_STARTUP_ID"), atom("WM_CLIENT_MACHINE")};
    w.applyPending |= atoms.empty() || std::any_of(atoms.begin(), atoms.end(), [&](xcb_atom_t a) {
                          return std::find(labels.begin(), labels.end(), a) == labels.end();
                      });
    const std::vector<xcb_atom_t> props = atoms.empty() ? followedProperties() : atoms;
    for (auto a : props) {
        reply(xcb_get_property(m_connection, 0, id, a, XCB_ATOM_ANY, 0, 2048).sequence,
              [this, id, generation, a](void *p) {
                  auto it = m_windows.find(id);
                  if (it == m_windows.end() || it->second->generation != generation || !p)
                      return;
                  auto &w = *it->second;
                  auto *r = static_cast<xcb_get_property_reply_t *>(p);
                  const auto bytes = xcb_get_property_value_length(r);
                  auto *data = static_cast<const char *>(xcb_get_property_value(r));
                  auto *words = reinterpret_cast<const uint32_t *>(data);
                  const auto count = r->format == 32 ? bytes / 4 : 0;
                  if (a == atom("_NET_WM_SYNC_REQUEST_COUNTER")) {
                      // The basic counter; an extended second one is not used.
                      const xcb_sync_counter_t counter =
                              r->type == XCB_ATOM_CARDINAL && count >= 1 ? words[0] : 0;
                      if (counter != w.syncCounter) {
                          w.syncCounter = counter;
                          w.syncPending = false;
                          if (w.syncAlarm)
                              xcb_sync_destroy_alarm(m_connection, std::exchange(w.syncAlarm, 0));
                          // As KWin does: requests count up from a counter the
                          // window manager zeroed.
                          w.syncValue = 0;
                          if (counter && m_syncEvent)
                              xcb_sync_set_counter(m_connection, counter, {0, 0});
                      }
                  } else if (a == atom("WM_CLIENT_MACHINE"))
                      w.clientMachine = r->type == XCB_ATOM_STRING && r->format == 8
                              ? std::string(data, size_t(bytes))
                              : std::string();
                  else if (a == atom("_NET_WM_WINDOW_TYPE") && !w.roleLocked) {
                      w.dialog = w.popup = w.dragIcon = false;
                      if (r->type == XCB_ATOM_ATOM) {
                          for (int i = 0; i < count; ++i) {
                              if (words[i] == atom("_NET_WM_WINDOW_TYPE_NORMAL"))
                                  break;
                              if (words[i] == atom("_NET_WM_WINDOW_TYPE_DIALOG")) {
                                  w.dialog = true;
                                  break;
                              }
                              for (auto name :
                                   {"_NET_WM_WINDOW_TYPE_MENU", "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU",
                                    "_NET_WM_WINDOW_TYPE_POPUP_MENU", "_NET_WM_WINDOW_TYPE_TOOLTIP",
                                    "_NET_WM_WINDOW_TYPE_COMBO", "_NET_WM_WINDOW_TYPE_DND",
                                    "_NET_WM_WINDOW_TYPE_NOTIFICATION"})
                                  w.popup |= words[i] == atom(name);
                              w.dragIcon = words[i] == atom("_NET_WM_WINDOW_TYPE_DND");
                              if (w.popup)
                                  break;
                          }
                      }
                  } else if (a == XCB_ATOM_WM_NAME)
                      w.legacyTitle = r->type == XCB_ATOM_STRING && r->format == 8
                              ? base::strings::latin1ToUtf8(std::string_view(data, size_t(bytes)))
                              : std::string();
                  else if (a == atom("_NET_WM_NAME")) {
                      w.hasTitle = r->type == atom("UTF8_STRING") && r->format == 8;
                      w.title = w.hasTitle
                              ? base::strings::sanitizedUtf8(std::string_view(data, size_t(bytes)))
                              : std::string();
                  } else if (a == XCB_ATOM_WM_CLASS) {
                      w.instance.clear();
                      w.clazz.clear();
                      if (r->type == XCB_ATOM_STRING && r->format == 8) {
                          const auto parts =
                                  base::strings::split(std::string_view(data, size_t(bytes)), '\0');
                          if (parts.size() >= 2) {
                              w.instance = parts[0];
                              w.clazz = parts[1];
                          }
                      }
                  } else if (a == XCB_ATOM_WM_TRANSIENT_FOR)
                      w.transient = r->type == XCB_ATOM_WINDOW && count == 1 ? words[0] : 0;
                  else if (a == atom("WM_CLIENT_LEADER")) {
                      w.leader = r->type == XCB_ATOM_WINDOW && count == 1 ? words[0] : 0;
                      w.leaderClass.clear();
                      if (w.leader && w.leader != id) {
                          const auto leader = w.leader;
                          reply(xcb_get_property(m_connection, 0, leader, XCB_ATOM_WM_CLASS,
                                                 XCB_ATOM_STRING, 0, 2048)
                                        .sequence,
                                [this, id, generation, leader](void *p) {
                                    auto it = m_windows.find(id);
                                    if (!p || it == m_windows.end()
                                        || it->second->generation != generation
                                        || it->second->leader != leader)
                                        return;
                                    auto &w = *it->second;
                                    auto *r = static_cast<xcb_get_property_reply_t *>(p);
                                    if (r->type == XCB_ATOM_STRING && r->format == 8) {
                                        const auto parts =
                                                base::strings::split(propertyBytes(r), '\0');
                                        if (parts.size() >= 2)
                                            w.leaderClass = parts[1];
                                    }
                                    if (w.window && w.clazz.empty())
                                        w.window->setApplicationId(
                                                lowerLatin1ToUtf8(w.leaderClass));
                                });
                          reply(xcb_get_property(m_connection, 0, leader, atom("_NET_STARTUP_ID"),
                                                 atom("UTF8_STRING"), 0, 2048)
                                        .sequence,
                                [this, id, generation, leader](void *p) {
                                    auto it = m_windows.find(id);
                                    if (!p || it == m_windows.end()
                                        || it->second->generation != generation
                                        || it->second->leader != leader
                                        || !it->second->startupId.empty())
                                        return;
                                    auto *r = static_cast<xcb_get_property_reply_t *>(p);
                                    if (r->type == atom("UTF8_STRING") && r->format == 8)
                                        it->second->startupId =
                                                base::strings::sanitizedUtf8(propertyBytes(r));
                                });
                      }
                  } else if (a == atom("_NET_STARTUP_ID"))
                      w.startupId = r->format == 8 && r->type == atom("UTF8_STRING")
                              ? base::strings::sanitizedUtf8(std::string_view(data, size_t(bytes)))
                              : std::string();
                  else if (a == XCB_ATOM_WM_HINTS) {
                      w.urgent =
                              r->type == XCB_ATOM_WM_HINTS && count >= 1 && (words[0] & (1 << 8));
                      if (!w.window)
                          w.initiallyMinimized = r->type == XCB_ATOM_WM_HINTS && count >= 3
                                  && (words[0] & 2) && words[2] == 3;
                      w.input = !(r->type == XCB_ATOM_WM_HINTS && count >= 2 && (words[0] & 1))
                              || words[1];
                      updateAttention(w);
                  } else if (a == atom("WM_PROTOCOLS")) {
                      w.deleteWindow = w.takeFocus = w.pingable = w.syncable = false;
                      if (r->type == XCB_ATOM_ATOM) {
                          for (int i = 0; i < count; ++i) {
                              w.deleteWindow |= words[i] == atom("WM_DELETE_WINDOW");
                              w.takeFocus |= words[i] == atom("WM_TAKE_FOCUS");
                              w.pingable |= words[i] == atom("_NET_WM_PING");
                              w.syncable |= words[i] == atom("_NET_WM_SYNC_REQUEST");
                          }
                      }
                  } else if (a == atom("_NET_WM_WINDOW_OPACITY")) {
                      w.opacity = r->type == XCB_ATOM_CARDINAL && count >= 1
                              ? float(double(words[0]) / 0xffffffffu)
                              : 1;
                  } else if (a == atom("_MOTIF_WM_HINTS")) {
                      w.decorated = true;
                      if (r->type == a && count >= 3 && (words[0] & 2))
                          w.decorated = words[2] & 1 ? !(words[2] & (1 << 3)) : words[2] & (1 << 3);
                  } else if (a == XCB_ATOM_WM_NORMAL_HINTS) {
                      w.minimum = base::Size(1, 1);
                      w.maximum = base::Size();
                      w.increment = base::Size(1, 1);
                      w.base = base::Size(0, 0);
                      if (r->type == XCB_ATOM_WM_SIZE_HINTS && count >= 15) {
                          auto size = [words](int i) {
                              return base::Size(std::clamp<int32_t>(words[i], 1, 65535),
                                           std::clamp<int32_t>(words[i + 1], 1, 65535));
                          };
                          w.minimum = words[0] & (1 << 4) ? size(5) : base::Size(1, 1);
                          w.maximum = words[0] & (1 << 5) ? size(7).expandedTo(w.minimum) : base::Size();
                          w.increment = words[0] & (1 << 6) ? size(9) : base::Size(1, 1);
                          w.base = count >= 17 && (words[0] & (1 << 8))
                                  ? base::Size(std::clamp<int32_t>(words[15], 0, 65535),
                                          std::clamp<int32_t>(words[16], 0, 65535))
                                  : w.minimum;
                      }
                  } else if (a == atom("_NET_WM_STATE") && !w.roleLocked) {
                      w.fullscreen = w.maximized = w.demandsAttention = w.modal = false;
                      w.above = w.skipTaskbar = w.skipPager = false;
                      bool h = false, v = false;
                      for (int i = 0; r->type == XCB_ATOM_ATOM && i < count; ++i) {
                          w.above |= words[i] == atom("_NET_WM_STATE_ABOVE");
                          w.skipTaskbar |= words[i] == atom("_NET_WM_STATE_SKIP_TASKBAR");
                          w.skipPager |= words[i] == atom("_NET_WM_STATE_SKIP_PAGER");
                          w.demandsAttention |= words[i] == atom("_NET_WM_STATE_DEMANDS_ATTENTION");
                          w.fullscreen |= words[i] == atom("_NET_WM_STATE_FULLSCREEN");
                          w.modal |= words[i] == atom("_NET_WM_STATE_MODAL");
                          h |= words[i] == atom("_NET_WM_STATE_MAXIMIZED_HORZ");
                          v |= words[i] == atom("_NET_WM_STATE_MAXIMIZED_VERT");
                      }
                      w.maximized = h && v;
                      for (const auto &request : w.stateRequests)
                          applyStateRequest(w, request);
                      updateAttention(w);
                  }
              });
    }
    // _NET_WM_PID is whatever the client wrote; the server knows the process
    // behind the connection. Asked before the sentinel below, so it lands first.
    if (atoms.empty() && m_clientPids && !w.pid) {
        reply(queryClientPid(m_connection, id), [this, id, generation](void *p) {
            auto it = m_windows.find(id);
            if (!p || it == m_windows.end() || it->second->generation != generation)
                return;
            const int64_t pid = clientPidFromReply(p);
            if (pid > 0 && pid <= INT32_MAX && pid != getpid() && pid != m_serverPid)
                it->second->pid = uint32_t(pid);
        });
    }
    reply(xcb_get_input_focus(m_connection).sequence, [this, id, generation](void *) {
        auto it = m_windows.find(id);
        if (it == m_windows.end() || it->second->generation != generation)
            return;
        auto &w = *it->second;
        w.propertiesPending = false;
        if (std::exchange(w.allPropertiesDirty, false)) {
            w.dirtyProperties.clear();
            properties(w);
            return;
        }
        if (!w.dirtyProperties.empty()) {
            properties(w, std::exchange(w.dirtyProperties, {}));
            return;
        }
        if (!std::exchange(w.applyPending, false) && w.metadataReady) {
            if (w.window) {
                w.window->setTitle(displayTitle(w));
                w.window->setApplicationId(applicationId(w));
            }
            return;
        }
        w.metadataReady = true;
        if ((w.overrideRedirect || w.popup || w.transient == m_root)
            && (!w.transient || w.transient == m_root)) {
            for (auto &[otherId, other] : m_windows) {
                if (otherId != id && other->window && other->window->isActive()) {
                    w.transient = otherId;
                    break;
                }
            }
        }
        if (w.window) {
            w.window->setTitle(displayTitle(w));
            w.window->setApplicationId(applicationId(w));
            w.window->setMinimumSize(sizeFromX(w.minimum));
            w.window->setMaximumSize(w.maximum.isEmpty() ? base::Size() : sizeFromX(w.maximum));
            w.window->setDecorated(w.decorated);
            w.window->setOpacity(w.opacity);
            w.window->setFocusPolicy(w.input || w.takeFocus ? wm::FocusPolicy::Normal
                                                            : wm::FocusPolicy::NoFocus);
            w.window->setModal(w.modal);
        }
        present(w);
        if (w.window)
            synchronize(w);
        for (auto &[id, child] : m_windows) {
            if (child->window) {
                auto parent = m_windows.find(child->transient);
                child->window->setTransientParent(
                        parent == m_windows.end() ? nullptr : parent->second->window);
            }
        }
    });
}

void Xwm::present(X11Window &w)
{
    if (w.surface) {
        const auto bounding = w.boundingShape ? std::optional(regionFromX(*w.boundingShape))
                                              : std::nullopt;
        w.surface->node()->setContentShape(bounding);
        auto input = w.inputShape ? std::optional(regionFromX(*w.inputShape)) : std::nullopt;
        if (bounding)
            input = input ? *input & *bounding : bounding;
        // A drag icon follows the pointer; it must never be what is under it.
        if (w.dragIcon)
            input = base::Region();
        w.surface->setExternalInputRegion(input ? std::optional(*input) : std::nullopt);
    }
    if (!w.surface || !w.mapped || !w.metadataReady || !w.surface->hasContents())
        return;
    // EWMH requires the window type before mapping; keep its role until withdrawal.
    w.roleLocked = true;
    if (w.overrideRedirect || w.popup) {
        w.surface->setPosition(w.geometry.topLeft());
        w.surface->setHidden(false);
        auto owner = m_windows.find(w.transient);
        auto *parent = owner != m_windows.end() ? owner->second->window : nullptr;
        w.surface->setHidden(parent && !parent->isVisible());
        m_manager->setAuxiliarySurface(w.surface, parent);
        return;
    }
    if (w.window) {
        const auto edge = m_manager->eventHandler()->draggedWindow() == w.window
                ? m_manager->eventHandler()->draggedEdge()
                : wm::ResizeEdge::None;
        const auto delta = w.window->contentBounds().size() - w.surface->contentSize();
        w.window->setSurfaceOrigin(base::Point(wm::isLeftEdge(edge) ? delta.width() : 0,
                                          wm::isTopEdge(edge) ? delta.height() : 0));
        return;
    }
    wm::WindowInit init;
    init.title = displayTitle(w);
    init.applicationId = applicationId(w);
    init.processId = ownerPid(w);
    init.decorated = w.decorated;
    init.requestedSize = w.surface->contentSize();
    init.minimumSize = sizeFromX(w.minimum);
    init.maximumSize = w.maximum.isEmpty() ? base::Size() : sizeFromX(w.maximum);
    init.focusPolicy = w.input || w.takeFocus ? wm::FocusPolicy::Normal : wm::FocusPolicy::NoFocus;
    auto parent = m_windows.find(w.transient);
    init.transientParent = parent == m_windows.end() ? nullptr : parent->second->window;
    init.modal = w.modal;
    init.closeCallback = [this, id = w.id] {
        auto it = m_windows.find(id);
        if (it == m_windows.end())
            return;
        if (it->second->deleteWindow)
            message(id, atom("WM_DELETE_WINDOW"));
        else {
            xcb_kill_client(m_connection, id);
            xcb_flush(m_connection);
        }
    };
    w.surface->setHidden(false);
    applyHints(w);
    w.window = m_manager->createWindow(w.surface, init);
    if (!w.window)
        return;
    updateAttention(w);
    // From here the state goes to the window and back into the property.
    w.stateRequests.clear();
    w.window->setPingCallback([this, id = w.id] {
        ping(id);
    });
    w.window->setKillCallback([this, id = w.id] {
        kill(id);
    });
    property(w.id, "_NET_WM_DESKTOP", XCB_ATOM_CARDINAL, {0});
    w.window->addObserver(&w);
    w.window->setOpacity(w.opacity);
    w.window->setMinimumSize(sizeFromX(w.minimum));
    w.window->setMaximumSize(w.maximum.isEmpty() ? base::Size() : sizeFromX(w.maximum));
    if (w.maximized)
        w.window->maximize();
    if (w.fullscreen)
        w.window->setFullscreen(true);
    if (w.initiallyMinimized)
        w.window->minimize();
    else
        w.window->activate();
    synchronize(w);
    roots();
}

void Xwm::detach(X11Window &w)
{
    auto *surface = std::exchange(w.surface, nullptr);
    auto *window = std::exchange(w.window, nullptr);
    w.writtenExtents.reset();
    w.writtenState.reset();
    w.writtenWmState.reset();
    if (surface) {
        surface->removeSurfaceObserver(&w);
        surface->setHidden(true);
        surface->node()->setContentShape(std::nullopt);
        surface->setExternalInputRegion(std::nullopt);
        m_manager->clearSurfaceOwner(surface);
    }
    if (window) {
        window->removeObserver(&w);
        window->setCloseCallback({});
        m_manager->destroyWindow(window);
    }
}

Xwm::X11Window *Xwm::adopt(uint32_t window, const base::Rect &xGeometry, bool overrideRedirect)
{
    if (window == m_support || (m_dnd && m_dnd->owns(window)))
        return nullptr;
    const uint32_t clientBits = ~xcb_get_setup(m_connection)->resource_id_mask;
    if (m_windows.size() < std::min(m_windowLimit, SurfaceAssociation::limit)) {
        m_disconnecting = 0;
    } else if ((window & clientBits) == m_disconnecting) {
        // Created before the disconnection took effect; its windows are going.
        return nullptr;
    } else {
        // Only a client holding that many windows gets here. It is the one
        // to go; restarting Xwayland would take every other X11 client too.
        std::map<uint32_t, std::pair<size_t, uint32_t>> clients;
        for (const auto &[id, other] : m_windows) {
            if ((id & clientBits) == (m_support & clientBits))
                continue;
            auto &client = clients[id & clientBits];
            ++client.first;
            client.second = id;
        }
        const auto greediest = std::max_element(
                clients.begin(), clients.end(), [](const auto &a, const auto &b) {
                    return a.second.first < b.second.first;
                });
        if (greediest == clients.end())
            return nullptr;
        if (greediest->first != m_disconnecting) {
            LM_WARNING(logs())
                    << "Xwayland window limit reached; disconnecting the X11 client with"
                    << greediest->second.first << "windows";
            xcb_kill_client(m_connection, greediest->second.second);
            xcb_flush(m_connection);
            m_disconnecting = greediest->first;
        }
        if ((window & clientBits) == greediest->first)
            return nullptr;
    }
    auto w = std::make_unique<X11Window>(this, window, ++m_generation);
    w->xGeometry = xGeometry;
    w->geometry = base::Rect(fromX(w->xGeometry.topLeft()), sizeFromX(w->xGeometry.size()));
    w->overrideRedirect = overrideRedirect;
    // Added to what is already selected: the selection bridge asks a requestor
    // for STRUCTURE_NOTIFY mid-transfer, and one adopted here keeps it.
    adjustEventMask(window, kWindowEvents, 0);
    if (m_shapeEvent) {
        xcb_shape_select_input(m_connection, window, 1);
        shape(*w);
    }
    auto *adopted = w.get();
    m_windows[window] = std::move(w);
    return adopted;
}

void Xwm::adjustEventMask(uint32_t window, uint32_t set, uint32_t clear)
{
    const auto cookie = xcb_get_window_attributes(m_connection, window);
    xcb_get_window_attributes_reply_t *reply =
            xcb_get_window_attributes_reply(m_connection, cookie, nullptr);
    if (!reply)
        return;
    uint32_t mask = (reply->your_event_mask | set) & ~clear;
    free(reply);
    xcb_change_window_attributes(m_connection, window, XCB_CW_EVENT_MASK, &mask);
}

void Xwm::forget(uint32_t window)
{
    auto it = m_windows.find(window);
    if (it == m_windows.end())
        return;
    m_association->removeWindow(window);
    auto owned = std::move(it->second);
    m_windows.erase(it);
    detach(*owned);
    for (auto &[childId, child] : m_windows) {
        if (child->transient == window) {
            child->transient = 0;
            if (child->overrideRedirect || child->popup)
                present(*child);
        }
    }
}

void Xwm::synchronize(X11Window &w)
{
    if (!w.window || w.syncing)
        return;
    w.syncing = true;
    for (auto &[id, child] : m_windows) {
        if (child->transient == w.id && (child->overrideRedirect || child->popup))
            present(*child);
    }
    base::Rect rect = w.window->contentBounds();
    // Size hints are in X11 pixels, so the increments are applied there; a
    // character cell is not a whole number of logical units at every scale.
    base::Size xSize = sizeToX(rect.size());
    if (w.window->stateType() == wm::WindowStateType::Normal) {
        base::Size constrained = xSize;
        constrained.setWidth(constrainedExtent(xSize.width(), w.base.width(), w.increment.width(),
                                               w.minimum.width(), w.maximum.width()));
        constrained.setHeight(constrainedExtent(xSize.height(), w.base.height(),
                                                w.increment.height(), w.minimum.height(),
                                                w.maximum.height()));
        if (constrained != xSize) {
            const base::Size size = sizeFromX(constrained);
            if (size != rect.size()) {
                w.window->setBounds(base::Rect(w.window->bounds().topLeft(),
                                               w.window->frameInsets().outset(size)));
                rect = w.window->contentBounds();
            }
            xSize = rect.size() == size ? constrained : sizeToX(rect.size());
        }
    }
    present(w);
    w.geometry = rect;
    const base::Point xPosition = toX(rect.topLeft());
    // A client that paints to a sync counter takes one size at a time; a newer
    // one waits for the counter, as in KWin, and only the latest is sent.
    bool deferred = false;
    if (xSize != w.xGeometry.size()) {
        if (w.syncPending)
            deferred = w.syncDeferred = true;
        else
            requestSync(w);
    }
    if (!deferred) {
        w.sizeUnknown = false;
        w.xGeometry = base::Rect(xPosition, xSize);
        uint32_t values[] = {uint32_t(xPosition.x()), uint32_t(xPosition.y()),
                             uint32_t(xSize.width()), uint32_t(xSize.height()), 0};
        xcb_configure_window(m_connection, w.id,
                             XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH
                                     | XCB_CONFIG_WINDOW_HEIGHT | XCB_CONFIG_WINDOW_BORDER_WIDTH,
                             values);
        xcb_configure_notify_event_t e {};
        e.response_type = XCB_CONFIGURE_NOTIFY;
        e.event = e.window = w.id;
        e.x = xPosition.x();
        e.y = xPosition.y();
        e.width = xSize.width();
        e.height = xSize.height();
        xcb_send_event(m_connection, 0, w.id, XCB_EVENT_MASK_STRUCTURE_NOTIFY,
                       reinterpret_cast<char *>(&e));
    }
    auto insets = w.window->frameInsets();
    auto x = [this](int value) {
        return uint32_t(base::roundToInt(value * m_scale));
    };
    property(w.id, "_NET_FRAME_EXTENTS", XCB_ATOM_CARDINAL,
             {x(insets.left), x(insets.right), x(insets.top), x(insets.bottom)}, w.writtenExtents);
    std::vector<uint32_t> states;
    // Presented, not actual: a window being dragged out of a tiled state has
    // already stopped being maximized as far as the client is concerned.
    const auto state = w.window->presentedStateType();
    if (state == wm::WindowStateType::Fullscreen)
        states.push_back(atom("_NET_WM_STATE_FULLSCREEN"));
    if (state == wm::WindowStateType::Maximized
        || (state == wm::WindowStateType::Fullscreen
            && w.window->state()->restoreStateType() == wm::WindowStateType::Maximized)) {
        states.push_back(atom("_NET_WM_STATE_MAXIMIZED_HORZ"));
        states.push_back(atom("_NET_WM_STATE_MAXIMIZED_VERT"));
    }
    const bool iconic = state == wm::WindowStateType::Minimized || w.window->isHidden();
    if (iconic)
        states.push_back(atom("_NET_WM_STATE_HIDDEN"));
    if (w.window->isModal())
        states.push_back(atom("_NET_WM_STATE_MODAL"));
    if (w.above)
        states.push_back(atom("_NET_WM_STATE_ABOVE"));
    if (w.skipTaskbar)
        states.push_back(atom("_NET_WM_STATE_SKIP_TASKBAR"));
    if (w.skipPager)
        states.push_back(atom("_NET_WM_STATE_SKIP_PAGER"));
    if (w.window->isActive())
        states.push_back(atom("_NET_WM_STATE_FOCUSED"));
    else if (w.urgent || w.demandsAttention)
        states.push_back(atom("_NET_WM_STATE_DEMANDS_ATTENTION"));
    property(w.id, "_NET_WM_STATE", XCB_ATOM_ATOM, std::move(states), w.writtenState);
    property(w.id, "WM_STATE", atom("WM_STATE"), {iconic ? 3u : 1u, 0}, w.writtenWmState);
    xcb_flush(m_connection);
    w.syncing = false;
}

void Xwm::focus(X11Window &w, bool active)
{
    if (active) {
        w.demandsAttention = false;
        w.urgent = false;
        updateAttention(w);
        uint32_t mode = XCB_STACK_MODE_ABOVE;
        xcb_configure_window(m_connection, w.id, XCB_CONFIG_WINDOW_STACK_MODE, &mode);
        // Obtain X server time without blocking; WM_TAKE_FOCUS forbids CurrentTime.
        property(m_support, "_LINGMO_FOCUS_TIMESTAMP", XCB_ATOM_INTEGER, {w.id});
    } else {
        xcb_set_input_focus(m_connection, XCB_INPUT_FOCUS_POINTER_ROOT, XCB_NONE, XCB_CURRENT_TIME);
    }
    synchronize(w);
    roots();
    xcb_flush(m_connection);
}

void Xwm::roots()
{
    if (!m_root || m_atoms.empty())
        return;
    m_lastOutput = m_manager->outputBounds();
    m_lastWorkArea = m_manager->workAreaFor(nullptr);
    const base::Size desktop = sizeToX(m_lastOutput.size());
    property(m_root, "_NET_DESKTOP_GEOMETRY", XCB_ATOM_CARDINAL,
             {uint32_t(desktop.width()), uint32_t(desktop.height())});
    const base::Point workAreaOrigin = toX(m_lastWorkArea.topLeft());
    const base::Size workArea = sizeToX(m_lastWorkArea.size());
    property(m_root, "_NET_WORKAREA", XCB_ATOM_CARDINAL,
             {uint32_t(workAreaOrigin.x()), uint32_t(workAreaOrigin.y()),
              uint32_t(workArea.width()), uint32_t(workArea.height())});
    property(m_root, "_NET_NUMBER_OF_DESKTOPS", XCB_ATOM_CARDINAL, {1});
    property(m_root, "_NET_CURRENT_DESKTOP", XCB_ATOM_CARDINAL, {0});
    std::vector<uint32_t> list, stacking;
    uint32_t active = XCB_NONE;
    for (auto &[id, w] : m_windows) {
        if (w->window)
            list.push_back(id);
    }
    std::sort(list.begin(), list.end(), [this](uint32_t a, uint32_t b) {
        return m_windows.at(a)->mapOrder < m_windows.at(b)->mapOrder;
    });
    for (auto *window : m_manager->windows()) {
        for (auto &[id, w] : m_windows) {
            if (w->window == window) {
                stacking.push_back(id);
                if (window->isActive())
                    active = id;
            }
        }
    }
    property(m_root, "_NET_CLIENT_LIST", XCB_ATOM_WINDOW, list);
    property(m_root, "_NET_CLIENT_LIST_STACKING", XCB_ATOM_WINDOW, stacking);
    property(m_root, "_NET_ACTIVE_WINDOW", XCB_ATOM_WINDOW, {active});
}

void Xwm::event(xcb_generic_event_t *generic)
{
    const auto type = generic->response_type & 0x7f;
    if (m_shapeEvent && type == m_shapeEvent + XCB_SHAPE_NOTIFY
        && !(generic->response_type & 0x80)) {
        auto *e = reinterpret_cast<xcb_shape_notify_event_t *>(generic);
        auto it = m_windows.find(e->affected_window);
        if (it != m_windows.end())
            shape(*it->second);
        return;
    }
    if (m_syncEvent && type == m_syncEvent + XCB_SYNC_ALARM_NOTIFY) {
        auto *e = reinterpret_cast<xcb_sync_alarm_notify_event_t *>(generic);
        for (auto &[id, w] : m_windows) {
            if (w->syncAlarm != e->alarm)
                continue;
            w->syncPending = false;
            if (std::exchange(w->syncDeferred, false))
                synchronize(*w);
            break;
        }
        return;
    }
    if (m_dnd && m_dnd->event(generic))
        return;
    if (m_primarySelection && m_primarySelection->event(generic))
        return;
    if (m_selection && m_selection->event(generic))
        return;
    if ((generic->response_type & 0x80) && type != XCB_CLIENT_MESSAGE)
        return;
    if (type == XCB_PROPERTY_NOTIFY) {
        auto *event = reinterpret_cast<xcb_property_notify_event_t *>(generic);
        if (event->window == m_support && event->atom == atom("_LINGMO_FOCUS_TIMESTAMP")) {
            for (auto &[id, window] : m_windows) {
                if (!window->window || !window->window->isActive())
                    continue;
                if (window->input)
                    xcb_set_input_focus(m_connection, XCB_INPUT_FOCUS_POINTER_ROOT, id,
                                        event->time);
                if (window->takeFocus)
                    message(id, atom("WM_TAKE_FOCUS"), event->time);
                break;
            }
            xcb_flush(m_connection);
            return;
        }
    }
    if (!type) {
        auto *error = reinterpret_cast<xcb_generic_error_t *>(generic);
        LM_WARNING(logs()) << "XWM request error" << int(error->error_code)
                           << int(error->major_code);
        if (!m_ready) {
            m_initializationFailed = true;
            if (initialized)
                initialized(false);
        }
        return;
    }
    if (type == XCB_CREATE_NOTIFY) {
        auto *e = reinterpret_cast<xcb_create_notify_event_t *>(generic);
        if (e->parent == m_root)
            adopt(e->window, base::Rect(e->x, e->y, e->width, e->height), e->override_redirect);
        return;
    }
    if (type == XCB_REPARENT_NOTIFY) {
        auto *e = reinterpret_cast<xcb_reparent_notify_event_t *>(generic);
        if (e->parent == m_root && !m_windows.contains(e->window)) {
            // Moved onto the root from another window; the event has no size.
            auto *w = adopt(e->window, base::Rect(e->x, e->y, 1, 1), e->override_redirect);
            if (!w)
                return;
            w->sizeUnknown = true;
            const auto id = w->id;
            const auto generation = w->generation;
            reply(xcb_get_geometry(m_connection, id).sequence, [this, id, generation](void *p) {
                auto it = m_windows.find(id);
                if (!p || it == m_windows.end() || it->second->generation != generation)
                    return;
                auto &w = *it->second;
                // A configure handled since then is at least as new as the reply.
                if (!std::exchange(w.sizeUnknown, false) || w.window)
                    return;
                auto *r = static_cast<xcb_get_geometry_reply_t *>(p);
                w.xGeometry = base::Rect(r->x, r->y, r->width, r->height);
                w.geometry = base::Rect(fromX(w.xGeometry.topLeft()), sizeFromX(w.xGeometry.size()));
                present(w);
            });
            return;
        }
    }
    if (type == XCB_CLIENT_MESSAGE) {
        auto *e = reinterpret_cast<xcb_client_message_event_t *>(generic);
        if (e->format == 32 && e->window == m_root && e->type == atom("WM_PROTOCOLS")
            && e->data.data32[0] == atom("_NET_WM_PING")) {
            auto it = m_windows.find(e->data.data32[2]);
            if (it != m_windows.end() && it->second->pingSerial
                && e->data.data32[1] == it->second->pingSerial) {
                it->second->pingSerial = 0;
                if (it->second->window)
                    it->second->window->setResponsive(true);
            }
            return;
        }
    }
    uint32_t id = 0;
    switch (type) {
    case XCB_MAP_REQUEST:
        id = reinterpret_cast<xcb_map_request_event_t *>(generic)->window;
        break;
    case XCB_MAP_NOTIFY:
        id = reinterpret_cast<xcb_map_notify_event_t *>(generic)->window;
        break;
    case XCB_UNMAP_NOTIFY:
        id = reinterpret_cast<xcb_unmap_notify_event_t *>(generic)->window;
        break;
    case XCB_DESTROY_NOTIFY:
        id = reinterpret_cast<xcb_destroy_notify_event_t *>(generic)->window;
        break;
    case XCB_CONFIGURE_REQUEST:
        id = reinterpret_cast<xcb_configure_request_event_t *>(generic)->window;
        break;
    case XCB_CONFIGURE_NOTIFY:
        id = reinterpret_cast<xcb_configure_notify_event_t *>(generic)->window;
        break;
    case XCB_REPARENT_NOTIFY:
        id = reinterpret_cast<xcb_reparent_notify_event_t *>(generic)->window;
        break;
    case XCB_FOCUS_IN:
    case XCB_FOCUS_OUT:
        id = reinterpret_cast<xcb_focus_in_event_t *>(generic)->event;
        break;
    case XCB_PROPERTY_NOTIFY:
        id = reinterpret_cast<xcb_property_notify_event_t *>(generic)->window;
        break;
    case XCB_CLIENT_MESSAGE:
        id = reinterpret_cast<xcb_client_message_event_t *>(generic)->window;
        break;
    default:
        return;
    }
    auto it = m_windows.find(id);
    if (it == m_windows.end())
        return;
    auto &w = *it->second;
    if (type == XCB_FOCUS_IN) {
        auto *e = reinterpret_cast<xcb_focus_in_event_t *>(generic);
        if (e->mode == XCB_NOTIFY_MODE_NORMAL && w.window && !w.window->isActive()) {
            // Delayed X focus notifications must not override a newer WM activation.
            X11Window *active = nullptr;
            for (auto &[otherId, other] : m_windows) {
                if (other->window && other->window->isActive())
                    active = other.get();
            }
            if (active)
                focus(*active, true);
            else
                focus(w, false);
        }
    } else if (type == XCB_REPARENT_NOTIFY) {
        // Only children of the root are followed; the DestroyNotify of one
        // moved into another window goes to its new parent instead.
        if (reinterpret_cast<xcb_reparent_notify_event_t *>(generic)->parent != m_root) {
            // No longer managed: stop its events and shape changes from
            // arriving for a window the lookup would only drop.
            adjustEventMask(id, 0, kWindowEvents);
            if (m_shapeEvent)
                xcb_shape_select_input(m_connection, id, 0);
            forget(id);
            roots();
        }
    } else if (type == XCB_MAP_REQUEST) {
        // A read from before the request can miss state set just before mapping,
        // and the window is created, its role locked, from whatever was read.
        w.metadataReady = false;
        properties(w);
        // Mapping is the prerequisite for Xwayland to produce a wl_surface.
        xcb_map_window(m_connection, id);
    } else if (type == XCB_MAP_NOTIFY) {
        w.overrideRedirect = reinterpret_cast<xcb_map_notify_event_t *>(generic)->override_redirect;
        if (!w.mapped)
            w.mapOrder = ++m_mapOrder;
        w.mapped = true;
        shape(w);
        properties(w);
        present(w);
    } else if (type == XCB_UNMAP_NOTIFY || type == XCB_DESTROY_NOTIFY) {
        w.mapped = false;
        w.roleLocked = w.metadataReady = false;
        w.stateRequests.clear();
        if (type == XCB_DESTROY_NOTIFY) {
            forget(id);
        } else {
            m_association->removeWindow(id);
            detach(w);
            property(id, "WM_STATE", atom("WM_STATE"), {0, 0});
        }
        roots();
    } else if (type == XCB_PROPERTY_NOTIFY) {
        auto *e = reinterpret_cast<xcb_property_notify_event_t *>(generic);
        if (e->atom == XCB_ATOM_WM_CLASS || e->atom == atom("_NET_STARTUP_ID")) {
            for (auto &[childId, child] : m_windows) {
                if (childId != id && child->leader == id)
                    properties(*child, {atom("WM_CLIENT_LEADER")});
            }
        }
        // Only what changed is read again. Anything else, such as the
        // _NET_WM_USER_TIME GTK updates on every key press, is not followed,
        // and _NET_WM_STATE is the window manager's own to write.
        const auto followed = followedProperties();
        if (e->atom != atom("_NET_WM_STATE")
            && std::find(followed.begin(), followed.end(), e->atom) != followed.end())
            properties(w, {e->atom});
    } else if (type == XCB_CONFIGURE_REQUEST) {
        auto *e = reinterpret_cast<xcb_configure_request_event_t *>(generic);
        // The request in X11 pixels, then in logical units.
        base::Rect xRect = w.window ? base::Rect(toX(w.window->contentBounds().topLeft()),
                                                 sizeToX(w.window->contentBounds().size()))
                                    : w.xGeometry;
        // The event fills what the request leaves out from the server's geometry, which
        // can predate our answer to an earlier request; it only replaces an unknown size.
        if (!w.window && std::exchange(w.sizeUnknown, false))
            xRect = base::Rect(e->x, e->y, e->width, e->height);
        if (e->value_mask & XCB_CONFIG_WINDOW_X)
            xRect.moveLeft(e->x);
        if (e->value_mask & XCB_CONFIG_WINDOW_Y)
            xRect.moveTop(e->y);
        if (e->value_mask & XCB_CONFIG_WINDOW_WIDTH)
            xRect.setWidth(std::max<int>(1, e->width));
        if (e->value_mask & XCB_CONFIG_WINDOW_HEIGHT)
            xRect.setHeight(std::max<int>(1, e->height));
        const base::Rect rect(fromX(xRect.topLeft()), sizeFromX(xRect.size()));
        if (w.window) {
            auto insets = w.window->frameInsets();
            w.window->requestBounds(base::Rect(rect.topLeft() - base::Point(insets.left, insets.top),
                                               insets.outset(rect.size())));
            synchronize(w);
        } else {
            w.geometry = rect;
            w.xGeometry = xRect;
            uint32_t values[] = {uint32_t(xRect.x()), uint32_t(xRect.y()),
                                 uint32_t(xRect.width()), uint32_t(xRect.height())};
            xcb_configure_window(m_connection, id, 15, values);
        }
    } else if (type == XCB_CONFIGURE_NOTIFY) {
        auto *e = reinterpret_cast<xcb_configure_notify_event_t *>(generic);
        shape(w);
        if (!w.window) {
            w.sizeUnknown = false;
            w.xGeometry = base::Rect(e->x, e->y, e->width, e->height);
            w.geometry = base::Rect(fromX(w.xGeometry.topLeft()), sizeFromX(w.xGeometry.size()));
            present(w);
        }
    } else if (type == XCB_CLIENT_MESSAGE) {
        auto *e = reinterpret_cast<xcb_client_message_event_t *>(generic);
        if (e->format != 32)
            return;
        auto *d = e->data.data32;
        if (e->type == atom("WL_SURFACE_SERIAL")) {
            // Xwayland delivers its own; a copy another client sent could take a
            // serial ahead of Xwayland's and leave every later window unmatched.
            if (generic->response_type & 0x80)
                return;
            if (!m_association->addWindow(SurfaceAssociation::serial(d[0], d[1]), id))
                LM_WARNING(logs()) << "Xwayland rejected window serial" << id;
        } else if (e->type == atom("_NET_REQUEST_FRAME_EXTENTS")) {
            // A client asking waits for the property to be written, changed or not.
            w.writtenExtents.reset();
            if (w.window)
                synchronize(w);
            else
                property(id, "_NET_FRAME_EXTENTS", XCB_ATOM_CARDINAL, {0, 0, 0, 0});
        } else if (e->type == atom("_NET_WM_STATE") && d[0] <= 2 && !w.window && w.mapped
                   && !w.overrideRedirect && !w.popup) {
            // Mapped, so the client asks rather than sets the property, but the
            // window is only created once Xwayland's surface has content. SDL
            // and GTK ask for fullscreen right after mapping. A client that
            // keeps asking before then is not remembered past a few requests.
            if (w.stateRequests.size() < 16) {
                w.stateRequests.push_back({d[0], d[1], d[2]});
                applyStateRequest(w, w.stateRequests.back());
            }
        } else if (w.window) {
            if (e->type == atom("_NET_WM_MOVERESIZE")) {
                auto *seat = m_manager->seat();
                auto *handler = m_manager->eventHandler();
                if (d[2] == 11 && handler->draggedWindow() == w.window) {
                    handler->completeDrag(wm::ToplevelEventHandler::DragResult::Revert);
                } else if (seat) {
                    constexpr wm::ResizeEdge edges[] = {
                            wm::ResizeEdge::TopLeft,     wm::ResizeEdge::Top,
                            wm::ResizeEdge::TopRight,    wm::ResizeEdge::Right,
                            wm::ResizeEdge::BottomRight, wm::ResizeEdge::Bottom,
                            wm::ResizeEdge::BottomLeft,  wm::ResizeEdge::Left};
                    // The request carries no serial; what is held on the window
                    // decides, the pointer's press first.
                    std::optional<base::PointF> location;
                    std::optional<int32_t> touchId;
                    if (seat->pointerGrabSurface() == w.surface && seat->pressedButtons()) {
                        location = seat->pointerLocation();
                    } else if (const auto point = seat->touchPointOn(w.surface)) {
                        location = point->location;
                        touchId = point->id;
                    }
                    if (location && d[2] < std::size(edges))
                        handler->beginResize(w.window, edges[d[2]], *location, touchId);
                    else if (location && d[2] == 8)
                        handler->beginMove(w.window, *location, touchId);
                }
            } else if (e->type == atom("_NET_ACTIVE_WINDOW")) {
                if (!w.window->isVisible())
                    w.window->restore();
                w.window->activate();
            } else if (e->type == atom("_NET_CLOSE_WINDOW")) {
                w.window->requestClose();
            } else if (e->type == atom("WM_CHANGE_STATE") && d[0] == 3) {
                w.window->minimize();
            } else if (e->type == atom("_NET_WM_STATE") && d[0] <= 2) {
                for (int i = 1; i <= 2; ++i) {
                    if (i == 2 && d[1] == d[2])
                        continue;
                    const auto current = w.window->stateType();
                    if (d[i] == atom("_NET_WM_STATE_DEMANDS_ATTENTION")) {
                        w.demandsAttention = d[0] == 1 || (d[0] == 2 && !w.demandsAttention);
                        updateAttention(w);
                        synchronize(w);
                    } else if (d[i] == atom("_NET_WM_STATE_ABOVE")
                               || d[i] == atom("_NET_WM_STATE_SKIP_TASKBAR")
                               || d[i] == atom("_NET_WM_STATE_SKIP_PAGER")) {
                        bool &state = d[i] == atom("_NET_WM_STATE_ABOVE")  ? w.above
                                : d[i] == atom("_NET_WM_STATE_SKIP_TASKBAR") ? w.skipTaskbar
                                                                              : w.skipPager;
                        state = d[0] == 1 || (d[0] == 2 && !state);
                        applyHints(w);
                        synchronize(w);
                    } else if (d[i] == atom("_NET_WM_STATE_MODAL")) {
                        w.modal = d[0] == 1 || (d[0] == 2 && !w.modal);
                        w.window->setModal(w.modal);
                        synchronize(w);
                    } else if (d[i] == atom("_NET_WM_STATE_FULLSCREEN"))
                        w.window->setFullscreen(
                                d[0] == 1
                                || (d[0] == 2 && current != wm::WindowStateType::Fullscreen));
                    else if (d[i] == atom("_NET_WM_STATE_MAXIMIZED_HORZ")
                             || d[i] == atom("_NET_WM_STATE_MAXIMIZED_VERT")) {
                        if (i == 2
                            && (d[1] == atom("_NET_WM_STATE_MAXIMIZED_HORZ")
                                || d[1] == atom("_NET_WM_STATE_MAXIMIZED_VERT")))
                            continue;
                        if (d[0] == 1 || (d[0] == 2 && current != wm::WindowStateType::Maximized))
                            w.window->maximize();
                        else if (current == wm::WindowStateType::Maximized)
                            w.window->restore();
                    }
                }
            }
        }
    }
}

} // namespace Lingmo::XWayland
