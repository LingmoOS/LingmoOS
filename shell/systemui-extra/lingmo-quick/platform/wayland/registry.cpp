/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2018 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2024 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "registry.h"
#include "wayland-pointer_p.h"
// Qt
#include <QDebug>
// wayland
#include <wayland-util.h>
#include <wayland-lingmo-window-management-client-protocol.h>
#include <wayland-lingmo-shell-client-protocol.h>
#include <wayland-client-protocol.h>
#include <wayland-plasma-virtual-desktop-client-protocol.h>

#include <windowmanager/lingmowindowmanagement.h>
#include <windowmanager/lingmoshell.h>
#include <KWayland/Client/plasmavirtualdesktop.h>

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
namespace LingmoUIQuick::WaylandClient
{
namespace
{
struct SupportedInterfaceData {
    quint32 maxVersion;
    QByteArray name;
    const wl_interface *interface;
    void (Registry::*announcedSignal)(quint32, quint32);
    void (Registry::*removedSignal)(quint32);
};
// clang-format off
static const QMap<Registry::Interface, SupportedInterfaceData> s_interfaces = {
        {Registry::Interface::LingmoUIShell, {
            2,
            QByteArrayLiteral("lingmo_shell"),
            &lingmo_shell_interface,
            &Registry::lingmoShellAnnounced,
            &Registry::lingmoShellRemoved
        }},
        {Registry::Interface::LingmoUIWindowManagement, {
            1,
            QByteArrayLiteral("lingmo_window_management"),
            &lingmo_window_management_interface,
            &Registry::lingmoWindowManagementAnnounced,
            &Registry::lingmoWindowManagementRemoved
        }},
        {Registry::Interface::PlasmaVirtualDesktopManagement, {
            2,
            QByteArrayLiteral("org_kde_plasma_virtual_desktop_management"),
            &org_kde_plasma_virtual_desktop_management_interface,
            &Registry::plasmaVirtualDesktopManagementAnnounced,
            &Registry::plasmaVirtualDesktopManagementRemoved
        }},
};

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
    explicit Private(Registry *q);
    void setup();
    bool hasInterface(Interface interface) const;
    AnnouncedInterface interface(Interface interface) const;
    QVector<AnnouncedInterface> interfaces(Interface interface) const;
    Interface interfaceForName(quint32 name) const;
    template<typename T>
    T *bind(Interface interface, uint32_t name, uint32_t version) const;
    template<class T, typename WL>
    T *create(quint32 name, quint32 version, QObject *parent, WL *(Registry::*bindMethod)(uint32_t, uint32_t) const);

    LingmoUIWayland::WaylandClient::WaylandPointer<wl_registry, wl_registry_destroy> registry;
    static const struct wl_callback_listener s_callbackListener;
    LingmoUIWayland::WaylandClient::WaylandPointer<wl_callback, wl_callback_destroy> callback;
    KWayland::Client::EventQueue *queue = nullptr;

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

KWayland::Client::EventQueue *Registry::eventQueue()
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
        qDebug() << "Unknown interface announced: " << interface << "/" << name << "/" << version;
        return;
    }
    qDebug() << "Wayland Interface: " << interface << "/" << name << "/" << version;
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

QVector<Registry::AnnouncedInterface> Registry::Private::interfaces(Interface interface) const
{
    QVector<Registry::AnnouncedInterface> retVal;
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

QVector<Registry::AnnouncedInterface> Registry::interfaces(Interface interface) const
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

BIND(LingmoUIShell, lingmo_shell)
BIND(LingmoUIWindowManagement, lingmo_window_management)
BIND(PlasmaVirtualDesktopManagement, org_kde_plasma_virtual_desktop_management)

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

#define CREATE2(__NAME__, __BINDNAME__) \
__NAME__ *Registry::create##__NAME__(quint32 name, quint32 version, QObject *parent) \
{ \
    return d->create<__NAME__>(name, version, parent, &Registry::bind##__BINDNAME__); \
}

#define CREATE(__NAME__) CREATE2(__NAME__, __NAME__)

CREATE(LingmoUIShell)
CREATE(LingmoUIWindowManagement)
CREATE(PlasmaVirtualDesktopManagement)

#undef CREATE
#undef CREATE2

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
        qDebug() << "Don't have interface " << int(interface) << "with name " << name << "and minimum version" << version;
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

