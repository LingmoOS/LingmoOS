#include "lingmowindowmanagement.h"



#include "lingmo-window-management-client-protocol.h"
#include "lingmowaylandpointer.h"

#include <QFutureWatcher>
#include <QTimer>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <QtConcurrentRun>


class Q_DECL_HIDDEN LingmoUIWindowManagement::Private
{
public:
    Private(LingmoUIWindowManagement *q);
    LingmoUIWaylandPointer<lingmo_window_management, lingmo_window_management_destroy> wm;
    EventQueue *queue = nullptr;
    bool showingDesktop = false;
    QList<LingmoUIWindow *> windows;
    LingmoUIWindow *activeWindow = nullptr;
    QVector<quint32> stackingOrder;
    QVector<QByteArray> stackingOrderUuids;

    void setup(lingmo_window_management *wm);

private:
    static void showDesktopCallback(void *data, lingmo_window_management *lingmo_window_management, uint32_t state);

    static void windowCallback(void *data, struct lingmo_window_management *lingmo_window_management, const char *uuid);
    static void windowWithUuidCallback(void *data, struct lingmo_window_management *lingmo_window_management, uint32_t id, const char *uuid);
    static void stackingOrderUuidsCallback(void *data, struct lingmo_window_management *lingmo_window_management, const char *uuids);
    void setShowDesktop(bool set);
    void windowCreated(lingmo_window *id, quint32 internalId, const char *uuid);
    void setStackingOrder(const QVector<quint32> &ids);
    void setStackingOrder(const QVector<QByteArray> &uuids);

    static struct lingmo_window_management_listener s_listener;
    LingmoUIWindowManagement *q;
};

class Q_DECL_HIDDEN LingmoUIWindow::Private
{
public:
    Private(lingmo_window *window, quint32 internalId, const char *uuid, LingmoUIWindow *q);
    LingmoUIWaylandPointer<lingmo_window, lingmo_window_destroy> window;
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
    bool acceptFocus = false;
    bool modality = false;
    bool virtualDesktopChangeable = false;
    QIcon icon;
    LingmoUIWindowManagement *wm = nullptr;
    bool unmapped = false;
    QPointer<LingmoUIWindow> parentWindow;
    QMetaObject::Connection parentWindowUnmappedConnection;
    QStringList lingmoVirtualDesktops;
    QStringList lingmoActivities;
    QRect geometry;
    quint32 pid = 0;
    QString applicationMenuServiceName;
    QString applicationMenuObjectPath;
    bool highlight = false;

private:
    static void titleChangedCallback(void *data, lingmo_window *window, const char *title);
    static void appIdChangedCallback(void *data, lingmo_window *window, const char *app_id);
    static void pidChangedCallback(void *data, lingmo_window *window, uint32_t pid);
    static void stateChangedCallback(void *data, lingmo_window *window, uint32_t state);
    static void themedIconNameChangedCallback(void *data, lingmo_window *window, const char *name);
    static void unmappedCallback(void *data, lingmo_window *window);
    static void initialStateCallback(void *data, lingmo_window *window);
    static void parentWindowCallback(void *data, lingmo_window *window, lingmo_window *parent);
    static void windowGeometryCallback(void *data, lingmo_window *window, int32_t x, int32_t y, uint32_t width, uint32_t height);
    static void iconChangedCallback(void *data, lingmo_window *lingmo_window);
    static void virtualDesktopEnteredCallback(void *data, lingmo_window *lingmo_window, const char *id);
    static void virtualDesktopLeftCallback(void *data, lingmo_window *lingmo_window, const char *id);
    static void appmenuChangedCallback(void *data, lingmo_window *lingmo_window, const char *service_name, const char *object_path);
    static void activityEnteredCallback(void *data, lingmo_window *lingmo_window, const char *id);
    static void activityLeftCallback(void *data, lingmo_window *lingmo_window, const char *id);
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
    void setAcceptFocus(bool set);
    void setModality(bool set);
    void setParentWindow(LingmoUIWindow *parentWindow);
    void setPid(const quint32 pid);

    static Private *cast(void *data)
    {
        return reinterpret_cast<Private *>(data);
    }

    LingmoUIWindow *q;

    static struct lingmo_window_listener s_listener;
};

LingmoUIWindowManagement::Private::Private(LingmoUIWindowManagement *q)
    : q(q)
{
}

lingmo_window_management_listener LingmoUIWindowManagement::Private::s_listener = {
    .show_desktop_changed = showDesktopCallback,
    .stacking_order_changed = stackingOrderUuidsCallback,
    .window_created = windowCallback,
};

void LingmoUIWindowManagement::Private::setup(lingmo_window_management *windowManagement)
{
    Q_ASSERT(!wm);
    Q_ASSERT(windowManagement);
    wm.setup(windowManagement);
    lingmo_window_management_add_listener(windowManagement, &s_listener, this);
}

void LingmoUIWindowManagement::Private::showDesktopCallback(void *data, lingmo_window_management *lingmo_window_management, uint32_t state)
{
    auto wm = reinterpret_cast<LingmoUIWindowManagement::Private *>(data);
    Q_ASSERT(wm->wm == lingmo_window_management);
    switch (state) {
    case LINGMO_WINDOW_MANAGEMENT_SHOW_DESKTOP_ENABLED:
        wm->setShowDesktop(true);
        break;
    case LINGMO_WINDOW_MANAGEMENT_SHOW_DESKTOP_DISABLED:
        wm->setShowDesktop(false);
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
}

void LingmoUIWindowManagement::Private::setShowDesktop(bool set)
{
    if (showingDesktop == set) {
        return;
    }
    showingDesktop = set;
    Q_EMIT q->showingDesktopChanged(showingDesktop);
}

void LingmoUIWindowManagement::Private::windowCallback(void *data, lingmo_window_management *interface, const char *_uuid)
{
    QByteArray uuid(_uuid);
    auto wm = reinterpret_cast<LingmoUIWindowManagement::Private *>(data);
    Q_ASSERT(wm->wm == interface);
    QTimer *timer = new QTimer();
    timer->setSingleShot(true);
    timer->setInterval(0);
    QObject::connect(
        timer,
        &QTimer::timeout,
        wm->q,
        [timer, wm, uuid] {
            //internalId 不用了，以uuid为唯一标识
            wm->windowCreated(lingmo_window_management_create_window(wm->wm, uuid), 0, uuid);
            timer->deleteLater();
        },
        Qt::QueuedConnection);
    timer->start();
}

//not used
void LingmoUIWindowManagement::Private::windowWithUuidCallback(void *data, lingmo_window_management *interface, uint32_t id, const char *_uuid)
{
    QByteArray uuid(_uuid);
    auto wm = reinterpret_cast<LingmoUIWindowManagement::Private *>(data);
    Q_ASSERT(wm->wm == interface);
    QTimer *timer = new QTimer();
    timer->setSingleShot(true);
    timer->setInterval(0);
    QObject::connect(
        timer,
        &QTimer::timeout,
        wm->q,
        [timer, wm, id, uuid] {
            wm->windowCreated(lingmo_window_management_create_window(wm->wm, uuid), id, uuid);
            timer->deleteLater();
        },
        Qt::QueuedConnection);
    timer->start();
}

void LingmoUIWindowManagement::Private::windowCreated(lingmo_window *id, quint32 internalId, const char *uuid)
{
    if (queue) {
        queue->addProxy(id);
    }
    LingmoUIWindow *window = new LingmoUIWindow(q, id, internalId, uuid);
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
    QObject::connect(window, &LingmoUIWindow::unmapped, q, windowRemoved);
    QObject::connect(window, &LingmoUIWindow::activeChanged, q, [this, window] {
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

void LingmoUIWindowManagement::Private::stackingOrderUuidsCallback(void *data, lingmo_window_management *interface, const char *uuids)
{
    auto wm = reinterpret_cast<LingmoUIWindowManagement::Private *>(data);
    Q_ASSERT(wm->wm == interface);
    wm->setStackingOrder(QByteArray(uuids).split(';').toVector());
}


void LingmoUIWindowManagement::Private::setStackingOrder(const QVector<quint32> &ids)
{
    if (stackingOrder == ids) {
        return;
    }
    stackingOrder = ids;
    Q_EMIT q->stackingOrderChanged();
}


void LingmoUIWindowManagement::Private::setStackingOrder(const QVector<QByteArray> &uuids)
{
    if (stackingOrderUuids == uuids) {
        return;
    }
    stackingOrderUuids = uuids;
    Q_EMIT q->stackingOrderUuidsChanged();
}

LingmoUIWindowManagement::LingmoUIWindowManagement(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

LingmoUIWindowManagement::~LingmoUIWindowManagement()
{
    release();
}

void LingmoUIWindowManagement::destroy()
{
    if (!d->wm) {
        return;
    }
    Q_EMIT interfaceAboutToBeDestroyed();
    d->wm.destroy();
}

void LingmoUIWindowManagement::release()
{
    if (!d->wm) {
        return;
    }
    Q_EMIT interfaceAboutToBeReleased();
    d->wm.release();
}

void LingmoUIWindowManagement::setup(lingmo_window_management *wm)
{
    d->setup(wm);
}

void LingmoUIWindowManagement::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *LingmoUIWindowManagement::eventQueue()
{
    return d->queue;
}

bool LingmoUIWindowManagement::isValid() const
{
    return d->wm.isValid();
}

LingmoUIWindowManagement::operator lingmo_window_management *()
{
    return d->wm;
}

LingmoUIWindowManagement::operator lingmo_window_management *() const
{
    return d->wm;
}

void LingmoUIWindowManagement::hideDesktop()
{
    setShowingDesktop(false);
}

void LingmoUIWindowManagement::showDesktop()
{
    setShowingDesktop(true);
}

void LingmoUIWindowManagement::setShowingDesktop(bool show)
{
    lingmo_window_management_show_desktop(d->wm,
                                                  show ? LINGMO_WINDOW_MANAGEMENT_SHOW_DESKTOP_ENABLED
                                                       : LINGMO_WINDOW_MANAGEMENT_SHOW_DESKTOP_DISABLED);
}

bool LingmoUIWindowManagement::isShowingDesktop() const
{
    return d->showingDesktop;
}

QList<LingmoUIWindow *> LingmoUIWindowManagement::windows() const
{
    return d->windows;
}

LingmoUIWindow *LingmoUIWindowManagement::activeWindow() const
{
    return d->activeWindow;
}

QVector<quint32> LingmoUIWindowManagement::stackingOrder() const
{
    return d->stackingOrder;
}


QVector<QByteArray> LingmoUIWindowManagement::stackingOrderUuids() const
{
    return d->stackingOrderUuids;
}

lingmo_window_listener LingmoUIWindow::Private::s_listener = {
    .title_changed = titleChangedCallback,
    .app_id_changed = appIdChangedCallback,
    .state_changed = stateChangedCallback,
    .themed_icon_name_changed = themedIconNameChangedCallback,
    .unmapped = unmappedCallback,
    .initial_state = initialStateCallback,
    .parent_window = parentWindowCallback,
    .geometry = windowGeometryCallback,
    .icon_changed = iconChangedCallback,
    .pid_changed = pidChangedCallback,
    .virtual_desktop_entered = virtualDesktopEnteredCallback,
    .virtual_desktop_left = virtualDesktopLeftCallback,
    .application_menu = appmenuChangedCallback,
    .activity_entered = activityEnteredCallback,
    .activity_left = activityLeftCallback
};

void LingmoUIWindow::Private::appmenuChangedCallback(void *data, lingmo_window *window, const char *service_name, const char *object_path)
{
    Q_UNUSED(window)

    Private *p = cast(data);

    p->applicationMenuServiceName = QString::fromUtf8(service_name);
    p->applicationMenuObjectPath = QString::fromUtf8(object_path);

    Q_EMIT p->q->applicationMenuChanged();
}

void LingmoUIWindow::Private::parentWindowCallback(void *data, lingmo_window *window, lingmo_window *parent)
{
    Q_UNUSED(window)
    Private *p = cast(data);
    const auto windows = p->wm->windows();
    auto it = std::find_if(windows.constBegin(), windows.constEnd(), [parent](const LingmoUIWindow *w) {
        return *w == parent;
    });
    p->setParentWindow(it != windows.constEnd() ? *it : nullptr);
}

void LingmoUIWindow::Private::windowGeometryCallback(void *data, lingmo_window *window, int32_t x, int32_t y, uint32_t width, uint32_t height)
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

void LingmoUIWindow::Private::setParentWindow(LingmoUIWindow *parent)
{
    const auto old = parentWindow;
    QObject::disconnect(parentWindowUnmappedConnection);
    if (parent && !parent->d->unmapped) {
        parentWindow = QPointer<LingmoUIWindow>(parent);
        parentWindowUnmappedConnection = QObject::connect(parent, &LingmoUIWindow::unmapped, q, [this] {
            setParentWindow(nullptr);
        });
    } else {
        parentWindow = QPointer<LingmoUIWindow>();
        parentWindowUnmappedConnection = QMetaObject::Connection();
    }
    if (parentWindow.data() != old.data()) {
        Q_EMIT q->parentWindowChanged();
    }
}

void LingmoUIWindow::Private::initialStateCallback(void *data, lingmo_window *window)
{
    Q_UNUSED(window)
    Private *p = cast(data);
    if (!p->unmapped) {
        Q_EMIT p->wm->windowCreated(p->q);
    }
}

void LingmoUIWindow::Private::titleChangedCallback(void *data, lingmo_window *window, const char *title)
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

void LingmoUIWindow::Private::appIdChangedCallback(void *data, lingmo_window *window, const char *appId)
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

void LingmoUIWindow::Private::pidChangedCallback(void *data, lingmo_window *window, uint32_t pid)
{
    Q_UNUSED(window)
    Private *p = cast(data);
    if (p->pid == static_cast<quint32>(pid)) {
        return;
    }
    p->pid = pid;
}

void LingmoUIWindow::Private::unmappedCallback(void *data, lingmo_window *window)
{
    auto p = cast(data);
    Q_UNUSED(window);
    p->unmapped = true;
    Q_EMIT p->q->unmapped();
    p->q->deleteLater();
}

void LingmoUIWindow::Private::virtualDesktopEnteredCallback(void *data, lingmo_window *window, const char *id)
{
    auto p = cast(data);
    Q_UNUSED(window);
    const QString stringId(QString::fromUtf8(id));
    p->lingmoVirtualDesktops << stringId;
    Q_EMIT p->q->lingmoVirtualDesktopEntered(stringId);
    if (p->lingmoVirtualDesktops.count() == 1) {
        Q_EMIT p->q->onAllDesktopsChanged();
    }
}

void LingmoUIWindow::Private::virtualDesktopLeftCallback(void *data, lingmo_window *window, const char *id)
{
    auto p = cast(data);
    Q_UNUSED(window);
    const QString stringId(QString::fromUtf8(id));
    p->lingmoVirtualDesktops.removeAll(stringId);
    Q_EMIT p->q->lingmoVirtualDesktopLeft(stringId);
    if (p->lingmoVirtualDesktops.isEmpty()) {
        Q_EMIT p->q->onAllDesktopsChanged();
    }
}

void LingmoUIWindow::Private::activityEnteredCallback(void *data, lingmo_window *window, const char *id)
{
    auto p = cast(data);
    Q_UNUSED(window);
    const QString stringId(QString::fromUtf8(id));
    p->lingmoActivities << stringId;
    Q_EMIT p->q->lingmoActivityEntered(stringId);
}

void LingmoUIWindow::Private::activityLeftCallback(void *data, lingmo_window *window, const char *id)
{
    auto p = cast(data);
    Q_UNUSED(window);
    const QString stringId(QString::fromUtf8(id));
    p->lingmoActivities.removeAll(stringId);
    Q_EMIT p->q->lingmoActivityLeft(stringId);
}

void LingmoUIWindow::Private::stateChangedCallback(void *data, lingmo_window *window, uint32_t state)
{
    auto p = cast(data);

    Q_UNUSED(window);
    p->setActive(state & LINGMO_WINDOW_STATE_ACTIVE);
    p->setMinimized(state & LINGMO_WINDOW_STATE_MINIMIZED);
    p->setMaximized(state & LINGMO_WINDOW_STATE_MAXIMIZED);
    p->setFullscreen(state & LINGMO_WINDOW_STATE_FULLSCREEN);
    p->setKeepAbove(state & LINGMO_WINDOW_STATE_KEEP_ABOVE);
    p->setKeepBelow(state & LINGMO_WINDOW_STATE_KEEP_BELOW);
    p->setOnAllDesktops(state & LINGMO_WINDOW_STATE_ON_ALL_DESKTOPS);
    p->setDemandsAttention(state & LINGMO_WINDOW_STATE_DEMANDS_ATTENTION);
    p->setCloseable(state & LINGMO_WINDOW_STATE_CLOSEABLE);
    p->setFullscreenable(state & LINGMO_WINDOW_STATE_FULLSCREENABLE);
    p->setMaximizeable(state & LINGMO_WINDOW_STATE_MAXIMIZABLE);
    p->setMinimizeable(state & LINGMO_WINDOW_STATE_MINIMIZABLE);
    p->setSkipTaskbar(state & LINGMO_WINDOW_STATE_SKIPTASKBAR);
    p->setSkipSwitcher(state & LINGMO_WINDOW_STATE_SKIPSWITCHER);
    p->setShadeable(state & LINGMO_WINDOW_STATE_SHADEABLE);
    p->setShaded(state & LINGMO_WINDOW_STATE_SHADED);
    p->setMovable(state & LINGMO_WINDOW_STATE_MOVABLE);
    p->setResizable(state & LINGMO_WINDOW_STATE_RESIZABLE);
    p->setVirtualDesktopChangeable(state & LINGMO_WINDOW_STATE_VIRTUAL_DESKTOP_CHANGEABLE);
}

void LingmoUIWindow::Private::themedIconNameChangedCallback(void *data, lingmo_window *window, const char *name)
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
    char buf[4096];
    int retryCount = 0;
    int n;
    while (true)
    {
        n = read(fd, buf, 4096);
        if (n == -1 && (errno == EAGAIN) && ++retryCount < 1000) {
            usleep(1000);
        } else
        {
            break;
        }
    }
    if (n > 0) {
        data.append(buf, n);
        n = readData(fd, data);
    }
    return n;
}

void LingmoUIWindow::Private::iconChangedCallback(void *data, lingmo_window *window)
{
    auto p = cast(data);
    Q_UNUSED(window);
    int pipeFds[2];
    if (pipe2(pipeFds, O_CLOEXEC | O_NONBLOCK) != 0) {
        return;
    }
    lingmo_window_get_icon(p->window, pipeFds[1]);
    close(pipeFds[1]);
    const int pipeFd = pipeFds[0];
    auto readIcon = [pipeFd]() -> QIcon {
        QByteArray content;
        if (readData(pipeFd, content) != 0) {
            close(pipeFd);
            return QIcon();
        }
        close(pipeFd);
        if (qgetenv("XDG_SESSION_DESKTOP") == QString("lingmo-wlcom"))
        {
            int width = (content.at(0) << 0) | 
                    (content.at(1) << 8) |
                    (content.at(2) << 16) |
                    (content.at(3) << 24);
            int height = (content.at(4) << 0) | 
                    (content.at(5) << 8) |
                    (content.at(6) << 16) |
                    (content.at(7) << 24);
            if (content.size() - 8 != width * height *4 ) {
                return QIcon();
            }
            QImage image(width, height, QImage::Format_ARGB32);
            memcpy(image.bits(), content.constData() + 8, content.size() - 8);
            QIcon icon = QIcon(QPixmap::fromImage(image));
            return icon;
        }
        else
        {
            QDataStream ds(content);
            QIcon icon;
            ds >> icon;
            return icon;
        }
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

void LingmoUIWindow::Private::setActive(bool set)
{
    if (active == set) {
        return;
    }
    active = set;
    Q_EMIT q->activeChanged();
}

void LingmoUIWindow::Private::setFullscreen(bool set)
{
    if (fullscreen == set) {
        return;
    }
    fullscreen = set;
    Q_EMIT q->fullscreenChanged();
}

void LingmoUIWindow::Private::setKeepAbove(bool set)
{
    if (keepAbove == set) {
        return;
    }
    keepAbove = set;
    Q_EMIT q->keepAboveChanged();
}

void LingmoUIWindow::Private::setKeepBelow(bool set)
{
    if (keepBelow == set) {
        return;
    }
    keepBelow = set;
    Q_EMIT q->keepBelowChanged();
}

void LingmoUIWindow::Private::setMaximized(bool set)
{
    if (maximized == set) {
        return;
    }
    maximized = set;
    Q_EMIT q->maximizedChanged();
}

void LingmoUIWindow::Private::setMinimized(bool set)
{
    if (minimized == set) {
        return;
    }
    minimized = set;
    Q_EMIT q->minimizedChanged();
}

void LingmoUIWindow::Private::setOnAllDesktops(bool set)
{
    if (onAllDesktops == set) {
        return;
    }
    onAllDesktops = set;
    Q_EMIT q->onAllDesktopsChanged();
}

void LingmoUIWindow::Private::setDemandsAttention(bool set)
{
    if (demandsAttention == set) {
        return;
    }
    demandsAttention = set;
    Q_EMIT q->demandsAttentionChanged();
}

void LingmoUIWindow::Private::setCloseable(bool set)
{
    if (closeable == set) {
        return;
    }
    closeable = set;
    Q_EMIT q->closeableChanged();
}

void LingmoUIWindow::Private::setFullscreenable(bool set)
{
    if (fullscreenable == set) {
        return;
    }
    fullscreenable = set;
    Q_EMIT q->fullscreenableChanged();
}

void LingmoUIWindow::Private::setMaximizeable(bool set)
{
    if (maximizeable == set) {
        return;
    }
    maximizeable = set;
    Q_EMIT q->maximizeableChanged();
}

void LingmoUIWindow::Private::setMinimizeable(bool set)
{
    if (minimizeable == set) {
        return;
    }
    minimizeable = set;
    Q_EMIT q->minimizeableChanged();
}

void LingmoUIWindow::Private::setSkipTaskbar(bool skip)
{
    if (skipTaskbar == skip) {
        return;
    }
    skipTaskbar = skip;
    Q_EMIT q->skipTaskbarChanged();
}

void LingmoUIWindow::Private::setSkipSwitcher(bool skip)
{
    if (skipSwitcher == skip) {
        return;
    }
    skipSwitcher = skip;
    Q_EMIT q->skipSwitcherChanged();
}

void LingmoUIWindow::Private::setShadeable(bool set)
{
    if (shadeable == set) {
        return;
    }
    shadeable = set;
    Q_EMIT q->shadeableChanged();
}

void LingmoUIWindow::Private::setShaded(bool set)
{
    if (shaded == set) {
        return;
    }
    shaded = set;
    Q_EMIT q->shadedChanged();
}

void LingmoUIWindow::Private::setMovable(bool set)
{
    if (movable == set) {
        return;
    }
    movable = set;
    Q_EMIT q->movableChanged();
}

void LingmoUIWindow::Private::setResizable(bool set)
{
    if (resizable == set) {
        return;
    }
    resizable = set;
    Q_EMIT q->resizableChanged();
}

void LingmoUIWindow::Private::setVirtualDesktopChangeable(bool set)
{
    if (virtualDesktopChangeable == set) {
        return;
    }
    virtualDesktopChangeable = set;
    Q_EMIT q->virtualDesktopChangeableChanged();
}

void LingmoUIWindow::Private::setAcceptFocus(bool set)
{
    if (acceptFocus == set) {
        return;
    }
    acceptFocus = set;
    Q_EMIT q->acceptFocusChanged();
}

void LingmoUIWindow::Private::setModality(bool set)
{
    if (modality == set) {
        return;
    }
    modality = set;
    Q_EMIT q->modalityChanged();
}

LingmoUIWindow::Private::Private(lingmo_window *w, quint32 internalId, const char *uuid, LingmoUIWindow *q)
    : internalId(internalId)
    , uuid(uuid)
    , q(q)
{
    Q_ASSERT(!this->uuid.isEmpty());
    window.setup(w);
    lingmo_window_add_listener(w, &s_listener, this);
}

LingmoUIWindow::LingmoUIWindow(LingmoUIWindowManagement *parent, lingmo_window *window, quint32 internalId, const char *uuid)
    : QObject(parent)
    , d(new Private(window, internalId, uuid, this))
{
}

LingmoUIWindow::~LingmoUIWindow()
{
    release();
}

void LingmoUIWindow::destroy()
{
    d->window.destroy();
}

void LingmoUIWindow::release()
{
    d->window.release();
}

bool LingmoUIWindow::isValid() const
{
    return d->window.isValid();
}

LingmoUIWindow::operator lingmo_window *() const
{
    return d->window;
}

LingmoUIWindow::operator lingmo_window *()
{
    return d->window;
}

QString LingmoUIWindow::appId() const
{
    return d->appId;
}

quint32 LingmoUIWindow::pid() const
{
    return d->pid;
}

QString LingmoUIWindow::title() const
{
    return d->title;
}


quint32 LingmoUIWindow::virtualDesktop() const
{
    return d->desktop;
}

bool LingmoUIWindow::isActive() const
{
    return d->active;
}

bool LingmoUIWindow::isFullscreen() const
{
    return d->fullscreen;
}

bool LingmoUIWindow::isKeepAbove() const
{
    return d->keepAbove;
}

bool LingmoUIWindow::isKeepBelow() const
{
    return d->keepBelow;
}

bool LingmoUIWindow::isMaximized() const
{
    return d->maximized;
}

bool LingmoUIWindow::isMinimized() const
{
    return d->minimized;
}

bool LingmoUIWindow::isOnAllDesktops() const
{
    if (lingmo_window_get_version(d->window) < 8) {
        return d->onAllDesktops;
    } else {
        return d->lingmoVirtualDesktops.isEmpty();
    }
}

bool LingmoUIWindow::isDemandingAttention() const
{
    return d->demandsAttention;
}

bool LingmoUIWindow::isCloseable() const
{
    return d->closeable;
}

bool LingmoUIWindow::isFullscreenable() const
{
    return d->fullscreenable;
}

bool LingmoUIWindow::isMaximizeable() const
{
    return d->maximizeable;
}

bool LingmoUIWindow::isMinimizeable() const
{
    return d->minimizeable;
}

bool LingmoUIWindow::skipTaskbar() const
{
    return d->skipTaskbar;
}

bool LingmoUIWindow::skipSwitcher() const
{
    return d->skipSwitcher;
}

QIcon LingmoUIWindow::icon() const
{
    return d->icon;
}

bool LingmoUIWindow::isShadeable() const
{
    return d->shadeable;
}

bool LingmoUIWindow::isShaded() const
{
    return d->shaded;
}

bool LingmoUIWindow::isResizable() const
{
    return d->resizable;
}

bool LingmoUIWindow::isMovable() const
{
    return d->movable;
}

bool LingmoUIWindow::isVirtualDesktopChangeable() const
{
    return d->virtualDesktopChangeable;
}

QString LingmoUIWindow::applicationMenuObjectPath() const
{
    return d->applicationMenuObjectPath;
}

QString LingmoUIWindow::applicationMenuServiceName() const
{
    return d->applicationMenuServiceName;
}

void LingmoUIWindow::requestActivate()
{
    lingmo_window_set_state(d->window, LINGMO_WINDOW_STATE_ACTIVE, LINGMO_WINDOW_STATE_ACTIVE);
}

void LingmoUIWindow::requestClose()
{
    lingmo_window_close(d->window);
}

void LingmoUIWindow::requestMove()
{
    lingmo_window_request_move(d->window);
}

void LingmoUIWindow::requestResize()
{
    lingmo_window_request_resize(d->window);
}

void LingmoUIWindow::requestVirtualDesktop(quint32 desktop)
{
    //not used
}

void LingmoUIWindow::requestToggleKeepAbove()
{
    if (d->keepAbove) {
        lingmo_window_set_state(d->window, LINGMO_WINDOW_STATE_KEEP_ABOVE, 0);
    } else {
        lingmo_window_set_state(d->window, LINGMO_WINDOW_STATE_KEEP_ABOVE, LINGMO_WINDOW_STATE_KEEP_ABOVE);
    }
}

void LingmoUIWindow::requestDemandAttention()
{
    if (d->demandsAttention) {
        lingmo_window_set_state(d->window, LINGMO_WINDOW_STATE_DEMANDS_ATTENTION, 0);
    } else {
        lingmo_window_set_state(d->window, LINGMO_WINDOW_STATE_DEMANDS_ATTENTION, LINGMO_WINDOW_STATE_DEMANDS_ATTENTION);
    }
}

void LingmoUIWindow::requestToggleKeepBelow()
{
    if (d->keepBelow) {
        lingmo_window_set_state(d->window, LINGMO_WINDOW_STATE_KEEP_BELOW, 0);
    } else {
        lingmo_window_set_state(d->window, LINGMO_WINDOW_STATE_KEEP_BELOW, LINGMO_WINDOW_STATE_KEEP_BELOW);
    }
}

void LingmoUIWindow::requestToggleMinimized()
{
    if (d->minimized) {
        lingmo_window_set_state(d->window, LINGMO_WINDOW_STATE_MINIMIZED, 0);
    } else {
        lingmo_window_set_state(d->window, LINGMO_WINDOW_STATE_MINIMIZED, LINGMO_WINDOW_STATE_MINIMIZED);
    }
}

void LingmoUIWindow::requestToggleMaximized()
{
    if (d->maximized) {
        lingmo_window_set_state(d->window, LINGMO_WINDOW_STATE_MAXIMIZED, 0);
    } else {
        lingmo_window_set_state(d->window, LINGMO_WINDOW_STATE_MAXIMIZED, LINGMO_WINDOW_STATE_MAXIMIZED);
    }
}

void LingmoUIWindow::setStartupGeometry(Surface *surface, const QRect &geometry)
{
    lingmo_window_set_startup_geometry(d->window, *surface, geometry.x(),geometry.y(),geometry.width(),geometry.height());
}

void LingmoUIWindow::setMinimizedGeometry(Surface *panel, const QRect &geom)
{
    lingmo_window_set_minimized_geometry(d->window, *panel, geom.x(), geom.y(), geom.width(), geom.height());
}

void LingmoUIWindow::unsetMinimizedGeometry(Surface *panel)
{
    lingmo_window_unset_minimized_geometry(d->window, *panel);
}

void LingmoUIWindow::requestToggleShaded()
{
    if (d->shaded) {
        lingmo_window_set_state(d->window, LINGMO_WINDOW_STATE_SHADED, 0);
    } else {
        lingmo_window_set_state(d->window, LINGMO_WINDOW_STATE_SHADED, LINGMO_WINDOW_STATE_SHADED);
    }
}


quint32 LingmoUIWindow::internalId() const
{
    return d->internalId;
}

QByteArray LingmoUIWindow::uuid() const
{
    return d->uuid;
}

QPointer<LingmoUIWindow> LingmoUIWindow::parentWindow() const
{
    return d->parentWindow;
}

QRect LingmoUIWindow::geometry() const
{
    return d->geometry;
}

void LingmoUIWindow::requestEnterVirtualDesktop(const QString &id)
{
    lingmo_window_request_enter_virtual_desktop(d->window, id.toUtf8());
}

void LingmoUIWindow::requestEnterNewVirtualDesktop()
{
    lingmo_window_request_enter_new_virtual_desktop(d->window);
}

void LingmoUIWindow::requestLeaveVirtualDesktop(const QString &id)
{
    lingmo_window_request_leave_virtual_desktop(d->window, id.toUtf8());
}

QStringList LingmoUIWindow::lingmoVirtualDesktops() const
{
    return d->lingmoVirtualDesktops;
}

void LingmoUIWindow::requestEnterActivity(const QString &id)
{
    lingmo_window_request_enter_activity(d->window, id.toUtf8());
}

void LingmoUIWindow::requestLeaveActivity(const QString &id)
{
    lingmo_window_request_leave_activity(d->window, id.toUtf8());
}

QStringList LingmoUIWindow::lingmoActivities() const
{
    return d->lingmoActivities;
}

void LingmoUIWindow::sendToOutput(KWayland::Client::Output *output) const
{
    if (lingmo_window_get_version(d->window) >= LINGMO_WINDOW_SEND_TO_OUTPUT_SINCE_VERSION) {
        lingmo_window_send_to_output(d->window, *output);
    }
}

void LingmoUIWindow::setHighlight()
{
    if(d->highlight)
        return;

    lingmo_window_highlight(d->window);
    d->highlight = true;
}

void LingmoUIWindow::unsetHightlight()
{
    if(!d->highlight)
        return;

    lingmo_window_unset_highlight(d->window);
    d->highlight = false;
}

bool LingmoUIWindow::isHighlight()
{
    return d->highlight;
}




