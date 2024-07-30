/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "plasmawindowmanagement.h"
#include "event_queue.h"
#include "output.h"
#include "plasmavirtualdesktop.h"
#include "plasmawindowmodel.h"
#include "surface.h"
#include "wayland_pointer_p.h"
// Wayland
#include <wayland-plasma-window-management-client-protocol.h>

#include <QFutureWatcher>
#include <QTimer>
#include <QtConcurrentRun>
#include <qplatformdefs.h>

#include <cerrno>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN PlasmaWindowManagement::Private : public QObject
{
    Q_OBJECT
public:
    Private(PlasmaWindowManagement *q);
    WaylandPointer<org_kde_plasma_window_management, org_kde_plasma_window_management_destroy> wm;
    EventQueue *queue = nullptr;
    bool showingDesktop = false;
    QList<PlasmaWindow *> windows;
    PlasmaWindow *activeWindow = nullptr;
    QList<quint32> stackingOrder;
    QList<QByteArray> stackingOrderUuids;

    void setup(org_kde_plasma_window_management *wm);

private:
    static void showDesktopCallback(void *data, org_kde_plasma_window_management *org_kde_plasma_window_management, uint32_t state);
    static void windowCallback(void *data, org_kde_plasma_window_management *org_kde_plasma_window_management, uint32_t id);
    static void windowWithUuidCallback(void *data, org_kde_plasma_window_management *org_kde_plasma_window_management, uint32_t id, const char *uuid);
    static void stackingOrderCallback(void *data, org_kde_plasma_window_management *org_kde_plasma_window_management, wl_array *ids);
    static void stackingOrderUuidsCallback(void *data, org_kde_plasma_window_management *org_kde_plasma_window_management, const char *uuids);
    static void stackingOrder2Callback(void *data, org_kde_plasma_window_management *interface);
    void setShowDesktop(bool set);
    void windowCreated(org_kde_plasma_window *id, quint32 internalId, const char *uuid);
    void setStackingOrder(const QList<quint32> &ids);
    void setStackingOrder(const QList<QByteArray> &uuids);

    static void stackingOrderWindow(void *data, struct org_kde_plasma_stacking_order *org_kde_plasma_stacking_order, const char *uuid);
    static void stackingOrderDone(void *data, struct org_kde_plasma_stacking_order *org_kde_plasma_stacking_order);

    static struct org_kde_plasma_window_management_listener s_listener;
    static const org_kde_plasma_stacking_order_listener s_stackingOrderListener;
    PlasmaWindowManagement *q;
};

class Q_DECL_HIDDEN PlasmaWindow::Private
{
public:
    Private(org_kde_plasma_window *window, quint32 internalId, const char *uuid, PlasmaWindow *q);
    WaylandPointer<org_kde_plasma_window, org_kde_plasma_window_destroy> window;
    quint32 internalId; ///< @deprecated
    QByteArray uuid;
    QString title;
    QString appId;
    quint32 desktop = 0;
    bool active = false;
    bool minimized = false;
    bool maximized = false;
    bool fullscreen = false;
    bool keepAbove = false;
    bool keepBelow = false;
    bool onAllDesktops = false;
    bool demandsAttention = false;
    bool closeable = false;
    bool minimizeable = false;
    bool maximizeable = false;
    bool fullscreenable = false;
    bool skipTaskbar = false;
    bool skipSwitcher = false;
    bool shadeable = false;
    bool shaded = false;
    bool movable = false;
    bool resizable = false;
    bool virtualDesktopChangeable = false;
    QIcon icon;
    PlasmaWindowManagement *wm = nullptr;
    bool unmapped = false;
    QPointer<PlasmaWindow> parentWindow;
    QMetaObject::Connection parentWindowUnmappedConnection;
    QStringList plasmaVirtualDesktops;
    QStringList plasmaActivities;
    QRect geometry;
    quint32 pid = 0;
    QString resourceName;
    QString applicationMenuServiceName;
    QString applicationMenuObjectPath;
    QRect clientGeometry;

private:
    static void titleChangedCallback(void *data, org_kde_plasma_window *window, const char *title);
    static void appIdChangedCallback(void *data, org_kde_plasma_window *window, const char *app_id);
    static void pidChangedCallback(void *data, org_kde_plasma_window *window, uint32_t pid);
    static void resourceNameChangedCallback(void *data, org_kde_plasma_window *window, const char *resourceName);
    static void stateChangedCallback(void *data, org_kde_plasma_window *window, uint32_t state);
    static void virtualDesktopChangedCallback(void *data, org_kde_plasma_window *window, int32_t number);
    static void themedIconNameChangedCallback(void *data, org_kde_plasma_window *window, const char *name);
    static void unmappedCallback(void *data, org_kde_plasma_window *window);
    static void initialStateCallback(void *data, org_kde_plasma_window *window);
    static void parentWindowCallback(void *data, org_kde_plasma_window *window, org_kde_plasma_window *parent);
    static void windowGeometryCallback(void *data, org_kde_plasma_window *window, int32_t x, int32_t y, uint32_t width, uint32_t height);
    static void iconChangedCallback(void *data, org_kde_plasma_window *org_kde_plasma_window);
    static void virtualDesktopEnteredCallback(void *data, org_kde_plasma_window *org_kde_plasma_window, const char *id);
    static void virtualDesktopLeftCallback(void *data, org_kde_plasma_window *org_kde_plasma_window, const char *id);
    static void appmenuChangedCallback(void *data, org_kde_plasma_window *org_kde_plasma_window, const char *service_name, const char *object_path);
    static void activityEnteredCallback(void *data, org_kde_plasma_window *org_kde_plasma_window, const char *id);
    static void activityLeftCallback(void *data, org_kde_plasma_window *org_kde_plasma_window, const char *id);
    static void clientGeometryCallback(void *data, org_kde_plasma_window *window, int32_t x, int32_t y, uint32_t width, uint32_t height);
    void setActive(bool set);
    void setMinimized(bool set);
    void setMaximized(bool set);
    void setFullscreen(bool set);
    void setKeepAbove(bool set);
    void setKeepBelow(bool set);
    void setOnAllDesktops(bool set);
    void setDemandsAttention(bool set);
    void setCloseable(bool set);
    void setMinimizeable(bool set);
    void setMaximizeable(bool set);
    void setFullscreenable(bool set);
    void setSkipTaskbar(bool skip);
    void setSkipSwitcher(bool skip);
    void setShadeable(bool set);
    void setShaded(bool set);
    void setMovable(bool set);
    void setResizable(bool set);
    void setVirtualDesktopChangeable(bool set);
    void setParentWindow(PlasmaWindow *parentWindow);
    void setPid(const quint32 pid);

    static Private *cast(void *data)
    {
        return reinterpret_cast<Private *>(data);
    }

    PlasmaWindow *q;

    static struct org_kde_plasma_window_listener s_listener;
};

struct StackingOrderData {
    QPointer<PlasmaWindowManagement::Private> wm;
    QList<QByteArray> list;
};

PlasmaWindowManagement::Private::Private(PlasmaWindowManagement *q)
    : q(q)
{
}

org_kde_plasma_window_management_listener PlasmaWindowManagement::Private::s_listener = {
    showDesktopCallback,
    windowCallback,
    stackingOrderCallback,
    stackingOrderUuidsCallback,
    windowWithUuidCallback,
    stackingOrder2Callback,
};

void PlasmaWindowManagement::Private::setup(org_kde_plasma_window_management *windowManagement)
{
    Q_ASSERT(!wm);
    Q_ASSERT(windowManagement);
    wm.setup(windowManagement);
    org_kde_plasma_window_management_add_listener(windowManagement, &s_listener, this);

    if (org_kde_plasma_window_management_get_version(windowManagement) >= ORG_KDE_PLASMA_WINDOW_MANAGEMENT_GET_STACKING_ORDER_SINCE_VERSION) {
        auto object = org_kde_plasma_window_management_get_stacking_order(wm);
        org_kde_plasma_stacking_order_add_listener(object,
                                                   &s_stackingOrderListener,
                                                   new StackingOrderData{
                                                       .wm = this,
                                                       .list = {},
                                                   });
    }
}

void PlasmaWindowManagement::Private::showDesktopCallback(void *data, org_kde_plasma_window_management *org_kde_plasma_window_management, uint32_t state)
{
    auto wm = reinterpret_cast<PlasmaWindowManagement::Private *>(data);
    Q_ASSERT(wm->wm == org_kde_plasma_window_management);
    switch (state) {
    case ORG_KDE_PLASMA_WINDOW_MANAGEMENT_SHOW_DESKTOP_ENABLED:
        wm->setShowDesktop(true);
        break;
    case ORG_KDE_PLASMA_WINDOW_MANAGEMENT_SHOW_DESKTOP_DISABLED:
        wm->setShowDesktop(false);
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
}

void PlasmaWindowManagement::Private::setShowDesktop(bool set)
{
    if (showingDesktop == set) {
        return;
    }
    showingDesktop = set;
    Q_EMIT q->showingDesktopChanged(showingDesktop);
}

void PlasmaWindowManagement::Private::windowCallback(void *data, org_kde_plasma_window_management *interface, uint32_t id)
{
    auto wm = reinterpret_cast<PlasmaWindowManagement::Private *>(data);
    Q_ASSERT(wm->wm == interface);
    QTimer *timer = new QTimer();
    timer->setSingleShot(true);
    timer->setInterval(0);
    QObject::connect(
        timer,
        &QTimer::timeout,
        wm->q,
        [timer, wm, id] {
            wm->windowCreated(org_kde_plasma_window_management_get_window(wm->wm, id), id, "unavailable");
            timer->deleteLater();
        },
        Qt::QueuedConnection);
    timer->start();
}

void PlasmaWindowManagement::Private::windowWithUuidCallback(void *data, org_kde_plasma_window_management *interface, uint32_t id, const char *_uuid)
{
    QByteArray uuid(_uuid);
    auto wm = reinterpret_cast<PlasmaWindowManagement::Private *>(data);
    Q_ASSERT(wm->wm == interface);
    QTimer *timer = new QTimer();
    timer->setSingleShot(true);
    timer->setInterval(0);
    QObject::connect(
        timer,
        &QTimer::timeout,
        wm->q,
        [timer, wm, id, uuid] {
            wm->windowCreated(org_kde_plasma_window_management_get_window_by_uuid(wm->wm, uuid), id, uuid);
            timer->deleteLater();
        },
        Qt::QueuedConnection);
    timer->start();
}

void PlasmaWindowManagement::Private::windowCreated(org_kde_plasma_window *id, quint32 internalId, const char *uuid)
{
    if (queue) {
        queue->addProxy(id);
    }
    PlasmaWindow *window = new PlasmaWindow(q, id, internalId, uuid);
    window->d->wm = q;
    windows << window;

    const auto windowRemoved = [this, window] {
        windows.removeAll(window);
        if (activeWindow == window) {
            activeWindow = nullptr;
            Q_EMIT q->activeWindowChanged();
        }
    };

    QObject::connect(window, &QObject::destroyed, q, windowRemoved);
    // unmapped is emitted earlier than QObject::destroyed. We want to update windows earlier to ensure other slot will see the up to date value of
    // PlasmaWindowManagement::windows().
    QObject::connect(window, &PlasmaWindow::unmapped, q, windowRemoved);
    QObject::connect(window, &PlasmaWindow::activeChanged, q, [this, window] {
        if (window->d->unmapped) {
            return;
        }
        if (window->isActive()) {
            if (activeWindow == window) {
                return;
            }
            activeWindow = window;
            Q_EMIT q->activeWindowChanged();
        } else {
            if (activeWindow == window) {
                activeWindow = nullptr;
                Q_EMIT q->activeWindowChanged();
            }
        }
    });
}

void PlasmaWindowManagement::Private::stackingOrderCallback(void *data, org_kde_plasma_window_management *interface, wl_array *ids)
{
    // This is no-op since setStackingOrder(const QList<quint32> &ids) is deprecated since 5.73,
    // but we can't remove this method because it's needed in
    // PlasmaWindowManagement::Private::s_listener struct
}

void PlasmaWindowManagement::Private::stackingOrderUuidsCallback(void *data, org_kde_plasma_window_management *interface, const char *uuids)
{
    auto wm = reinterpret_cast<PlasmaWindowManagement::Private *>(data);
    Q_ASSERT(wm->wm == interface);
    wm->setStackingOrder(QByteArray(uuids).split(';').toVector());
}

void PlasmaWindowManagement::Private::stackingOrderWindow(void *data, org_kde_plasma_stacking_order *org_kde_plasma_stacking_order, const char *uuid)
{
    Q_UNUSED(org_kde_plasma_stacking_order);
    auto order = static_cast<StackingOrderData *>(data);
    order->list.push_back(uuid);
}

void PlasmaWindowManagement::Private::stackingOrderDone(void *data, org_kde_plasma_stacking_order *org_kde_plasma_stacking_order)
{
    auto order = static_cast<StackingOrderData *>(data);
    if (order->wm) {
        order->wm->setStackingOrder(order->list);
    }
    delete order;
    org_kde_plasma_stacking_order_destroy(org_kde_plasma_stacking_order);
}

const org_kde_plasma_stacking_order_listener PlasmaWindowManagement::Private::s_stackingOrderListener = {
    .window = stackingOrderWindow,
    .done = stackingOrderDone,
};

void PlasmaWindowManagement::Private::stackingOrder2Callback(void *data, org_kde_plasma_window_management *interface)
{
    auto wm = reinterpret_cast<PlasmaWindowManagement::Private *>(data);
    Q_ASSERT(wm->wm == interface);
    auto object = org_kde_plasma_window_management_get_stacking_order(wm->wm);
    org_kde_plasma_stacking_order_add_listener(object,
                                               &s_stackingOrderListener,
                                               new StackingOrderData{
                                                   .wm = wm,
                                                   .list = {},
                                               });
}

void PlasmaWindowManagement::Private::setStackingOrder(const QList<QByteArray> &uuids)
{
    if (stackingOrderUuids == uuids) {
        return;
    }
    stackingOrderUuids = uuids;
    Q_EMIT q->stackingOrderUuidsChanged();
}

PlasmaWindowManagement::PlasmaWindowManagement(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

PlasmaWindowManagement::~PlasmaWindowManagement()
{
    release();
}

void PlasmaWindowManagement::destroy()
{
    if (!d->wm) {
        return;
    }
    Q_EMIT interfaceAboutToBeDestroyed();
    d->wm.destroy();
}

void PlasmaWindowManagement::release()
{
    if (!d->wm) {
        return;
    }
    Q_EMIT interfaceAboutToBeReleased();
    d->wm.release();
}

void PlasmaWindowManagement::setup(org_kde_plasma_window_management *wm)
{
    d->setup(wm);
}

void PlasmaWindowManagement::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *PlasmaWindowManagement::eventQueue()
{
    return d->queue;
}

bool PlasmaWindowManagement::isValid() const
{
    return d->wm.isValid();
}

PlasmaWindowManagement::operator org_kde_plasma_window_management *()
{
    return d->wm;
}

PlasmaWindowManagement::operator org_kde_plasma_window_management *() const
{
    return d->wm;
}

void PlasmaWindowManagement::hideDesktop()
{
    setShowingDesktop(false);
}

void PlasmaWindowManagement::showDesktop()
{
    setShowingDesktop(true);
}

void PlasmaWindowManagement::setShowingDesktop(bool show)
{
    org_kde_plasma_window_management_show_desktop(d->wm,
                                                  show ? ORG_KDE_PLASMA_WINDOW_MANAGEMENT_SHOW_DESKTOP_ENABLED
                                                       : ORG_KDE_PLASMA_WINDOW_MANAGEMENT_SHOW_DESKTOP_DISABLED);
}

bool PlasmaWindowManagement::isShowingDesktop() const
{
    return d->showingDesktop;
}

QList<PlasmaWindow *> PlasmaWindowManagement::windows() const
{
    return d->windows;
}

PlasmaWindow *PlasmaWindowManagement::activeWindow() const
{
    return d->activeWindow;
}

PlasmaWindowModel *PlasmaWindowManagement::createWindowModel()
{
    return new PlasmaWindowModel(this);
}

QList<QByteArray> PlasmaWindowManagement::stackingOrderUuids() const
{
    return d->stackingOrderUuids;
}

org_kde_plasma_window_listener PlasmaWindow::Private::s_listener = {
    titleChangedCallback,
    appIdChangedCallback,
    stateChangedCallback,
    virtualDesktopChangedCallback,
    themedIconNameChangedCallback,
    unmappedCallback,
    initialStateCallback,
    parentWindowCallback,
    windowGeometryCallback,
    iconChangedCallback,
    pidChangedCallback,
    virtualDesktopEnteredCallback,
    virtualDesktopLeftCallback,
    appmenuChangedCallback,
    activityEnteredCallback,
    activityLeftCallback,
    resourceNameChangedCallback,
    clientGeometryCallback,
};

void PlasmaWindow::Private::appmenuChangedCallback(void *data, org_kde_plasma_window *window, const char *service_name, const char *object_path)
{
    Q_UNUSED(window)

    Private *p = cast(data);

    p->applicationMenuServiceName = QString::fromUtf8(service_name);
    p->applicationMenuObjectPath = QString::fromUtf8(object_path);

    Q_EMIT p->q->applicationMenuChanged();
}

void PlasmaWindow::Private::parentWindowCallback(void *data, org_kde_plasma_window *window, org_kde_plasma_window *parent)
{
    Q_UNUSED(window)
    Private *p = cast(data);
    const auto windows = p->wm->windows();
    auto it = std::find_if(windows.constBegin(), windows.constEnd(), [parent](const PlasmaWindow *w) {
        return *w == parent;
    });
    p->setParentWindow(it != windows.constEnd() ? *it : nullptr);
}

void PlasmaWindow::Private::windowGeometryCallback(void *data, org_kde_plasma_window *window, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    Q_UNUSED(window)
    Private *p = cast(data);
    QRect geo(x, y, width, height);
    if (geo == p->geometry) {
        return;
    }
    p->geometry = geo;
    Q_EMIT p->q->geometryChanged();
}

void PlasmaWindow::Private::setParentWindow(PlasmaWindow *parent)
{
    const auto old = parentWindow;
    QObject::disconnect(parentWindowUnmappedConnection);
    if (parent && !parent->d->unmapped) {
        parentWindow = QPointer<PlasmaWindow>(parent);
        parentWindowUnmappedConnection = QObject::connect(parent, &PlasmaWindow::unmapped, q, [this] {
            setParentWindow(nullptr);
        });
    } else {
        parentWindow = QPointer<PlasmaWindow>();
        parentWindowUnmappedConnection = QMetaObject::Connection();
    }
    if (parentWindow.data() != old.data()) {
        Q_EMIT q->parentWindowChanged();
    }
}

void PlasmaWindow::Private::initialStateCallback(void *data, org_kde_plasma_window *window)
{
    Q_UNUSED(window)
    Private *p = cast(data);
    if (!p->unmapped) {
        Q_EMIT p->wm->windowCreated(p->q);
    }
}

void PlasmaWindow::Private::titleChangedCallback(void *data, org_kde_plasma_window *window, const char *title)
{
    Q_UNUSED(window)
    Private *p = cast(data);
    const QString t = QString::fromUtf8(title);
    if (p->title == t) {
        return;
    }
    p->title = t;
    Q_EMIT p->q->titleChanged();
}

void PlasmaWindow::Private::appIdChangedCallback(void *data, org_kde_plasma_window *window, const char *appId)
{
    Q_UNUSED(window)
    Private *p = cast(data);
    const QString s = QString::fromUtf8(appId);
    if (s == p->appId) {
        return;
    }
    p->appId = s;
    Q_EMIT p->q->appIdChanged();
}

void PlasmaWindow::Private::pidChangedCallback(void *data, org_kde_plasma_window *window, uint32_t pid)
{
    Q_UNUSED(window)
    Private *p = cast(data);
    if (p->pid == static_cast<quint32>(pid)) {
        return;
    }
    p->pid = pid;
}

void PlasmaWindow::Private::resourceNameChangedCallback(void *data, org_kde_plasma_window *window, const char *resourceName)
{
    Q_UNUSED(window)
    Private *p = cast(data);
    const QString s = QString::fromUtf8(resourceName);
    if (s == p->resourceName) {
        return;
    }
    p->resourceName = s;
    Q_EMIT p->q->resourceNameChanged();
}

void PlasmaWindow::Private::virtualDesktopChangedCallback([[maybe_unused]] void *data,
                                                          [[maybe_unused]] org_kde_plasma_window *window,
                                                          [[maybe_unused]] int32_t number)
{
    // Can't remove this method as it's used in PlasmaWindow::Private::s_listener struct
}

void PlasmaWindow::Private::unmappedCallback(void *data, org_kde_plasma_window *window)
{
    auto p = cast(data);
    Q_UNUSED(window);
    p->unmapped = true;
    Q_EMIT p->q->unmapped();
    p->q->deleteLater();
}

void PlasmaWindow::Private::virtualDesktopEnteredCallback(void *data, org_kde_plasma_window *window, const char *id)
{
    auto p = cast(data);
    Q_UNUSED(window);
    const QString stringId(QString::fromUtf8(id));
    p->plasmaVirtualDesktops << stringId;
    Q_EMIT p->q->plasmaVirtualDesktopEntered(stringId);
    if (p->plasmaVirtualDesktops.count() == 1) {
        Q_EMIT p->q->onAllDesktopsChanged();
    }
}

void PlasmaWindow::Private::virtualDesktopLeftCallback(void *data, org_kde_plasma_window *window, const char *id)
{
    auto p = cast(data);
    Q_UNUSED(window);
    const QString stringId(QString::fromUtf8(id));
    p->plasmaVirtualDesktops.removeAll(stringId);
    Q_EMIT p->q->plasmaVirtualDesktopLeft(stringId);
    if (p->plasmaVirtualDesktops.isEmpty()) {
        Q_EMIT p->q->onAllDesktopsChanged();
    }
}

void PlasmaWindow::Private::activityEnteredCallback(void *data, org_kde_plasma_window *window, const char *id)
{
    auto p = cast(data);
    Q_UNUSED(window);
    const QString stringId(QString::fromUtf8(id));
    p->plasmaActivities << stringId;
    Q_EMIT p->q->plasmaActivityEntered(stringId);
}

void PlasmaWindow::Private::activityLeftCallback(void *data, org_kde_plasma_window *window, const char *id)
{
    auto p = cast(data);
    Q_UNUSED(window);
    const QString stringId(QString::fromUtf8(id));
    p->plasmaActivities.removeAll(stringId);
    Q_EMIT p->q->plasmaActivityLeft(stringId);
}

void PlasmaWindow::Private::stateChangedCallback(void *data, org_kde_plasma_window *window, uint32_t state)
{
    auto p = cast(data);
    Q_UNUSED(window);
    p->setActive(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_ACTIVE);
    p->setMinimized(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MINIMIZED);
    p->setMaximized(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MAXIMIZED);
    p->setFullscreen(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_FULLSCREEN);
    p->setKeepAbove(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_KEEP_ABOVE);
    p->setKeepBelow(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_KEEP_BELOW);
    p->setOnAllDesktops(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_ON_ALL_DESKTOPS);
    p->setDemandsAttention(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_DEMANDS_ATTENTION);
    p->setCloseable(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_CLOSEABLE);
    p->setFullscreenable(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_FULLSCREENABLE);
    p->setMaximizeable(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MAXIMIZABLE);
    p->setMinimizeable(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MINIMIZABLE);
    p->setSkipTaskbar(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_SKIPTASKBAR);
    p->setSkipSwitcher(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_SKIPSWITCHER);
    p->setShadeable(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_SHADEABLE);
    p->setShaded(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_SHADED);
    p->setMovable(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MOVABLE);
    p->setResizable(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_RESIZABLE);
    p->setVirtualDesktopChangeable(state & ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_VIRTUAL_DESKTOP_CHANGEABLE);
}

void PlasmaWindow::Private::themedIconNameChangedCallback(void *data, org_kde_plasma_window *window, const char *name)
{
    auto p = cast(data);
    Q_UNUSED(window);
    const QString themedName = QString::fromUtf8(name);
    if (!themedName.isEmpty()) {
        QIcon icon = QIcon::fromTheme(themedName);
        p->icon = icon;
    } else {
        p->icon = QIcon();
    }
    Q_EMIT p->q->iconChanged();
}

static int readData(int fd, QByteArray &data)
{
    // implementation based on QtWayland file qwaylanddataoffer.cpp
    char buf[4096];
    int retryCount = 0;
    int n;
    while (true) {
        n = QT_READ(fd, buf, sizeof buf);
        if (n > 0) {
            data.append(buf, n);
        } else if (n == -1 && (errno == EAGAIN) && ++retryCount < 1000) {
            usleep(1000);
        } else {
            break;
        }
    }
    return n;
}

void PlasmaWindow::Private::iconChangedCallback(void *data, org_kde_plasma_window *window)
{
    auto p = cast(data);
    Q_UNUSED(window);
    int pipeFds[2];
    if (pipe2(pipeFds, O_CLOEXEC | O_NONBLOCK) != 0) {
        return;
    }
    org_kde_plasma_window_get_icon(p->window, pipeFds[1]);
    close(pipeFds[1]);
    const int pipeFd = pipeFds[0];
    auto readIcon = [pipeFd]() -> QIcon {
        QByteArray content;
        if (readData(pipeFd, content) != 0) {
            close(pipeFd);
            return QIcon();
        }
        close(pipeFd);
        QDataStream ds(content);
        QIcon icon;
        ds >> icon;
        return icon;
    };
    QFutureWatcher<QIcon> *watcher = new QFutureWatcher<QIcon>(p->q);
    QObject::connect(watcher, &QFutureWatcher<QIcon>::finished, p->q, [p, watcher] {
        watcher->deleteLater();
        QIcon icon = watcher->result();
        if (!icon.isNull()) {
            p->icon = icon;
        } else {
            p->icon = QIcon::fromTheme(QStringLiteral("wayland"));
        }
        Q_EMIT p->q->iconChanged();
    });
    watcher->setFuture(QtConcurrent::run(readIcon));
}

void PlasmaWindow::Private::setActive(bool set)
{
    if (active == set) {
        return;
    }
    active = set;
    Q_EMIT q->activeChanged();
}

void PlasmaWindow::Private::setFullscreen(bool set)
{
    if (fullscreen == set) {
        return;
    }
    fullscreen = set;
    Q_EMIT q->fullscreenChanged();
}

void PlasmaWindow::Private::setKeepAbove(bool set)
{
    if (keepAbove == set) {
        return;
    }
    keepAbove = set;
    Q_EMIT q->keepAboveChanged();
}

void PlasmaWindow::Private::setKeepBelow(bool set)
{
    if (keepBelow == set) {
        return;
    }
    keepBelow = set;
    Q_EMIT q->keepBelowChanged();
}

void PlasmaWindow::Private::setMaximized(bool set)
{
    if (maximized == set) {
        return;
    }
    maximized = set;
    Q_EMIT q->maximizedChanged();
}

void PlasmaWindow::Private::setMinimized(bool set)
{
    if (minimized == set) {
        return;
    }
    minimized = set;
    Q_EMIT q->minimizedChanged();
}

void PlasmaWindow::Private::setOnAllDesktops(bool set)
{
    if (onAllDesktops == set) {
        return;
    }
    onAllDesktops = set;
    Q_EMIT q->onAllDesktopsChanged();
}

void PlasmaWindow::Private::setDemandsAttention(bool set)
{
    if (demandsAttention == set) {
        return;
    }
    demandsAttention = set;
    Q_EMIT q->demandsAttentionChanged();
}

void PlasmaWindow::Private::setCloseable(bool set)
{
    if (closeable == set) {
        return;
    }
    closeable = set;
    Q_EMIT q->closeableChanged();
}

void PlasmaWindow::Private::setFullscreenable(bool set)
{
    if (fullscreenable == set) {
        return;
    }
    fullscreenable = set;
    Q_EMIT q->fullscreenableChanged();
}

void PlasmaWindow::Private::setMaximizeable(bool set)
{
    if (maximizeable == set) {
        return;
    }
    maximizeable = set;
    Q_EMIT q->maximizeableChanged();
}

void PlasmaWindow::Private::setMinimizeable(bool set)
{
    if (minimizeable == set) {
        return;
    }
    minimizeable = set;
    Q_EMIT q->minimizeableChanged();
}

void PlasmaWindow::Private::setSkipTaskbar(bool skip)
{
    if (skipTaskbar == skip) {
        return;
    }
    skipTaskbar = skip;
    Q_EMIT q->skipTaskbarChanged();
}

void PlasmaWindow::Private::setSkipSwitcher(bool skip)
{
    if (skipSwitcher == skip) {
        return;
    }
    skipSwitcher = skip;
    Q_EMIT q->skipSwitcherChanged();
}

void PlasmaWindow::Private::setShadeable(bool set)
{
    if (shadeable == set) {
        return;
    }
    shadeable = set;
    Q_EMIT q->shadeableChanged();
}

void PlasmaWindow::Private::setShaded(bool set)
{
    if (shaded == set) {
        return;
    }
    shaded = set;
    Q_EMIT q->shadedChanged();
}

void PlasmaWindow::Private::setMovable(bool set)
{
    if (movable == set) {
        return;
    }
    movable = set;
    Q_EMIT q->movableChanged();
}

void PlasmaWindow::Private::setResizable(bool set)
{
    if (resizable == set) {
        return;
    }
    resizable = set;
    Q_EMIT q->resizableChanged();
}

void PlasmaWindow::Private::setVirtualDesktopChangeable(bool set)
{
    if (virtualDesktopChangeable == set) {
        return;
    }
    virtualDesktopChangeable = set;
    Q_EMIT q->virtualDesktopChangeableChanged();
}

PlasmaWindow::Private::Private(org_kde_plasma_window *w, quint32 internalId, const char *uuid, PlasmaWindow *q)
    : internalId(internalId)
    , uuid(uuid)
    , q(q)
{
    Q_ASSERT(!this->uuid.isEmpty());
    window.setup(w);
    org_kde_plasma_window_add_listener(w, &s_listener, this);
}

PlasmaWindow::PlasmaWindow(PlasmaWindowManagement *parent, org_kde_plasma_window *window, quint32 internalId, const char *uuid)
    : QObject(parent)
    , d(new Private(window, internalId, uuid, this))
{
}

PlasmaWindow::~PlasmaWindow()
{
    release();
}

void PlasmaWindow::destroy()
{
    d->window.destroy();
}

void PlasmaWindow::release()
{
    d->window.release();
}

bool PlasmaWindow::isValid() const
{
    return d->window.isValid();
}

PlasmaWindow::operator org_kde_plasma_window *() const
{
    return d->window;
}

PlasmaWindow::operator org_kde_plasma_window *()
{
    return d->window;
}

QString PlasmaWindow::appId() const
{
    return d->appId;
}

quint32 PlasmaWindow::pid() const
{
    return d->pid;
}

QString PlasmaWindow::resourceName() const
{
    return d->resourceName;
}

QString PlasmaWindow::title() const
{
    return d->title;
}

bool PlasmaWindow::isActive() const
{
    return d->active;
}

bool PlasmaWindow::isFullscreen() const
{
    return d->fullscreen;
}

bool PlasmaWindow::isKeepAbove() const
{
    return d->keepAbove;
}

bool PlasmaWindow::isKeepBelow() const
{
    return d->keepBelow;
}

bool PlasmaWindow::isMaximized() const
{
    return d->maximized;
}

bool PlasmaWindow::isMinimized() const
{
    return d->minimized;
}

bool PlasmaWindow::isOnAllDesktops() const
{
    // from protocol version 8 virtual desktops are managed by plasmaVirtualDesktops
    if (org_kde_plasma_window_get_version(d->window) < 8) {
        return d->onAllDesktops;
    } else {
        return d->plasmaVirtualDesktops.isEmpty();
    }
}

bool PlasmaWindow::isDemandingAttention() const
{
    return d->demandsAttention;
}

bool PlasmaWindow::isCloseable() const
{
    return d->closeable;
}

bool PlasmaWindow::isFullscreenable() const
{
    return d->fullscreenable;
}

bool PlasmaWindow::isMaximizeable() const
{
    return d->maximizeable;
}

bool PlasmaWindow::isMinimizeable() const
{
    return d->minimizeable;
}

bool PlasmaWindow::skipTaskbar() const
{
    return d->skipTaskbar;
}

bool PlasmaWindow::skipSwitcher() const
{
    return d->skipSwitcher;
}

QIcon PlasmaWindow::icon() const
{
    return d->icon;
}

bool PlasmaWindow::isShadeable() const
{
    return d->shadeable;
}

bool PlasmaWindow::isShaded() const
{
    return d->shaded;
}

bool PlasmaWindow::isResizable() const
{
    return d->resizable;
}

bool PlasmaWindow::isMovable() const
{
    return d->movable;
}

bool PlasmaWindow::isVirtualDesktopChangeable() const
{
    return d->virtualDesktopChangeable;
}

QString PlasmaWindow::applicationMenuObjectPath() const
{
    return d->applicationMenuObjectPath;
}

QString PlasmaWindow::applicationMenuServiceName() const
{
    return d->applicationMenuServiceName;
}

void PlasmaWindow::requestActivate()
{
    org_kde_plasma_window_set_state(d->window, ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_ACTIVE, ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_ACTIVE);
}

void PlasmaWindow::requestClose()
{
    org_kde_plasma_window_close(d->window);
}

void PlasmaWindow::requestMove()
{
    org_kde_plasma_window_request_move(d->window);
}

void PlasmaWindow::requestResize()
{
    org_kde_plasma_window_request_resize(d->window);
}

void PlasmaWindow::requestToggleKeepAbove()
{
    if (d->keepAbove) {
        org_kde_plasma_window_set_state(d->window, ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_KEEP_ABOVE, 0);
    } else {
        org_kde_plasma_window_set_state(d->window, ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_KEEP_ABOVE, ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_KEEP_ABOVE);
    }
}

void PlasmaWindow::requestToggleKeepBelow()
{
    if (d->keepBelow) {
        org_kde_plasma_window_set_state(d->window, ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_KEEP_BELOW, 0);
    } else {
        org_kde_plasma_window_set_state(d->window, ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_KEEP_BELOW, ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_KEEP_BELOW);
    }
}

void PlasmaWindow::requestToggleMinimized()
{
    if (d->minimized) {
        org_kde_plasma_window_set_state(d->window, ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MINIMIZED, 0);
    } else {
        org_kde_plasma_window_set_state(d->window, ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MINIMIZED, ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MINIMIZED);
    }
}

void PlasmaWindow::requestToggleMaximized()
{
    if (d->maximized) {
        org_kde_plasma_window_set_state(d->window, ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MAXIMIZED, 0);
    } else {
        org_kde_plasma_window_set_state(d->window, ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MAXIMIZED, ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MAXIMIZED);
    }
}

void PlasmaWindow::requestToggleFullscreen()
{
    if (d->fullscreen) {
        org_kde_plasma_window_set_state(d->window, ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_FULLSCREEN, 0);
    } else {
        org_kde_plasma_window_set_state(d->window, ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_FULLSCREEN, ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_FULLSCREEN);
    }
}

void PlasmaWindow::setMinimizedGeometry(Surface *panel, const QRect &geom)
{
    org_kde_plasma_window_set_minimized_geometry(d->window, *panel, geom.x(), geom.y(), geom.width(), geom.height());
}

void PlasmaWindow::unsetMinimizedGeometry(Surface *panel)
{
    org_kde_plasma_window_unset_minimized_geometry(d->window, *panel);
}

void PlasmaWindow::requestToggleShaded()
{
    if (d->shaded) {
        org_kde_plasma_window_set_state(d->window, ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_SHADED, 0);
    } else {
        org_kde_plasma_window_set_state(d->window, ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_SHADED, ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_SHADED);
    }
}

QByteArray PlasmaWindow::uuid() const
{
    return d->uuid;
}

QPointer<PlasmaWindow> PlasmaWindow::parentWindow() const
{
    return d->parentWindow;
}

QRect PlasmaWindow::geometry() const
{
    return d->geometry;
}

void PlasmaWindow::requestEnterVirtualDesktop(const QString &id)
{
    org_kde_plasma_window_request_enter_virtual_desktop(d->window, id.toUtf8());
}

void PlasmaWindow::requestEnterNewVirtualDesktop()
{
    org_kde_plasma_window_request_enter_new_virtual_desktop(d->window);
}

void PlasmaWindow::requestLeaveVirtualDesktop(const QString &id)
{
    org_kde_plasma_window_request_leave_virtual_desktop(d->window, id.toUtf8());
}

QStringList PlasmaWindow::plasmaVirtualDesktops() const
{
    return d->plasmaVirtualDesktops;
}

void PlasmaWindow::requestEnterActivity(const QString &id)
{
    org_kde_plasma_window_request_enter_activity(d->window, id.toUtf8());
}

void PlasmaWindow::requestLeaveActivity(const QString &id)
{
    org_kde_plasma_window_request_leave_activity(d->window, id.toUtf8());
}

QStringList PlasmaWindow::plasmaActivities() const
{
    return d->plasmaActivities;
}

void PlasmaWindow::sendToOutput(KWayland::Client::Output *output) const
{
    if (org_kde_plasma_window_get_version(d->window) >= ORG_KDE_PLASMA_WINDOW_SEND_TO_OUTPUT_SINCE_VERSION) {
        org_kde_plasma_window_send_to_output(d->window, *output);
    }
}

class Q_DECL_HIDDEN PlasmaActivationFeedback::Private
{
public:
    Private(PlasmaActivationFeedback *q);
    WaylandPointer<org_kde_plasma_activation_feedback, org_kde_plasma_activation_feedback_destroy> feedback;
    EventQueue *queue = nullptr;

    void setup(org_kde_plasma_activation_feedback *feedback);

private:
    static void activationCallback(void *data, struct org_kde_plasma_activation_feedback *feedback, struct org_kde_plasma_activation *id);

    static struct org_kde_plasma_activation_feedback_listener s_listener;
    PlasmaActivationFeedback *q;
};

PlasmaActivationFeedback::Private::Private(PlasmaActivationFeedback *q)
    : q(q)
{
}

org_kde_plasma_activation_feedback_listener PlasmaActivationFeedback::Private::s_listener = {
    activationCallback,
};

void PlasmaActivationFeedback::Private::activationCallback(void *data, org_kde_plasma_activation_feedback *interface, struct org_kde_plasma_activation *id)
{
    auto feedbackPrivate = reinterpret_cast<PlasmaActivationFeedback::Private *>(data);
    Q_ASSERT(feedbackPrivate->feedback == interface);
    auto activation = new PlasmaActivation(feedbackPrivate->q, id);
    Q_EMIT feedbackPrivate->q->activation(activation);
}

void PlasmaActivationFeedback::Private::setup(org_kde_plasma_activation_feedback *m)
{
    Q_ASSERT(!feedback);
    Q_ASSERT(m);
    feedback.setup(m);
    org_kde_plasma_activation_feedback_add_listener(m, &s_listener, this);
}

PlasmaActivationFeedback::PlasmaActivationFeedback(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

PlasmaActivationFeedback::~PlasmaActivationFeedback()
{
    release();
}

void PlasmaActivationFeedback::destroy()
{
    if (!d->feedback) {
        return;
    }
    Q_EMIT interfaceAboutToBeDestroyed();
    d->feedback.destroy();
}

void PlasmaActivationFeedback::release()
{
    if (!d->feedback) {
        return;
    }
    Q_EMIT interfaceAboutToBeReleased();
    d->feedback.release();
}

void PlasmaActivationFeedback::setup(org_kde_plasma_activation_feedback *wm)
{
    d->setup(wm);
}

void PlasmaActivationFeedback::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *PlasmaActivationFeedback::eventQueue()
{
    return d->queue;
}

bool PlasmaActivationFeedback::isValid() const
{
    return d->feedback.isValid();
}

PlasmaActivationFeedback::operator org_kde_plasma_activation_feedback *()
{
    return d->feedback;
}

PlasmaActivationFeedback::operator org_kde_plasma_activation_feedback *() const
{
    return d->feedback;
}

class Q_DECL_HIDDEN PlasmaActivation::Private
{
public:
    Private(org_kde_plasma_activation *activation, PlasmaActivation *q)
        : activation(activation)
    {
        org_kde_plasma_activation_add_listener(activation, &s_listener, q);
    }

    static PlasmaActivation *cast(void *data)
    {
        return reinterpret_cast<PlasmaActivation *>(data);
    }
    WaylandPointer<org_kde_plasma_activation, org_kde_plasma_activation_destroy> activation;

    static org_kde_plasma_activation_listener s_listener;
    static void app_idCallback(void *data, struct org_kde_plasma_activation *org_kde_plasma_activation, const char *app_id);
    static void finishedCallback(void *data, struct org_kde_plasma_activation *org_kde_plasma_activation);
};

org_kde_plasma_activation_listener PlasmaActivation::Private::s_listener = {
    app_idCallback,
    finishedCallback,
};

void PlasmaActivation::Private::app_idCallback(void *data, org_kde_plasma_activation *activation, const char *appId)
{
    Q_UNUSED(activation)
    Q_EMIT cast(data)->applicationId(QString::fromUtf8(appId));
}

void PlasmaActivation::Private::finishedCallback(void *data, org_kde_plasma_activation *)
{
    auto q = cast(data);
    Q_EMIT q->finished();
    Q_EMIT q->deleteLater();
    q->d->activation.release();
}

PlasmaActivation::PlasmaActivation(PlasmaActivationFeedback *parent, org_kde_plasma_activation *activation)
    : QObject(parent)
    , d(new PlasmaActivation::Private(activation, this))
{
}

PlasmaActivation::~PlasmaActivation() = default;

QRect PlasmaWindow::clientGeometry() const
{
    return d->clientGeometry;
}

void PlasmaWindow::Private::clientGeometryCallback(void *data, org_kde_plasma_window *window, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    Q_UNUSED(window)
    Private *p = cast(data);
    const QRect geo(x, y, width, height);
    if (p->clientGeometry == geo) {
        return;
    }
    p->clientGeometry = geo;
    Q_EMIT p->q->clientGeometryChanged();
}

}
}

#include "plasmawindowmanagement.moc"
