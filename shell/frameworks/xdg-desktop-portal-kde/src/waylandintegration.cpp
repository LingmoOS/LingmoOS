/*
 * SPDX-FileCopyrightText: 2018 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2018 Jan Grulich <jgrulich@redhat.com>
 * SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>
 */

#include "waylandintegration.h"
#include "qwayland-wayland.h"
#include "screencast.h"
#include "screencasting.h"
#include "waylandintegration_debug.h"
#include "waylandintegration_p.h"

#include <QDBusMetaType>
#include <QGuiApplication>

#include <KNotification>
#include <QEventLoop>
#include <QImage>
#include <QMenu>
#include <QScreen>
#include <QThread>
#include <QTimer>
#include <QWaylandClientExtensionTemplate>

#include <KLocalizedString>

// KWayland
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/plasmawindowmanagement.h>
#include <KWayland/Client/plasmawindowmodel.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/surface.h>
#include <KWayland/Client/xdgforeign.h>

// system
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <sys/mman.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon.h>

#include <qpa/qplatformnativeinterface.h>
#include <waylandintegration_debug.h>

Q_GLOBAL_STATIC(WaylandIntegration::WaylandIntegrationPrivate, globalWaylandIntegration)

namespace WaylandIntegration
{
FakeInput::FakeInput()
    : QWaylandClientExtensionTemplate<FakeInput>(5)
{
    initialize();
}

FakeInput::~FakeInput()
{
    if (isActive()) {
        destroy();
    }
}

QDebug operator<<(QDebug dbg, const Stream &c)
{
    dbg.nospace() << "Stream(" << c.map << ", " << c.nodeId << ")";
    return dbg.space();
}

const QDBusArgument &operator>>(const QDBusArgument &arg, Stream &stream)
{
    arg.beginStructure();
    arg >> stream.nodeId;

    arg.beginMap();
    while (!arg.atEnd()) {
        QString key;
        QVariant map;
        arg.beginMapEntry();
        arg >> key >> map;
        arg.endMapEntry();
        stream.map.insert(key, map);
    }
    arg.endMap();
    arg.endStructure();

    return arg;
}

const QDBusArgument &operator<<(QDBusArgument &arg, const Stream &stream)
{
    arg.beginStructure();
    arg << stream.nodeId;
    arg << stream.map;
    arg.endStructure();

    return arg;
}

}

void WaylandIntegration::init()
{
    globalWaylandIntegration->initWayland();
}

bool WaylandIntegration::isStreamingEnabled()
{
    return globalWaylandIntegration->isStreamingEnabled();
}

bool WaylandIntegration::isStreamingAvailable()
{
    return globalWaylandIntegration->isStreamingAvailable();
}

void WaylandIntegration::acquireStreamingInput(bool acquire)
{
    globalWaylandIntegration->acquireStreamingInput(acquire);
}

WaylandIntegration::Stream WaylandIntegration::startStreamingOutput(QScreen *screen, Screencasting::CursorMode mode)
{
    return globalWaylandIntegration->startStreamingOutput(screen, mode);
}

WaylandIntegration::Stream WaylandIntegration::startStreamingWorkspace(Screencasting::CursorMode mode)
{
    return globalWaylandIntegration->startStreamingWorkspace(mode);
}

WaylandIntegration::Stream WaylandIntegration::startStreamingRegion(const QRect &region, Screencasting::CursorMode mode)
{
    return globalWaylandIntegration->startStreamingRegion(region, mode);
}

WaylandIntegration::Stream WaylandIntegration::startStreamingVirtual(const QString &name, const QSize &size, Screencasting::CursorMode mode)
{
    return globalWaylandIntegration->startStreamingVirtualOutput(name, size, mode);
}

WaylandIntegration::Stream WaylandIntegration::startStreamingWindow(const QMap<int, QVariant> &win, Screencasting::CursorMode mode)
{
    return globalWaylandIntegration->startStreamingWindow(win, mode);
}

void WaylandIntegration::stopStreaming(uint node)
{
    globalWaylandIntegration->stopStreaming(node);
}

void WaylandIntegration::requestPointerButtonPress(quint32 linuxButton)
{
    globalWaylandIntegration->requestPointerButtonPress(linuxButton);
}

void WaylandIntegration::requestPointerButtonRelease(quint32 linuxButton)
{
    globalWaylandIntegration->requestPointerButtonRelease(linuxButton);
}

void WaylandIntegration::requestPointerMotion(const QSizeF &delta)
{
    globalWaylandIntegration->requestPointerMotion(delta);
}

void WaylandIntegration::requestPointerMotionAbsolute(uint stream, const QPointF &pos)
{
    globalWaylandIntegration->requestPointerMotionAbsolute(stream, pos);
}

void WaylandIntegration::requestPointerAxisDiscrete(Qt::Orientation axis, qreal delta)
{
    globalWaylandIntegration->requestPointerAxisDiscrete(axis, delta);
}

void WaylandIntegration::requestPointerAxis(qreal x, qreal y)
{
    globalWaylandIntegration->requestPointerAxis(x, y);
}

void WaylandIntegration::requestKeyboardKeycode(int keycode, bool state)
{
    globalWaylandIntegration->requestKeyboardKeycode(keycode, state);
}

void WaylandIntegration::requestKeyboardKeysym(int keysym, bool state)
{
    globalWaylandIntegration->requestKeyboardKeysym(keysym, state);
}

void WaylandIntegration::requestTouchDown(quint32 touchPoint, const QPointF &pos)
{
    globalWaylandIntegration->requestTouchDown(touchPoint, pos);
}

void WaylandIntegration::requestTouchMotion(quint32 touchPoint, const QPointF &pos)
{
    globalWaylandIntegration->requestTouchMotion(touchPoint, pos);
}

void WaylandIntegration::requestTouchUp(quint32 touchPoint)
{
    globalWaylandIntegration->requestTouchUp(touchPoint);
}

void WaylandIntegration::setParentWindow(QWindow *window, const QString &parentWindow)
{
    globalWaylandIntegration->setParentWindow(window, parentWindow);
}

KWayland::Client::PlasmaWindowManagement *WaylandIntegration::plasmaWindowManagement()
{
    return globalWaylandIntegration->plasmaWindowManagement();
}

WaylandIntegration::WaylandIntegration *WaylandIntegration::waylandIntegration()
{
    return globalWaylandIntegration;
}

WaylandIntegration::WaylandIntegrationPrivate::WaylandIntegrationPrivate()
    : WaylandIntegration()
    , m_registryInitialized(false)
    , m_registry(nullptr)
    , m_fakeInput(nullptr)
    , m_screencasting(nullptr)
{
    qDBusRegisterMetaType<Stream>();
    qDBusRegisterMetaType<Streams>();
}

WaylandIntegration::WaylandIntegrationPrivate::~WaylandIntegrationPrivate() = default;

bool WaylandIntegration::WaylandIntegrationPrivate::isStreamingEnabled() const
{
    return !m_streams.isEmpty();
}

bool WaylandIntegration::WaylandIntegrationPrivate::isStreamingAvailable() const
{
    return m_screencasting;
}

void WaylandIntegration::WaylandIntegrationPrivate::acquireStreamingInput(bool acquire)
{
    if (acquire) {
        if (!m_fakeInput) {
            m_fakeInput = std::make_unique<FakeInput>();
            if (!m_fakeInput->isActive()) {
                qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "org_kde_kwin_fake_input protocol is unsupported by the compositor";
            } else {
                m_fakeInput->authenticate(QCoreApplication::applicationName(), QStringLiteral("Remote desktop session input"));
            }
        }
        ++m_streamInput;
    } else {
        Q_ASSERT(m_streamInput > 0);
        --m_streamInput;
        if (m_streamInput == 0) {
            m_fakeInput.reset();
        }
    }
}

WaylandIntegration::Stream WaylandIntegration::WaylandIntegrationPrivate::startStreamingWindow(const QMap<int, QVariant> &win,
                                                                                               Screencasting::CursorMode cursorMode)
{
    auto uuid = win[KWayland::Client::PlasmaWindowModel::Uuid].toString();
    return startStreaming(m_screencasting->createWindowStream(uuid, cursorMode), {{QLatin1String("source_type"), static_cast<uint>(ScreenCastPortal::Window)}});
}

WaylandIntegration::Stream WaylandIntegration::WaylandIntegrationPrivate::startStreamingOutput(QScreen *screen, Screencasting::CursorMode mode)
{
    auto stream = m_screencasting->createOutputStream(screen, mode);
    if (!stream) {
        qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "Cannot stream, output not found" << screen->name();
        auto notification = new KNotification(QStringLiteral("screencastfailure"), KNotification::CloseOnTimeout);
        notification->setTitle(i18n("Failed to start screencasting"));
        notification->setIconName(QStringLiteral("dialog-error"));
        notification->sendEvent();
        return {};
    }

    return startStreaming(stream,
                          {
                              {QLatin1String("size"), screen->size()},
                              {QLatin1String("source_type"), static_cast<uint>(ScreenCastPortal::Monitor)},
                          });
}

WaylandIntegration::Stream WaylandIntegration::WaylandIntegrationPrivate::startStreamingWorkspace(Screencasting::CursorMode mode)
{
    QRect workspace;
    const auto screens = qGuiApp->screens();
    for (QScreen *screen : screens) {
        workspace |= screen->geometry();
    }
    return startStreaming(m_screencasting->createRegionStream(workspace, 1, mode),
                          {
                              {QLatin1String("size"), workspace.size()},
                              {QLatin1String("source_type"), static_cast<uint>(ScreenCastPortal::Monitor)},
                          });
}

WaylandIntegration::Stream WaylandIntegration::WaylandIntegrationPrivate::startStreamingRegion(const QRect region, Screencasting::CursorMode mode)
{
    return startStreaming(m_screencasting->createRegionStream(region, 1, mode),
                          {
                              {QLatin1String("size"), region.size()},
                              {QLatin1String("source_type"), static_cast<uint>(ScreenCastPortal::Monitor)},
                          });
}

WaylandIntegration::Stream
WaylandIntegration::WaylandIntegrationPrivate::startStreamingVirtualOutput(const QString &name, const QSize &size, Screencasting::CursorMode mode)
{
    return startStreaming(m_screencasting->createVirtualOutputStream(name, size, 1, mode),
                          {
                              {QLatin1String("size"), size},
                              {QLatin1String("source_type"), static_cast<uint>(ScreenCastPortal::Virtual)},
                          });
}

WaylandIntegration::Stream WaylandIntegration::WaylandIntegrationPrivate::startStreaming(ScreencastingStream *stream, const QVariantMap &streamOptions)
{
    QEventLoop loop;
    Stream ret;

    connect(stream, &ScreencastingStream::failed, this, [&](const QString &error) {
        qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "failed to start streaming" << stream << error;

        KNotification *notification = new KNotification(QStringLiteral("screencastfailure"), KNotification::CloseOnTimeout);
        notification->setTitle(i18n("Failed to start screencasting"));
        notification->setText(error);
        notification->setIconName(QStringLiteral("dialog-error"));
        notification->sendEvent();

        loop.quit();
    });
    connect(stream, &ScreencastingStream::created, this, [&](uint32_t nodeid) {
        ret.stream = stream;
        ret.nodeId = nodeid;
        ret.map = streamOptions;
        m_streams.append(ret);

        connect(stream, &ScreencastingStream::closed, this, [this, nodeid] {
            stopStreaming(nodeid);
        });
        Q_ASSERT(ret.isValid());

        loop.quit();
    });
    QTimer::singleShot(3000, &loop, &QEventLoop::quit);
    loop.exec();
    return ret;
}

void WaylandIntegration::Stream::close()
{
    stream->deleteLater();
}

void WaylandIntegration::WaylandIntegrationPrivate::stopStreaming(uint32_t nodeid)
{
    for (auto it = m_streams.begin(), itEnd = m_streams.end(); it != itEnd; ++it) {
        if (it->nodeId == nodeid) {
            it->close();
            m_streams.erase(it);
            break;
        }
    }
}

void WaylandIntegration::WaylandIntegrationPrivate::requestPointerButtonPress(quint32 linuxButton)
{
    if (m_fakeInput && m_fakeInput->isActive()) {
        m_fakeInput->button(linuxButton, WL_POINTER_BUTTON_STATE_PRESSED);
    }
}

void WaylandIntegration::WaylandIntegrationPrivate::requestPointerButtonRelease(quint32 linuxButton)
{
    if (m_fakeInput && m_fakeInput->isActive()) {
        m_fakeInput->button(linuxButton, WL_POINTER_BUTTON_STATE_RELEASED);
    }
}

void WaylandIntegration::WaylandIntegrationPrivate::requestPointerMotion(const QSizeF &delta)
{
    if (m_fakeInput && m_fakeInput->isActive()) {
        m_fakeInput->pointer_motion(wl_fixed_from_double(delta.width()), wl_fixed_from_double(delta.height()));
    }
}

void WaylandIntegration::WaylandIntegrationPrivate::requestPointerMotionAbsolute(uint streamNodeId, const QPointF &pos)
{
    if (m_fakeInput && m_fakeInput->isActive()) {
        for (auto stream : std::as_const(m_streams)) {
            if (stream.nodeId == streamNodeId) {
                m_fakeInput->pointer_motion_absolute(wl_fixed_from_double(pos.x() + stream.stream->geometry().x()),
                                                     wl_fixed_from_double(pos.y() + stream.stream->geometry().y()));
                return;
            }
        }
        // If no stream is found, just send it as absolute coordinates relative to the workspace.
        m_fakeInput->pointer_motion_absolute(wl_fixed_from_double(pos.x()), wl_fixed_from_double(pos.y()));
    }
}

void WaylandIntegration::WaylandIntegrationPrivate::requestPointerAxisDiscrete(Qt::Orientation axis, qreal delta)
{
    if (m_fakeInput && m_fakeInput->isActive()) {
        switch (axis) {
        case Qt::Horizontal:
            m_fakeInput->axis(WL_POINTER_AXIS_HORIZONTAL_SCROLL, wl_fixed_from_double(delta));
            break;
        case Qt::Vertical:
            m_fakeInput->axis(WL_POINTER_AXIS_VERTICAL_SCROLL, wl_fixed_from_double(delta));
            break;
        }
    }
}
void WaylandIntegration::WaylandIntegrationPrivate::requestPointerAxis(qreal x, qreal y)
{
    if (m_fakeInput && m_fakeInput->isActive()) {
        if (x != 0) {
            m_fakeInput->axis(WL_POINTER_AXIS_HORIZONTAL_SCROLL, wl_fixed_from_double(x));
        }
        if (y != 0) {
            m_fakeInput->axis(WL_POINTER_AXIS_VERTICAL_SCROLL, wl_fixed_from_double(-y));
        }
    }
}

void WaylandIntegration::WaylandIntegrationPrivate::requestKeyboardKeycode(int keycode, bool state)
{
    if (m_fakeInput && m_fakeInput->isActive()) {
        if (state) {
            m_fakeInput->keyboard_key(keycode, WL_KEYBOARD_KEY_STATE_PRESSED);
        } else {
            m_fakeInput->keyboard_key(keycode, WL_KEYBOARD_KEY_STATE_RELEASED);
        }
    }
}

namespace
{
struct XKBStateDeleter {
    void operator()(struct xkb_state *state) const
    {
        return xkb_state_unref(state);
    }
};
struct XKBKeymapDeleter {
    void operator()(struct xkb_keymap *keymap) const
    {
        return xkb_keymap_unref(keymap);
    }
};
struct XKBContextDeleter {
    void operator()(struct xkb_context *context) const
    {
        return xkb_context_unref(context);
    }
};
using ScopedXKBState = std::unique_ptr<struct xkb_state, XKBStateDeleter>;
using ScopedXKBKeymap = std::unique_ptr<struct xkb_keymap, XKBKeymapDeleter>;
using ScopedXKBContext = std::unique_ptr<struct xkb_context, XKBContextDeleter>;
}
class Xkb : public QtWayland::wl_keyboard
{
public:
    struct Code {
        const uint32_t level;
        const uint32_t code;
    };
    std::optional<Code> keycodeFromKeysym(xkb_keysym_t keysym)
    {
        /* The offset between KEY_* numbering, and keycodes in the XKB evdev
         * dataset. */
        static const uint EVDEV_OFFSET = 8;

        auto layout = xkb_state_serialize_layout(m_state.get(), XKB_STATE_LAYOUT_EFFECTIVE);
        const xkb_keycode_t max = xkb_keymap_max_keycode(m_keymap.get());
        for (xkb_keycode_t keycode = xkb_keymap_min_keycode(m_keymap.get()); keycode < max; keycode++) {
            uint levelCount = xkb_keymap_num_levels_for_key(m_keymap.get(), keycode, layout);
            for (uint currentLevel = 0; currentLevel < levelCount; currentLevel++) {
                const xkb_keysym_t *syms;
                uint num_syms = xkb_keymap_key_get_syms_by_level(m_keymap.get(), keycode, layout, currentLevel, &syms);
                for (uint sym = 0; sym < num_syms; sym++) {
                    if (syms[sym] == keysym) {
                        return Code{currentLevel, keycode - EVDEV_OFFSET};
                    }
                }
            }
        }
        return {};
    }

    static Xkb *self()
    {
        static Xkb self;
        return &self;
    }

private:
    Xkb()
    {
        m_ctx.reset(xkb_context_new(XKB_CONTEXT_NO_FLAGS));
        if (!m_ctx) {
            qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "Failed to create xkb context";
            return;
        }
        m_keymap.reset(xkb_keymap_new_from_names(m_ctx.get(), nullptr, XKB_KEYMAP_COMPILE_NO_FLAGS));
        if (!m_keymap) {
            qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "Failed to create the keymap";
            return;
        }
        m_state.reset(xkb_state_new(m_keymap.get()));
        if (!m_state) {
            qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "Failed to create the xkb state";
            return;
        }

        QPlatformNativeInterface *nativeInterface = qGuiApp->platformNativeInterface();
        auto seat = static_cast<wl_seat *>(nativeInterface->nativeResourceForIntegration("wl_seat"));
        init(wl_seat_get_keyboard(seat));
    }

    void keyboard_keymap(uint32_t format, int32_t fd, uint32_t size) override
    {
        if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
            qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "unknown keymap format:" << format;
            close(fd);
            return;
        }

        char *map_str = static_cast<char *>(mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0));
        if (map_str == MAP_FAILED) {
            close(fd);
            return;
        }

        m_keymap.reset(xkb_keymap_new_from_string(m_ctx.get(), map_str, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS));
        munmap(map_str, size);
        close(fd);

        if (m_keymap)
            m_state.reset(xkb_state_new(m_keymap.get()));
        else
            m_state.reset(nullptr);
    }

    ScopedXKBContext m_ctx;
    ScopedXKBKeymap m_keymap;
    ScopedXKBState m_state;
};

void WaylandIntegration::WaylandIntegrationPrivate::requestKeyboardKeysym(int keysym, bool state)
{
    if (m_fakeInput && m_fakeInput->isActive()) {
        auto keycode = Xkb::self()->keycodeFromKeysym(keysym);
        if (!keycode) {
            qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "Failed to convert keysym into keycode" << keysym;
            return;
        }

        auto sendKey = [this, state](int keycode) {
            if (state) {
                m_fakeInput->keyboard_key(keycode, WL_KEYBOARD_KEY_STATE_PRESSED);
            } else {
                m_fakeInput->keyboard_key(keycode, WL_KEYBOARD_KEY_STATE_RELEASED);
            }
        };
        switch (keycode->level) {
        case 0:
            break;
        case 1:
            sendKey(KEY_LEFTSHIFT);
            break;
        case 2:
            sendKey(KEY_RIGHTALT);
            break;
        default:
            qCWarning(XdgDesktopPortalKdeWaylandIntegration) << "Unsupported key level" << keycode->level;
            break;
        }
        sendKey(keycode->code);
    }
}

void WaylandIntegration::WaylandIntegrationPrivate::requestTouchDown(quint32 touchPoint, const QPointF &pos)
{
    if (m_fakeInput && m_fakeInput->isActive()) {
        m_fakeInput->touch_down(touchPoint, wl_fixed_from_double(pos.x()), wl_fixed_from_double(pos.y()));
    }
}

void WaylandIntegration::WaylandIntegrationPrivate::requestTouchMotion(quint32 touchPoint, const QPointF &pos)
{
    if (m_fakeInput && m_fakeInput->isActive()) {
        m_fakeInput->touch_motion(touchPoint, wl_fixed_from_double(pos.x()), wl_fixed_from_double(pos.y()));
    }
}

void WaylandIntegration::WaylandIntegrationPrivate::requestTouchUp(quint32 touchPoint)
{
    if (m_fakeInput && m_fakeInput->isActive()) {
        m_fakeInput->touch_up(touchPoint);
    }
}

static const char *windowParentHandlePropertyName = "waylandintegration-parentHandle";
void WaylandIntegration::WaylandIntegrationPrivate::setParentWindow(QWindow *window, const QString &parentHandle)
{
    if (!m_xdgImporter) {
        return;
    }

    if (window->isVisible()) {
        auto importedParent = m_xdgImporter->importTopLevel(parentHandle, window);
        auto surface = KWayland::Client::Surface::fromWindow(window);
        importedParent->setParentOf(surface);
    }

    window->setProperty(windowParentHandlePropertyName, parentHandle);
    window->installEventFilter(this);
}

bool WaylandIntegration::WaylandIntegrationPrivate::eventFilter(QObject *watched, QEvent *event)
{
    const bool ret = WaylandIntegration::WaylandIntegration::eventFilter(watched, event);
    QWindow *window = static_cast<QWindow *>(watched);
    if (event->type() == QEvent::Expose && window->isExposed()) {
        const QString parentHandle = window->property(windowParentHandlePropertyName).toString();
        auto importedParent = m_xdgImporter->importTopLevel(parentHandle, window);
        importedParent->setParentOf(KWayland::Client::Surface::fromWindow(window));
    }
    return ret;
}

void WaylandIntegration::WaylandIntegrationPrivate::authenticate()
{
    if (!m_waylandAuthenticationRequested) {
        m_fakeInput->authenticate(QStringLiteral("xdg-desktop-portals-kde"), i18n("Remote desktop"));
        m_waylandAuthenticationRequested = true;
    }
}

KWayland::Client::PlasmaWindowManagement *WaylandIntegration::WaylandIntegrationPrivate::plasmaWindowManagement()
{
    return m_windowManagement;
}

void WaylandIntegration::WaylandIntegrationPrivate::initWayland()
{
    auto connection = KWayland::Client::ConnectionThread::fromApplication(QGuiApplication::instance());

    if (!connection) {
        return;
    }

    m_registry = new KWayland::Client::Registry(this);

    connect(m_registry, &KWayland::Client::Registry::interfaceAnnounced, this, [this](const QByteArray &interfaceName, quint32 name, quint32 version) {
        if (interfaceName != "zkde_screencast_unstable_v1")
            return;
        m_screencasting = new Screencasting(m_registry, name, version, this);
    });
    connect(m_registry, &KWayland::Client::Registry::plasmaWindowManagementAnnounced, this, [this](quint32 name, quint32 version) {
        m_windowManagement = m_registry->createPlasmaWindowManagement(name, version, this);
        Q_EMIT waylandIntegration()->plasmaWindowManagementInitialized();
    });
    connect(m_registry, &KWayland::Client::Registry::importerUnstableV2Announced, this, [this](quint32 name, quint32 version) {
        m_xdgImporter = m_registry->createXdgImporter(name, std::min(version, quint32(1)), this);
    });
    connect(m_registry, &KWayland::Client::Registry::interfacesAnnounced, this, [this] {
        m_registryInitialized = true;
        qCDebug(XdgDesktopPortalKdeWaylandIntegration) << "Registry initialized";
    });

    m_registry->create(connection);
    m_registry->setup();
}
