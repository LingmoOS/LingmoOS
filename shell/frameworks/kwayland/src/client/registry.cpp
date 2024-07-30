/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2018 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "registry.h"
#include "appmenu.h"
#include "blur.h"
#include "compositor.h"
#include "connection_thread.h"
#include "contrast.h"
#include "datadevicemanager.h"
#include "dpms.h"
#include "event_queue.h"
#include "fakeinput.h"
#include "idleinhibit.h"
#include "logging.h"
#include "output.h"
#include "plasmashell.h"
#include "plasmavirtualdesktop.h"
#include "plasmawindowmanagement.h"
#include "pointerconstraints.h"
#include "pointergestures.h"
#include "relativepointer.h"
#include "seat.h"
#include "shadow.h"
#include "shell.h"
#include "shm_pool.h"
#include "slide.h"
#include "subcompositor.h"
#include "textinput_p.h"
#include "wayland_pointer_p.h"
#include "xdgdecoration.h"
#include "xdgforeign_v2.h"
#include "xdgoutput.h"
#include "xdgshell.h"
#include "xdgshell_p.h"
// Qt
#include <QDebug>
// wayland
#include "../compat/wayland-xdg-shell-v5-client-protocol.h"
#include <wayland-appmenu-client-protocol.h>
#include <wayland-blur-client-protocol.h>
#include <wayland-client-protocol.h>
#include <wayland-contrast-client-protocol.h>
#include <wayland-dpms-client-protocol.h>
#include <wayland-fake-input-client-protocol.h>
#include <wayland-idle-inhibit-unstable-v1-client-protocol.h>
#include <wayland-plasma-shell-client-protocol.h>
#include <wayland-plasma-virtual-desktop-client-protocol.h>
#include <wayland-plasma-window-management-client-protocol.h>
#include <wayland-pointer-constraints-unstable-v1-client-protocol.h>
#include <wayland-pointer-gestures-unstable-v1-client-protocol.h>
#include <wayland-relativepointer-unstable-v1-client-protocol.h>
#include <wayland-shadow-client-protocol.h>
#include <wayland-slide-client-protocol.h>
#include <wayland-text-input-v0-client-protocol.h>
#include <wayland-text-input-v2-client-protocol.h>
#include <wayland-xdg-decoration-unstable-v1-client-protocol.h>
#include <wayland-xdg-foreign-unstable-v2-client-protocol.h>
#include <wayland-xdg-output-unstable-v1-client-protocol.h>
#include <wayland-xdg-shell-client-protocol.h>
#include <wayland-xdg-shell-v6-client-protocol.h>

/*****
 * How to add another interface:
 * * define a new enum value in Registry::Interface
 * * define the bind<InterfaceName> method
 * * define the create<InterfaceName> method
 * * define the <interfaceName>Announced signal
 * * define the <interfaceName>Removed signal
 * * add a block to s_interfaces
 * * add the BIND macro for the new bind<InterfaceName>
 * * add the CREATE macro for the new create<InterfaceName>
 * * extend registry unit test to verify that it works
 ****/

namespace KWayland
{
namespace Client
{
namespace
{
struct SuppertedInterfaceData {
    quint32 maxVersion;
    QByteArray name;
    const wl_interface *interface;
    void (Registry::*announcedSignal)(quint32, quint32);
    void (Registry::*removedSignal)(quint32);
};
// clang-format off
static const QMap<Registry::Interface, SuppertedInterfaceData> s_interfaces = {
    {Registry::Interface::Compositor, {
        4,
        QByteArrayLiteral("wl_compositor"),
        &wl_compositor_interface,
        &Registry::compositorAnnounced,
        &Registry::compositorRemoved
    }},
    {Registry::Interface::DataDeviceManager, {
        3,
        QByteArrayLiteral("wl_data_device_manager"),
        &wl_data_device_manager_interface,
        &Registry::dataDeviceManagerAnnounced,
        &Registry::dataDeviceManagerRemoved
    }},
    {Registry::Interface::Output, {
        4,
        QByteArrayLiteral("wl_output"),
        &wl_output_interface,
        &Registry::outputAnnounced,
        &Registry::outputRemoved
    }},
    {Registry::Interface::Shm, {
        1,
        QByteArrayLiteral("wl_shm"),
        &wl_shm_interface,
        &Registry::shmAnnounced,
        &Registry::shmRemoved
    }},
    {Registry::Interface::Seat, {
        5,
        QByteArrayLiteral("wl_seat"),
        &wl_seat_interface,
        &Registry::seatAnnounced,
        &Registry::seatRemoved
    }},
    {Registry::Interface::Shell, {
        1,
        QByteArrayLiteral("wl_shell"),
        &wl_shell_interface,
        &Registry::shellAnnounced,
        &Registry::shellRemoved
    }},
    {Registry::Interface::SubCompositor, {
        1,
        QByteArrayLiteral("wl_subcompositor"),
        &wl_subcompositor_interface,
        &Registry::subCompositorAnnounced,
        &Registry::subCompositorRemoved
    }},
    {Registry::Interface::PlasmaShell, {
        8,
        QByteArrayLiteral("org_kde_plasma_shell"),
        &org_kde_plasma_shell_interface,
        &Registry::plasmaShellAnnounced,
        &Registry::plasmaShellRemoved
    }},
    {Registry::Interface::PlasmaVirtualDesktopManagement, {
        2,
        QByteArrayLiteral("org_kde_plasma_virtual_desktop_management"),
        &org_kde_plasma_virtual_desktop_management_interface,
        &Registry::plasmaVirtualDesktopManagementAnnounced,
        &Registry::plasmaVirtualDesktopManagementRemoved
    }},
    {Registry::Interface::PlasmaWindowManagement, {
        18,
        QByteArrayLiteral("org_kde_plasma_window_management"),
        &org_kde_plasma_window_management_interface,
        &Registry::plasmaWindowManagementAnnounced,
        &Registry::plasmaWindowManagementRemoved
    }},
    {Registry::Interface::FakeInput, {
        4,
        QByteArrayLiteral("org_kde_kwin_fake_input"),
        &org_kde_kwin_fake_input_interface,
        &Registry::fakeInputAnnounced,
        &Registry::fakeInputRemoved
    }},
    {Registry::Interface::Shadow, {
        2,
        QByteArrayLiteral("org_kde_kwin_shadow_manager"),
        &org_kde_kwin_shadow_manager_interface,
        &Registry::shadowAnnounced,
        &Registry::shadowRemoved
    }},
    {Registry::Interface::Blur, {
        1,
        QByteArrayLiteral("org_kde_kwin_blur_manager"),
        &org_kde_kwin_blur_manager_interface,
        &Registry::blurAnnounced,
        &Registry::blurRemoved
    }},
    {Registry::Interface::Contrast, {
        2,
        QByteArrayLiteral("org_kde_kwin_contrast_manager"),
        &org_kde_kwin_contrast_manager_interface,
        &Registry::contrastAnnounced,
        &Registry::contrastRemoved
    }},
    {Registry::Interface::Slide, {
        1,
        QByteArrayLiteral("org_kde_kwin_slide_manager"),
        &org_kde_kwin_slide_manager_interface,
        &Registry::slideAnnounced,
        &Registry::slideRemoved
    }},
    {Registry::Interface::Dpms, {
        1,
        QByteArrayLiteral("org_kde_kwin_dpms_manager"),
        &org_kde_kwin_dpms_manager_interface,
        &Registry::dpmsAnnounced,
        &Registry::dpmsRemoved
    }},
    {Registry::Interface::TextInputManagerUnstableV0, {
        1,
        QByteArrayLiteral("wl_text_input_manager"),
        &wl_text_input_manager_interface,
        &Registry::textInputManagerUnstableV0Announced,
        &Registry::textInputManagerUnstableV0Removed
    }},
    {Registry::Interface::TextInputManagerUnstableV2, {
        1,
        QByteArrayLiteral("zwp_text_input_manager_v2"),
        &zwp_text_input_manager_v2_interface,
        &Registry::textInputManagerUnstableV2Announced,
        &Registry::textInputManagerUnstableV2Removed
    }},
    {Registry::Interface::XdgShellUnstableV5, {
        1,
        QByteArrayLiteral("xdg_shell"),
        &zxdg_shell_v5_interface,
        &Registry::xdgShellUnstableV5Announced,
        &Registry::xdgShellUnstableV5Removed
    }},
    {Registry::Interface::RelativePointerManagerUnstableV1, {
        1,
        QByteArrayLiteral("zwp_relative_pointer_manager_v1"),
        &zwp_relative_pointer_manager_v1_interface,
        &Registry::relativePointerManagerUnstableV1Announced,
        &Registry::relativePointerManagerUnstableV1Removed
    }},
    {Registry::Interface::PointerGesturesUnstableV1, {
        1,
        QByteArrayLiteral("zwp_pointer_gestures_v1"),
        &zwp_pointer_gestures_v1_interface,
        &Registry::pointerGesturesUnstableV1Announced,
        &Registry::pointerGesturesUnstableV1Removed
    }},
    {Registry::Interface::PointerConstraintsUnstableV1, {
        1,
        QByteArrayLiteral("zwp_pointer_constraints_v1"),
        &zwp_pointer_constraints_v1_interface,
        &Registry::pointerConstraintsUnstableV1Announced,
        &Registry::pointerConstraintsUnstableV1Removed
    }},
    {Registry::Interface::XdgExporterUnstableV2, {
        1,
        QByteArrayLiteral("zxdg_exporter_v2"),
        &zxdg_exporter_v2_interface,
        &Registry::exporterUnstableV2Announced,
        &Registry::exporterUnstableV2Removed
    }},
    {Registry::Interface::XdgImporterUnstableV2, {
        1,
        QByteArrayLiteral("zxdg_importer_v2"),
        &zxdg_importer_v2_interface,
        &Registry::importerUnstableV2Announced,
        &Registry::importerUnstableV2Removed
    }},
    {Registry::Interface::XdgShellUnstableV6, {
        1,
        QByteArrayLiteral("zxdg_shell_v6"),
        &zxdg_shell_v6_interface,
        &Registry::xdgShellUnstableV6Announced,
        &Registry::xdgShellUnstableV6Removed
    }},
    {Registry::Interface::IdleInhibitManagerUnstableV1, {
        1,
        QByteArrayLiteral("zwp_idle_inhibit_manager_v1"),
        &zwp_idle_inhibit_manager_v1_interface,
        &Registry::idleInhibitManagerUnstableV1Announced,
        &Registry::idleInhibitManagerUnstableV1Removed
    }},
    {Registry::Interface::AppMenu, {
        1,
        QByteArrayLiteral("org_kde_kwin_appmenu_manager"),
        &org_kde_kwin_appmenu_manager_interface,
        &Registry::appMenuAnnounced,
        &Registry::appMenuRemoved
    }},
    {Registry::Interface::XdgOutputUnstableV1, {
        2,
        QByteArrayLiteral("zxdg_output_manager_v1"),
        &zxdg_output_manager_v1_interface,
        &Registry::xdgOutputAnnounced,
        &Registry::xdgOutputRemoved
    }},
    {Registry::Interface::XdgShellStable, {
        1,
        QByteArrayLiteral("xdg_wm_base"),
        &xdg_wm_base_interface,
        &Registry::xdgShellStableAnnounced,
        &Registry::xdgShellStableRemoved
    }},
    {Registry::Interface::XdgDecorationUnstableV1, {
        1,
        QByteArrayLiteral("zxdg_decoration_manager_v1"),
        &zxdg_decoration_manager_v1_interface,
        &Registry::xdgDecorationAnnounced,
        &Registry::xdgDecorationRemoved
    }},
    {Registry::Interface::PlasmaActivationFeedback, {
        1,
        QByteArrayLiteral("org_kde_plasma_activation_feedback"),
        &org_kde_plasma_activation_feedback_interface,
        &Registry::plasmaActivationFeedbackAnnounced,
        &Registry::plasmaActivationFeedbackRemoved
    }},
};
// clang-format on

static quint32 maxVersion(const Registry::Interface &interface)
{
    auto it = s_interfaces.find(interface);
    if (it != s_interfaces.end()) {
        return it.value().maxVersion;
    }
    return 0;
}
}

class Q_DECL_HIDDEN Registry::Private
{
public:
    Private(Registry *q);
    void setup();
    bool hasInterface(Interface interface) const;
    AnnouncedInterface interface(Interface interface) const;
    QList<AnnouncedInterface> interfaces(Interface interface) const;
    Interface interfaceForName(quint32 name) const;
    template<typename T>
    T *bind(Interface interface, uint32_t name, uint32_t version) const;
    template<class T, typename WL>
    T *create(quint32 name, quint32 version, QObject *parent, WL *(Registry::*bindMethod)(uint32_t, uint32_t) const);

    WaylandPointer<wl_registry, wl_registry_destroy> registry;
    static const struct wl_callback_listener s_callbackListener;
    WaylandPointer<wl_callback, wl_callback_destroy> callback;
    EventQueue *queue = nullptr;

private:
    void handleAnnounce(uint32_t name, const char *interface, uint32_t version);
    void handleRemove(uint32_t name);
    void handleGlobalSync();
    static void globalAnnounce(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version);
    static void globalRemove(void *data, struct wl_registry *registry, uint32_t name);
    static void globalSync(void *data, struct wl_callback *callback, uint32_t serial);

    Registry *q;
    struct InterfaceData {
        Interface interface;
        uint32_t name;
        uint32_t version;
    };
    QList<InterfaceData> m_interfaces;
    static const struct wl_registry_listener s_registryListener;
};

Registry::Private::Private(Registry *q)
    : q(q)
{
}

void Registry::Private::setup()
{
    wl_registry_add_listener(registry, &s_registryListener, this);
    wl_callback_add_listener(callback, &s_callbackListener, this);
}

Registry::Registry(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

Registry::~Registry()
{
    release();
}

void Registry::release()
{
    d->registry.release();
    d->callback.release();
}

void Registry::destroy()
{
    Q_EMIT registryDestroyed();
    d->registry.destroy();
    d->callback.destroy();
}

void Registry::create(wl_display *display)
{
    Q_ASSERT(display);
    Q_ASSERT(!isValid());
    d->registry.setup(wl_display_get_registry(display));
    d->callback.setup(wl_display_sync(display));
    if (d->queue) {
        d->queue->addProxy(d->registry);
        d->queue->addProxy(d->callback);
    }
}

void Registry::create(ConnectionThread *connection)
{
    create(connection->display());
    connect(connection, &ConnectionThread::connectionDied, this, &Registry::destroy);
}

void Registry::setup()
{
    Q_ASSERT(isValid());
    d->setup();
}

void Registry::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
    if (!queue) {
        return;
    }
    if (d->registry) {
        d->queue->addProxy(d->registry);
    }
    if (d->callback) {
        d->queue->addProxy(d->callback);
    }
}

EventQueue *Registry::eventQueue()
{
    return d->queue;
}

#ifndef K_DOXYGEN
const struct wl_registry_listener Registry::Private::s_registryListener = {globalAnnounce, globalRemove};

const struct wl_callback_listener Registry::Private::s_callbackListener = {globalSync};
#endif

void Registry::Private::globalAnnounce(void *data, wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
    auto r = reinterpret_cast<Registry::Private *>(data);
    Q_ASSERT(registry == r->registry);
    r->handleAnnounce(name, interface, version);
}

void Registry::Private::globalRemove(void *data, wl_registry *registry, uint32_t name)
{
    auto r = reinterpret_cast<Registry::Private *>(data);
    Q_ASSERT(registry == r->registry);
    r->handleRemove(name);
}

void Registry::Private::globalSync(void *data, wl_callback *callback, uint32_t serial)
{
    Q_UNUSED(serial)
    auto r = reinterpret_cast<Registry::Private *>(data);
    Q_ASSERT(r->callback == callback);
    r->handleGlobalSync();
    r->callback.release();
}

void Registry::Private::handleGlobalSync()
{
    Q_EMIT q->interfacesAnnounced();
}

namespace
{
static Registry::Interface nameToInterface(const char *interface)
{
    for (auto it = s_interfaces.constBegin(); it != s_interfaces.constEnd(); ++it) {
        if (qstrcmp(interface, it.value().name) == 0) {
            return it.key();
        }
    }
    return Registry::Interface::Unknown;
}
}

void Registry::Private::handleAnnounce(uint32_t name, const char *interface, uint32_t version)
{
    Interface i = nameToInterface(interface);
    Q_EMIT q->interfaceAnnounced(QByteArray(interface), name, version);
    if (i == Interface::Unknown) {
        qCDebug(KWAYLAND_CLIENT) << "Unknown interface announced: " << interface << "/" << name << "/" << version;
        return;
    }
    qCDebug(KWAYLAND_CLIENT) << "Wayland Interface: " << interface << "/" << name << "/" << version;
    m_interfaces.append({i, name, version});
    auto it = s_interfaces.constFind(i);
    if (it != s_interfaces.end()) {
        Q_EMIT(q->*it.value().announcedSignal)(name, version);
    }
}

void Registry::Private::handleRemove(uint32_t name)
{
    auto it = std::find_if(m_interfaces.begin(), m_interfaces.end(), [name](const InterfaceData &data) {
        return data.name == name;
    });
    if (it != m_interfaces.end()) {
        InterfaceData data = *(it);
        m_interfaces.erase(it);
        auto sit = s_interfaces.find(data.interface);
        if (sit != s_interfaces.end()) {
            Q_EMIT(q->*sit.value().removedSignal)(data.name);
        }
    }
    Q_EMIT q->interfaceRemoved(name);
}

bool Registry::Private::hasInterface(Registry::Interface interface) const
{
    auto it = std::find_if(m_interfaces.constBegin(), m_interfaces.constEnd(), [interface](const InterfaceData &data) {
        return data.interface == interface;
    });
    return it != m_interfaces.constEnd();
}

QList<Registry::AnnouncedInterface> Registry::Private::interfaces(Interface interface) const
{
    QList<Registry::AnnouncedInterface> retVal;
    for (auto it = m_interfaces.constBegin(); it != m_interfaces.constEnd(); ++it) {
        const auto &data = *it;
        if (data.interface == interface) {
            retVal << AnnouncedInterface{data.name, data.version};
        }
    }
    return retVal;
}

Registry::AnnouncedInterface Registry::Private::interface(Interface interface) const
{
    const auto all = interfaces(interface);
    if (!all.isEmpty()) {
        return all.last();
    }
    return AnnouncedInterface{0, 0};
}

Registry::Interface Registry::Private::interfaceForName(quint32 name) const
{
    auto it = std::find_if(m_interfaces.constBegin(), m_interfaces.constEnd(), [name](const InterfaceData &data) {
        return data.name == name;
    });
    if (it == m_interfaces.constEnd()) {
        return Interface::Unknown;
    }
    return (*it).interface;
}

bool Registry::hasInterface(Registry::Interface interface) const
{
    return d->hasInterface(interface);
}

QList<Registry::AnnouncedInterface> Registry::interfaces(Interface interface) const
{
    return d->interfaces(interface);
}

Registry::AnnouncedInterface Registry::interface(Interface interface) const
{
    return d->interface(interface);
}

// clang-format off
#define BIND2(__NAME__, __INAME__, __WL__) \
__WL__ *Registry::bind##__NAME__(uint32_t name, uint32_t version) const \
{ \
    return d->bind<__WL__>(Interface::__INAME__, name, qMin(maxVersion(Interface::__INAME__), version)); \
}

#define BIND(__NAME__, __WL__) BIND2(__NAME__, __NAME__, __WL__)
// clang-format on

BIND(Compositor, wl_compositor)
BIND(Output, wl_output)
BIND(Seat, wl_seat)
BIND(Shell, wl_shell)
BIND(Shm, wl_shm)
BIND(SubCompositor, wl_subcompositor)
BIND(DataDeviceManager, wl_data_device_manager)
BIND(PlasmaShell, org_kde_plasma_shell)
BIND(PlasmaActivationFeedback, org_kde_plasma_activation_feedback)
BIND(PlasmaVirtualDesktopManagement, org_kde_plasma_virtual_desktop_management)
BIND(PlasmaWindowManagement, org_kde_plasma_window_management)
BIND(FakeInput, org_kde_kwin_fake_input)
BIND(TextInputManagerUnstableV0, wl_text_input_manager)
BIND(TextInputManagerUnstableV2, zwp_text_input_manager_v2)
BIND(XdgShellUnstableV5, xdg_shell)
BIND(XdgShellUnstableV6, zxdg_shell_v6)
BIND(XdgShellStable, xdg_wm_base)
BIND(RelativePointerManagerUnstableV1, zwp_relative_pointer_manager_v1)
BIND(PointerGesturesUnstableV1, zwp_pointer_gestures_v1)
BIND(PointerConstraintsUnstableV1, zwp_pointer_constraints_v1)
BIND(XdgExporterUnstableV2, zxdg_exporter_v2)
BIND(XdgImporterUnstableV2, zxdg_importer_v2)
BIND(IdleInhibitManagerUnstableV1, zwp_idle_inhibit_manager_v1)
BIND2(ShadowManager, Shadow, org_kde_kwin_shadow_manager)
BIND2(BlurManager, Blur, org_kde_kwin_blur_manager)
BIND2(ContrastManager, Contrast, org_kde_kwin_contrast_manager)
BIND2(SlideManager, Slide, org_kde_kwin_slide_manager)
BIND2(DpmsManager, Dpms, org_kde_kwin_dpms_manager)
BIND2(AppMenuManager, AppMenu, org_kde_kwin_appmenu_manager)
BIND(XdgOutputUnstableV1, zxdg_output_manager_v1)
BIND(XdgDecorationUnstableV1, zxdg_decoration_manager_v1)

#undef BIND
#undef BIND2

template<class T, typename WL>
T *Registry::Private::create(quint32 name, quint32 version, QObject *parent, WL *(Registry::*bindMethod)(uint32_t, uint32_t) const)
{
    T *t = new T(parent);
    t->setEventQueue(queue);
    t->setup((q->*bindMethod)(name, version));
    QObject::connect(q, &Registry::interfaceRemoved, t, [t, name](quint32 removed) {
        if (name == removed) {
            Q_EMIT t->removed();
        }
    });
    QObject::connect(q, &Registry::registryDestroyed, t, &T::destroy);
    return t;
}

// clang-format off
#define CREATE2(__NAME__, __BINDNAME__) \
__NAME__ *Registry::create##__NAME__(quint32 name, quint32 version, QObject *parent) \
{ \
    return d->create<__NAME__>(name, version, parent, &Registry::bind##__BINDNAME__); \
}

#define CREATE(__NAME__) CREATE2(__NAME__, __NAME__)
// clang-format on

CREATE(Compositor)
CREATE(Seat)
CREATE(Shell)
CREATE(SubCompositor)
CREATE(Output)
CREATE(DataDeviceManager)
CREATE(PlasmaShell)
CREATE(PlasmaActivationFeedback)
CREATE(PlasmaVirtualDesktopManagement)
CREATE(PlasmaWindowManagement)
CREATE(FakeInput)
CREATE(ShadowManager)
CREATE(BlurManager)
CREATE(ContrastManager)
CREATE(SlideManager)
CREATE(DpmsManager)
CREATE2(ShmPool, Shm)
CREATE(AppMenuManager)

#undef CREATE
#undef CREATE2

XdgExporter *Registry::createXdgExporter(quint32 name, quint32 version, QObject *parent)
{
    // only V1 supported for now
    return d->create<XdgExporterUnstableV2>(name, version, parent, &Registry::bindXdgExporterUnstableV2);
}

XdgImporter *Registry::createXdgImporter(quint32 name, quint32 version, QObject *parent)
{
    // only V1 supported for now
    return d->create<XdgImporterUnstableV2>(name, version, parent, &Registry::bindXdgImporterUnstableV2);
}

TextInputManager *Registry::createTextInputManager(quint32 name, quint32 version, QObject *parent)
{
    switch (d->interfaceForName(name)) {
    case Interface::TextInputManagerUnstableV0:
        return d->create<TextInputManagerUnstableV0>(name, version, parent, &Registry::bindTextInputManagerUnstableV0);
    case Interface::TextInputManagerUnstableV2:
        return d->create<TextInputManagerUnstableV2>(name, version, parent, &Registry::bindTextInputManagerUnstableV2);
    default:
        return nullptr;
    }
}

XdgShell *Registry::createXdgShell(quint32 name, quint32 version, QObject *parent)
{
    switch (d->interfaceForName(name)) {
    case Interface::XdgShellUnstableV5:
        return d->create<XdgShellUnstableV5>(name, version, parent, &Registry::bindXdgShellUnstableV5);
    case Interface::XdgShellUnstableV6:
        return d->create<XdgShellUnstableV6>(name, version, parent, &Registry::bindXdgShellUnstableV6);
    case Interface::XdgShellStable:
        return d->create<XdgShellStable>(name, version, parent, &Registry::bindXdgShellStable);
    default:
        return nullptr;
    }
}

RelativePointerManager *Registry::createRelativePointerManager(quint32 name, quint32 version, QObject *parent)
{
    switch (d->interfaceForName(name)) {
    case Interface::RelativePointerManagerUnstableV1:
        return d->create<RelativePointerManager>(name, version, parent, &Registry::bindRelativePointerManagerUnstableV1);
    default:
        return nullptr;
    }
}

PointerGestures *Registry::createPointerGestures(quint32 name, quint32 version, QObject *parent)
{
    switch (d->interfaceForName(name)) {
    case Interface::PointerGesturesUnstableV1:
        return d->create<PointerGestures>(name, version, parent, &Registry::bindPointerGesturesUnstableV1);
    default:
        return nullptr;
    }
}

PointerConstraints *Registry::createPointerConstraints(quint32 name, quint32 version, QObject *parent)
{
    switch (d->interfaceForName(name)) {
    case Interface::PointerConstraintsUnstableV1:
        return d->create<PointerConstraints>(name, version, parent, &Registry::bindPointerConstraintsUnstableV1);
    default:
        return nullptr;
    }
}

IdleInhibitManager *Registry::createIdleInhibitManager(quint32 name, quint32 version, QObject *parent)
{
    switch (d->interfaceForName(name)) {
    case Interface::IdleInhibitManagerUnstableV1:
        return d->create<IdleInhibitManager>(name, version, parent, &Registry::bindIdleInhibitManagerUnstableV1);
    default:
        return nullptr;
    }
}

XdgOutputManager *Registry::createXdgOutputManager(quint32 name, quint32 version, QObject *parent)
{
    switch (d->interfaceForName(name)) {
    case Interface::XdgOutputUnstableV1:
        return d->create<XdgOutputManager>(name, version, parent, &Registry::bindXdgOutputUnstableV1);
    default:
        return nullptr;
    }
}

XdgDecorationManager *Registry::createXdgDecorationManager(quint32 name, quint32 version, QObject *parent)
{
    switch (d->interfaceForName(name)) {
    case Interface::XdgDecorationUnstableV1:
        return d->create<XdgDecorationManager>(name, version, parent, &Registry::bindXdgDecorationUnstableV1);
    default:
        return nullptr;
    }
}

namespace
{
static const wl_interface *wlInterface(Registry::Interface interface)
{
    auto it = s_interfaces.find(interface);
    if (it != s_interfaces.end()) {
        return it.value().interface;
    }
    return nullptr;
}
}

template<typename T>
T *Registry::Private::bind(Registry::Interface interface, uint32_t name, uint32_t version) const
{
    auto it = std::find_if(m_interfaces.constBegin(), m_interfaces.constEnd(), [=](const InterfaceData &data) {
        return data.interface == interface && data.name == name && data.version >= version;
    });
    if (it == m_interfaces.constEnd()) {
        qCDebug(KWAYLAND_CLIENT) << "Don't have interface " << int(interface) << "with name " << name << "and minimum version" << version;
        return nullptr;
    }
    auto t = reinterpret_cast<T *>(wl_registry_bind(registry, name, wlInterface(interface), version));
    if (queue) {
        queue->addProxy(t);
    }
    return t;
}

bool Registry::isValid() const
{
    return d->registry.isValid();
}

wl_registry *Registry::registry()
{
    return d->registry;
}

Registry::operator wl_registry *() const
{
    return d->registry;
}

Registry::operator wl_registry *()
{
    return d->registry;
}

}
}

#include "moc_registry.cpp"
