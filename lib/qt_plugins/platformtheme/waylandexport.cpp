#include "waylandexport.h"

#include <QWindow>

#ifdef LINGMO_PLATFORMTHEME_WAYLAND

#include <QGuiApplication>
#include <qguiapplication_platform.h>
#include <qpa/qplatformwindow_p.h>

#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <xdg-foreign-unstable-v2-client-protocol.h>

#include <cstring>

static void registryGlobal(void *data, wl_registry *registry, uint32_t name, const char *interface,
                           uint32_t)
{
    auto *exporter = static_cast<zxdg_exporter_v2 **>(data);
    if (std::strcmp(interface, zxdg_exporter_v2_interface.name) == 0) {
        *exporter = static_cast<zxdg_exporter_v2 *>(
                wl_registry_bind(registry, name, &zxdg_exporter_v2_interface, 1));
    }
}

static wl_display *boundDisplay = nullptr;
static zxdg_exporter_v2 *boundExporter = nullptr;

// Post routines run in ~QGuiApplication while the display is still connected. A
// later QGuiApplication can get the freed display's address back, so reset here.
static void releaseSharedExporter()
{
    if (boundExporter)
        zxdg_exporter_v2_destroy(boundExporter);
    boundExporter = nullptr;
    boundDisplay = nullptr;
}

// wl_registry has no destructor request, so the compositor keeps every registry
// until the client disconnects: bind the exporter once per connection and keep it.
static zxdg_exporter_v2 *sharedExporter(wl_display *display)
{
    if (display == boundDisplay)
        return boundExporter;

    // Qt empties its post routine list as it runs it; a reconnect keeps ours registered.
    if (!boundDisplay)
        qAddPostRoutine(releaseSharedExporter);
    boundDisplay = display;
    boundExporter = nullptr;

    // A queue of our own, so the roundtrip below never dispatches Qt's events.
    wl_event_queue *queue = wl_display_create_queue(display);
    auto *wrapper = static_cast<wl_display *>(wl_proxy_create_wrapper(display));
    wl_proxy_set_queue(reinterpret_cast<wl_proxy *>(wrapper), queue);
    wl_registry *registry = wl_display_get_registry(wrapper);
    wl_proxy_wrapper_destroy(wrapper);

    static const wl_registry_listener registryListener = {registryGlobal,
                                                          [](void *, wl_registry *, uint32_t) { }};
    wl_registry_add_listener(registry, &registryListener, &boundExporter);
    wl_display_roundtrip_queue(display, queue);

    // Off the queue that is about to go, onto the display's default one.
    if (boundExporter)
        wl_proxy_set_queue(reinterpret_cast<wl_proxy *>(boundExporter), nullptr);
    wl_registry_destroy(registry);
    wl_event_queue_destroy(queue);
    return boundExporter;
}

QString WaylandExport::exportWindow(QWindow *window)
{
    release();
    if (!window)
        return QString();

    auto *application = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    auto *native = window->nativeInterface<QNativeInterface::Private::QWaylandWindow>();
    if (!application || !native || !native->surface())
        return QString();

    m_display = application->display();
    zxdg_exporter_v2 *exporter = sharedExporter(m_display);
    if (!exporter)
        return QString();

    // A queue of our own, so the roundtrip below never dispatches Qt's events.
    wl_event_queue *queue = wl_display_create_queue(m_display);
    auto *wrapper = static_cast<zxdg_exporter_v2 *>(wl_proxy_create_wrapper(exporter));
    wl_proxy_set_queue(reinterpret_cast<wl_proxy *>(wrapper), queue);
    m_exported = zxdg_exporter_v2_export_toplevel(wrapper, native->surface());
    wl_proxy_wrapper_destroy(wrapper);

    m_handle.clear();
    static const zxdg_exported_v2_listener exportedListener = {
            [](void *data, zxdg_exported_v2 *, const char *handle) {
                *static_cast<QString *>(data) = QString::fromUtf8(handle);
            }};
    zxdg_exported_v2_add_listener(m_exported, &exportedListener, &m_handle);
    wl_display_roundtrip_queue(m_display, queue);
    // Off the queue that is about to go, onto the display's default one.
    wl_proxy_set_queue(reinterpret_cast<wl_proxy *>(m_exported), nullptr);

    wl_event_queue_destroy(queue);
    if (m_handle.isEmpty()) {
        release();
        return QString();
    }
    return QStringLiteral("wayland:") + m_handle;
}

void WaylandExport::release()
{
    if (!m_exported)
        return;
    zxdg_exported_v2_destroy(m_exported);
    wl_display_flush(m_display);
    m_exported = nullptr;
    m_handle.clear();
}

WaylandExport::~WaylandExport()
{
    release();
}

#else

QString WaylandExport::exportWindow(QWindow *)
{
    return QString();
}

WaylandExport::~WaylandExport() = default;

#endif
