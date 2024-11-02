#include "lingmowaylandinterface.h"

#include <QDebug>
#include <QApplication>
#include "lingmo-window-management-client-protocol.h"
#include "lingmo-shell-client-protocol.h"
#include "wayland-xdg-activation-v1-client-protocol.h"
#include "wayland-ext-idle-notify-v1-client-protocol.h"

static const u_int32_t lingmo_window_management_surpported_version = 1;
static const u_int32_t lingmo_shell_surpported_version = 1;
static const u_int32_t wayland_xdg_activation_surpported_version = 1;
static const u_int32_t wayland_ext_idle_notify_surpported_version = 1;

void handle_global(void *data,struct wl_registry *wl_registry,uint32_t name, const char *interface,  uint32_t version)
{
    auto waylandInterface = reinterpret_cast<LingmoUIWaylandInterface *>(data);
    Q_ASSERT(wl_registry == *waylandInterface->registry());
    waylandInterface->handleGlobal(data,wl_registry,name,interface,version);
}
void handle_global_remove(void *data, struct wl_registry *wl_registry, uint32_t name)
{

}

const struct wl_registry_listener LingmoUIWaylandInterface::s_registryListener = {handle_global, handle_global_remove};

LingmoUIWaylandInterface::LingmoUIWaylandInterface(QObject *parent)
    : AbstractInterface(parent)
{
    m_connection = KWayland::Client::ConnectionThread::fromApplication(qApp);
    m_registry = new Registry(this);
    m_registry->create(m_connection->display());

    connect(m_registry, &KWayland::Client::Registry::shellAnnounced, this, [=](){
        const auto interface = m_registry->interface(KWayland::Client::Registry::Interface::Shell);
        if (interface.name != 0) {
            m_shell = m_registry->createShell(interface.name, interface.version, this);
        }
    });

    wl_registry* registry = *m_registry;
    wl_registry_add_listener(registry, &s_registryListener, this);
    wl_display_dispatch(m_connection->display());
    wl_display_roundtrip(m_connection->display());

    m_registry->setup();
//    m_connection->roundtrip();
}

LingmoUIWaylandInterface::~LingmoUIWaylandInterface()
{

}

WindowInfo LingmoUIWaylandInterface::requestInfo(WindowId wid)
{
    WindowInfo windowInfo;

    auto w = windowFor(wid);

    if (w)
    {
        windowInfo.setIsValid(true);
        windowInfo.setWid(wid);
        windowInfo.setIsActive(w->isActive());
        windowInfo.setIsMinimized(w->isMinimized());
        windowInfo.setIsMaxVert(w->isMaximized());
        windowInfo.setIsMaxHoriz(w->isMaximized());
        windowInfo.setIsFullscreen(w->isFullscreen());
        windowInfo.setIsShaded(w->isShaded());
        windowInfo.setIsOnAllDesktops(w->isOnAllDesktops());
        windowInfo.setIsOnAllActivities(true);
        windowInfo.setHasSkipTaskbar(w->skipTaskbar());
        windowInfo.setHasSkipSwitcher(w->skipSwitcher());
        windowInfo.setIsKeepAbove(w->isKeepAbove());
        //! Window Abilities
        windowInfo.setIsClosable(w->isCloseable());
        windowInfo.setIsFullScreenable(w->isFullscreenable());
        windowInfo.setIsMaximizable(w->isMaximizeable());
        windowInfo.setIsMinimizable(w->isMinimizeable());
        windowInfo.setIsMovable(w->isMovable());
        windowInfo.setIsResizable(w->isResizable());
        windowInfo.setIsShadeable(w->isShadeable());
        windowInfo.setIsVirtualDesktopsChangeable(w->isVirtualDesktopChangeable());
    }
    else
    {
        windowInfo.setIsValid(false);
    }

    return windowInfo;
}

void LingmoUIWaylandInterface::requestActivate(WindowId wid)
{
    auto w = windowFor(wid);

    if (w) {
        w->requestActivate();
        m_connection->roundtrip();
        emit windowChanged(w->uuid());
    }
}

void LingmoUIWaylandInterface::requestClose(WindowId wid)
{
    auto w = windowFor(wid);

    if (w) {
        w->requestClose();
        m_connection->roundtrip();
    }
}

void LingmoUIWaylandInterface::requestToggleKeepAbove(WindowId wid)
{
    auto w = windowFor(wid);

    if (w) {
        w->requestToggleKeepAbove();
        m_connection->roundtrip();
        emit windowChanged(w->uuid());
    }
}

void LingmoUIWaylandInterface::requestToggleMinimized(WindowId wid)
{
    auto w = windowFor(wid);

    if (w) {
        w->requestToggleMinimized();
        m_connection->roundtrip();
    }
}

void LingmoUIWaylandInterface::requestToggleMaximized(WindowId wid)
{
    auto w = windowFor(wid);

    if (w) {
        w->requestToggleMaximized();
        m_connection->roundtrip();
    }
}

QIcon LingmoUIWaylandInterface::iconFor(WindowId wid)
{
    auto window = windowFor(wid);

    if (window) {
        return window->icon();
    }

    return QIcon();
}

QString LingmoUIWaylandInterface::titleFor(WindowId wid)
{
    auto window = windowFor(wid);

    if (window) {
        return window->title();
    }
    return QString();
}

QString LingmoUIWaylandInterface::windowGroupFor(WindowId wid)
{
    auto window = windowFor(wid);
    if (window)
    {
        m_connection->roundtrip();
        return window->appId();
    }
    else
        return QString();
}

quint32 LingmoUIWaylandInterface::pid(WindowId wid)
{
    quint32 pid = 0;
    auto window = windowFor(wid);
    if (window)
    {
        m_connection->roundtrip();
        return window->pid();
    }
    else
        return pid;
}

void LingmoUIWaylandInterface::showCurrentDesktop()
{
    if(m_lingmoWindowManager)
    {
        m_lingmoWindowManager->showDesktop();
        m_connection->roundtrip();
    }
}

void LingmoUIWaylandInterface::hideCurrentDesktop()
{
    if(m_lingmoWindowManager)
    {
        m_lingmoWindowManager->hideDesktop();
        m_connection->roundtrip();
    }
}

bool LingmoUIWaylandInterface::windowCanBeDragged(WindowId wid)
{
    auto w = windowFor(wid);

    if (w) {
        WindowInfo winfo = requestInfo(wid);
        return (winfo.isValid()
                && w->isMovable()
                && !winfo.isMinimized()
                && inCurrentDesktopActivity(winfo)
                && !winfo.isPlasmaDesktop());
    }

    return false;
}

bool LingmoUIWaylandInterface::windowCanBeMaximized(WindowId wid)
{
    auto w = windowFor(wid);

    if (w) {
        WindowInfo winfo = requestInfo(wid);
        return (winfo.isValid()
                && w->isMaximizeable()
                && !winfo.isMinimized()
                && inCurrentDesktopActivity(winfo)
                && !winfo.isPlasmaDesktop());
    }

    return false;
}

WindowId LingmoUIWaylandInterface::activeWindow()
{
    if (!m_lingmoWindowManager) {
        return 0;
    }
    m_connection->roundtrip();
    auto wid = m_lingmoWindowManager->activeWindow();
    return wid ? QVariant(wid->uuid()) : "";
}
bool LingmoUIWaylandInterface::removeHeaderBar(QWindow *window)
{
    if(!window)
        return false;
    if (!m_lingmoShell)
        return false;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return false;
    if(!m_surfaces.contains(window))
    {
        m_surfaces.insert(window,surface);
    }

    auto lingmoShellSurface = m_lingmoShell->createSurface(surface, window);
    if (!lingmoShellSurface)
        return false;
    if(!m_lingmoShellSurfaces.contains(window))
    {
        m_lingmoShellSurfaces.insert(window,lingmoShellSurface);
    }
    lingmoShellSurface->setSurfaceProperty(LingmoUIShellSurface::SurfaceProperty::NoTitleBar,1);

    window->installEventFilter(this);
    return true;
}

bool LingmoUIWaylandInterface::setWindowRadius(QWindow *window, int radius)
{
    if(!window || radius < 0)
        return false;

    if (!m_lingmoShell)
        return false;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return false;
    if(!m_surfaces.contains(window))
    {
        m_surfaces.insert(window,surface);
    }

    auto lingmoShellSurface = m_lingmoShell->createSurface(surface, window);
    if (!lingmoShellSurface)
        return false;
    if(!m_lingmoShellSurfaces.contains(window))
    {
        m_lingmoShellSurfaces.insert(window,lingmoShellSurface);
    }
    lingmoShellSurface->setSurfaceProperty(LingmoUIShellSurface::SurfaceProperty::WindowRadius,radius);

    window->installEventFilter(this);
    return true;

}

void LingmoUIWaylandInterface::activateWindow(QWindow *window1, QWindow *window2)
{
    if(!window1 || !window2)
        return;

    if(!m_seat)
        return;
    if(!m_xdgActivation)
        return;

    auto surface = KWayland::Client::Surface::fromWindow(window1);
    if (!surface)
        return;
    if(!m_surfaces.contains(window1))
    {
        m_surfaces.insert(window1,surface);
    }
    XdgActivationToken *token = m_xdgActivation->requestXdgActivationToken(m_seat,surface,0,"");
    connect(token, &XdgActivationToken::done,this,[=](const QString&str){
        auto surface2 = KWayland::Client::Surface::fromWindow(window2);
        if (!surface2)
            return;
        if(!m_surfaces.contains(window2))
        {
            m_surfaces.insert(window2,surface2);
        }
        m_xdgActivation->activateWindow(str, surface2);
    });
}

void LingmoUIWaylandInterface::setIdleInterval(int msec)
{
    if(!m_notifier || !m_seat)
        return;
     ExtIdleNotification* notification = m_notifier->requestExtIdleNotification(m_seat, msec);
     if(!notification)
         return;
     connect(notification,&ExtIdleNotification::idled,this,[=](){
         Q_EMIT idled();
     });
     connect(notification,&ExtIdleNotification::resumed,this,[=](){
         Q_EMIT resumed();
     });
}

QRect LingmoUIWaylandInterface::windowGeometry(const WindowId &windowId)
{
    LingmoUIWindow* w = windowFor(windowId);
    if(w)
        return w->geometry();
    else
        return QRect(0, 0, 0, 0);
}

void LingmoUIWaylandInterface::setPanelAutoHide(QWindow *window, bool autoHide)
{
    if(!window)
        return;
    if (!m_lingmoShell)
        return;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return;
    if(!m_surfaces.contains(window))
    {
        m_surfaces.insert(window,surface);
    }

    auto lingmoShellSurface = m_lingmoShell->createSurface(surface, window);
    if (!lingmoShellSurface)
        return;
    if(!m_lingmoShellSurfaces.contains(window))
    {
        m_lingmoShellSurfaces.insert(window,lingmoShellSurface);
    }
    lingmoShellSurface->setPanelAutoHide(autoHide);


    window->installEventFilter(this);
}

void LingmoUIWaylandInterface::setGrabKeyboard(QWindow *window, bool autoHide)
{
    Q_UNUSED(autoHide)
    if(!window)
        return;
    if (!m_lingmoShell)
        return;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return;
    if(!m_surfaces.contains(window))
    {
        m_surfaces.insert(window,surface);
    }

    auto lingmoShellSurface = m_lingmoShell->createSurface(surface, window);
    if (!lingmoShellSurface)
        return;
    if(!m_lingmoShellSurfaces.contains(window))
    {
        m_lingmoShellSurfaces.insert(window,lingmoShellSurface);
    }
    lingmoShellSurface->setGrabKeyboard(m_seat);
    window->installEventFilter(this);
}

void LingmoUIWaylandInterface::setWindowLayer(QWindow *window, WindowLayer layer)
{
    if(!window)
        return;
    if (!m_lingmoShell)
        return;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return;
    if(!m_surfaces.contains(window))
    {
        m_surfaces.insert(window,surface);
    }

    auto lingmoShellSurface = m_lingmoShell->createSurface(surface, window);
    if (!lingmoShellSurface)
        return;
    if(!m_lingmoShellSurfaces.contains(window))
    {
        m_lingmoShellSurfaces.insert(window,lingmoShellSurface);
    }
    switch(layer)
    {
        case WindowLayer::Desktop:
            lingmoShellSurface->setRole(LingmoUIShellSurface::Role::Desktop);
            break;
        case WindowLayer::Panel:
            lingmoShellSurface->setRole(LingmoUIShellSurface::Role::Panel);
            break;
        case WindowLayer::OnScreenDisplay:
            lingmoShellSurface->setRole(LingmoUIShellSurface::Role::OnScreenDisplay);
            break;
        case WindowLayer::Notification:
            lingmoShellSurface->setRole(LingmoUIShellSurface::Role::Notification);
            break;
        case WindowLayer::ToolTip:
            lingmoShellSurface->setRole(LingmoUIShellSurface::Role::ToolTip);
            break;
        case WindowLayer::CriticalNotification:
            lingmoShellSurface->setRole(LingmoUIShellSurface::Role::CriticalNotification);
            break;
        case WindowLayer::AppletPop:
            lingmoShellSurface->setRole(LingmoUIShellSurface::Role::AppletPop);
            break;
        case WindowLayer::ScreenLock:
            lingmoShellSurface->setRole(LingmoUIShellSurface::Role::ScreenLock);
            break;
        case WindowLayer::Watermark:
            lingmoShellSurface->setRole(LingmoUIShellSurface::Role::Watermark);
            break;
        case WindowLayer::SystemWindow:
            lingmoShellSurface->setRole(LingmoUIShellSurface::Role::SystemWindow);
            break;
        case WindowLayer::InputPanel:
            lingmoShellSurface->setRole(LingmoUIShellSurface::Role::InputPanel);
            break;
        case WindowLayer::Logout:
            lingmoShellSurface->setRole(LingmoUIShellSurface::Role::Logout);
            break;
        case WindowLayer::ScreenLockNotification:
            lingmoShellSurface->setRole(LingmoUIShellSurface::Role::ScreenLockNotification);
            break;
        case WindowLayer::Switcher:
            lingmoShellSurface->setRole(LingmoUIShellSurface::Role::Switcher);
            break;
        default:
        {
            qWarning()<<"It is not a surpported window layer by lingmo protocols! id:" << (int)layer;
            lingmoShellSurface->setRole(LingmoUIShellSurface::Role::Normal);
            break;
        }
    }
    window->installEventFilter(this);
}

WindowLayer LingmoUIWaylandInterface::windowLayer(QWindow *window)
{
    if(!window)
        return WindowLayer::Normal;
    if (!m_lingmoShell)
        return WindowLayer::Normal;;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return WindowLayer::Normal;;
    if(!m_surfaces.contains(window))
    {
        m_surfaces.insert(window,surface);
    }

    auto lingmoShellSurface = m_lingmoShell->createSurface(surface, window);
    if (!lingmoShellSurface)
        return WindowLayer::Normal;;
    if(!m_lingmoShellSurfaces.contains(window))
    {
        m_lingmoShellSurfaces.insert(window,lingmoShellSurface);
    }
    window->installEventFilter(this);

    WindowLayer layer;
    switch(lingmoShellSurface->role())
    {
        case LingmoUIShellSurface::Role::Normal:
            layer = WindowLayer::Normal;
            break;
        case LingmoUIShellSurface::Role::Desktop:
            layer = WindowLayer::Desktop;
            break;
        case LingmoUIShellSurface::Role::Panel:
            layer = WindowLayer::Panel;
            break;
        case LingmoUIShellSurface::Role::OnScreenDisplay:
            layer = WindowLayer::OnScreenDisplay;
            break;
        case LingmoUIShellSurface::Role::Notification:
            layer = WindowLayer::Notification;
            break;
        case LingmoUIShellSurface::Role::ToolTip:
            layer = WindowLayer::ToolTip;
            break;
        case LingmoUIShellSurface::Role::CriticalNotification:
            layer = WindowLayer::CriticalNotification;
            break;
        case LingmoUIShellSurface::Role::AppletPop:
            layer = WindowLayer::AppletPop;
            break;
        case LingmoUIShellSurface::Role::ScreenLock:
            layer = WindowLayer::ScreenLock;
            break;
        case LingmoUIShellSurface::Role::Watermark:
            layer = WindowLayer::Watermark;
            break;
        case LingmoUIShellSurface::Role::SystemWindow:
            layer = WindowLayer::SystemWindow;
            break;
        case LingmoUIShellSurface::Role::InputPanel:
            layer = WindowLayer::InputPanel;
            break;
        case LingmoUIShellSurface::Role::Logout:
            layer = WindowLayer::Logout;
            break;
        case LingmoUIShellSurface::Role::ScreenLockNotification:
            layer = WindowLayer::ScreenLockNotification;
            break;
        case LingmoUIShellSurface::Role::Switcher:
            layer = WindowLayer::Switcher;
            break;
        default:
            layer = WindowLayer::Normal;
            break;
    }
    return layer;
}

void LingmoUIWaylandInterface::setHighlight(const WindowId &wid, bool highlight)
{
    auto w = windowFor(wid);

    if (w)
    {
        highlight ?  w->setHighlight() :  w->unsetHightlight();
        m_connection->roundtrip();
    }
}

bool LingmoUIWaylandInterface::istHighlight(const WindowId &wid)
{
    auto w = windowFor(wid);

    if (w)
    {
        return w->isHighlight();
    }
    else
        return false;
}

void LingmoUIWaylandInterface::setOpenUnderCursor(QWindow *window)
{
    if(!window)
        return;
    if (!m_lingmoShell)
        return;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return;
    if(!m_surfaces.contains(window))
    {
        m_surfaces.insert(window,surface);
    }

    auto lingmoShellSurface = m_lingmoShell->createSurface(surface, window);
    if (!lingmoShellSurface)
        return;
    if(!m_lingmoShellSurfaces.contains(window))
    {
        m_lingmoShellSurfaces.insert(window,lingmoShellSurface);
    }

    lingmoShellSurface->setOpenUnderCursor();

    window->installEventFilter(this);
}

void LingmoUIWaylandInterface::setOpenUnderCursor(QWindow *window, int x, int y)
{
    if(!window)
        return;
    if (!m_lingmoShell)
        return;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return;
    if(!m_surfaces.contains(window))
    {
        m_surfaces.insert(window,surface);
    }

    auto lingmoShellSurface = m_lingmoShell->createSurface(surface, window);
    if (!lingmoShellSurface)
        return;
    if(!m_lingmoShellSurfaces.contains(window))
    {
        m_lingmoShellSurfaces.insert(window,lingmoShellSurface);
    }

    lingmoShellSurface->setOpenUnderCursor(x, y);

    window->installEventFilter(this);
}

void LingmoUIWaylandInterface::setIconName(QWindow *window, const QString& iconName)
{
    if (!window || !m_lingmoShell)
        return;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return;
    if (!m_surfaces.contains(window))
    {
        m_surfaces.insert(window, surface);
    }

    auto lingmoShellSurface = m_lingmoShell->createSurface(surface, window);
    if (!lingmoShellSurface)
        return;
    if (!m_lingmoShellSurfaces.contains(window))
    {
        m_lingmoShellSurfaces.insert(window, lingmoShellSurface);
    }

    lingmoShellSurface->setIconName(iconName);

    window->installEventFilter(this);
}

QString LingmoUIWaylandInterface::currentSeatName()
{
    if (!m_lingmoShell)
        return QString();
    m_lingmoShell->updateCurrentOutput();
    wl_display_dispatch(m_connection->display());
    wl_display_roundtrip(m_connection->display());
    return m_lingmoShell->seatName();
}

QString LingmoUIWaylandInterface::currentOutputName()
{
    if (!m_lingmoShell)
        return QString();
    m_lingmoShell->updateCurrentOutput();
    wl_display_dispatch(m_connection->display());
    wl_display_roundtrip(m_connection->display());
    return m_lingmoShell->outputName();
}

QList<OutputInfo *> LingmoUIWaylandInterface::outputsInfo()
{
}

QList<WindowId> LingmoUIWaylandInterface::getWindowIdByPid(quint32 pid)
{
    QList<WindowId> list;
    wl_display_dispatch(m_connection->display());
    qDebug()<<m_lingmoWindowManager->windows().count();
    for(auto w : m_lingmoWindowManager->windows())
    {
        if(w->pid() == pid)
            list.append(w->uuid());
    }
    return list;
}

QList<WindowId> LingmoUIWaylandInterface::getWindowIdByTtile(const QString &title)
{
    QList<WindowId> list;
    for(auto w : m_lingmoWindowManager->windows())
    {
        if(w->title() == title)
            list.append(w->uuid());
    }
    return list;
}

void LingmoUIWaylandInterface::setGeometry(QWindow *window, const QRect &rect)
{
    if(!window)
        return;
    if (!m_lingmoShell)
        return;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return;
    if(!m_surfaces.contains(window))
    {
        m_surfaces.insert(window,surface);
    }

    auto lingmoShellSurface = m_lingmoShell->createSurface(surface, window);
    if (!lingmoShellSurface)
        return;
    if(!m_lingmoShellSurfaces.contains(window))
    {
        m_lingmoShellSurfaces.insert(window,lingmoShellSurface);
    }
    lingmoShellSurface->setPosition(rect.topLeft());
    window->resize(rect.size());


    window->installEventFilter(this);
}

void LingmoUIWaylandInterface::setSkipTaskBar(QWindow *window, bool skip)
{
    if(!window)
        return;
    if (!m_lingmoShell)
        return;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return;
    if(!m_surfaces.contains(window))
        m_surfaces.insert(window,surface);
    auto lingmoShellSurface = m_lingmoShell->createSurface(surface, window);
    if (!lingmoShellSurface)
        return;
    if(!m_lingmoShellSurfaces.contains(window))
        m_lingmoShellSurfaces.insert(window,lingmoShellSurface);

    lingmoShellSurface->setSkipTaskbar(skip);

    window->installEventFilter(this);
}

void LingmoUIWaylandInterface::setSkipSwitcher(QWindow *window, bool skip)
{
    if(!window)
        return;
    if (!m_lingmoShell)
        return;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return;
    if(!m_surfaces.contains(window))
        m_surfaces.insert(window,surface);
    auto lingmoShellSurface = m_lingmoShell->createSurface(surface, window);
    if (!lingmoShellSurface)
        return;
    if(!m_lingmoShellSurfaces.contains(window))
        m_lingmoShellSurfaces.insert(window,lingmoShellSurface);

    lingmoShellSurface->setSkipSwitcher(skip);
    window->installEventFilter(this);
}

bool LingmoUIWaylandInterface::skipTaskBar(const WindowId &wid)
{
    auto window = windowFor(wid);

    return window ? window->skipTaskbar() : false;
}

bool LingmoUIWaylandInterface::skipSwitcher(const WindowId &wid)
{
    auto window = windowFor(wid);

    return window ? window->skipSwitcher() : false;
}

bool LingmoUIWaylandInterface::isShowingDesktop()
{
    bool flag = false;
    if(m_lingmoWindowManager)
    {
        flag = m_lingmoWindowManager->isShowingDesktop();
    }
    return flag;
}

void LingmoUIWaylandInterface::setOnAllDesktops(const WindowId &wid)
{
    auto w = windowFor(wid);

    if (w && m_desktops.count() > 1)
    {
        if (w->isOnAllDesktops())
        {
            w->requestEnterVirtualDesktop(m_currentDesktop);
        }
    }
}

NET::WindowType LingmoUIWaylandInterface::windowType(WindowId wid)
{
    return NET::WindowType::Normal;
}

void LingmoUIWaylandInterface::setPanelTakefocus(QWindow *window, bool flag)
{
    if(!window)
        return;
    if (!m_lingmoShell)
        return;

    auto surface = KWayland::Client::Surface::fromWindow(window);
    if (!surface)
        return;
    if(!m_surfaces.contains(window))
    {
        m_surfaces.insert(window,surface);
    }

    auto lingmoShellSurface = m_lingmoShell->createSurface(surface, window);
    if (!lingmoShellSurface)
        return;
    if(!m_lingmoShellSurfaces.contains(window))
    {
        m_lingmoShellSurfaces.insert(window,lingmoShellSurface);
    }
    lingmoShellSurface->setPanelTakesFocus(flag);
    window->installEventFilter(this);
}

void LingmoUIWaylandInterface::demandAttention(const WindowId &wid)
{
    auto w = windowFor(wid);

    if (w) {
        w->requestDemandAttention();
        m_connection->roundtrip();
    }
}

bool LingmoUIWaylandInterface::eventFilter(QObject *obj, QEvent *ev)
{
    auto window = qobject_cast<QWindow*>(obj);
    if(window && ev->type() == QEvent::Hide)
    {
        if(m_lingmoShellSurfaces.contains(window))
        {
            auto lingmoShellSurface = m_lingmoShellSurfaces.value(window);
            if(lingmoShellSurface)
            {
                lingmoShellSurface->release();
                lingmoShellSurface->destroy();
            }
            m_lingmoShellSurfaces.remove(window);
        }
        if(m_surfaces.contains(window))
        {
            auto surface = m_surfaces.value(window);
            if(surface)
            {
                surface->release();
                surface->destroy();
            }
            m_surfaces.remove(window);
        }
    }
    return QObject::eventFilter(obj,ev);
}

bool LingmoUIWaylandInterface::lingmoProtocolReady()
{
    return m_lingmoShell && m_lingmoWindowManager;
}

Registry *LingmoUIWaylandInterface::registry()
{
    return m_registry;
}

LingmoUIWindow *LingmoUIWaylandInterface::windowFor(WindowId wid)
{
    auto it = std::find_if(m_lingmoWindowManager->windows().constBegin(), m_lingmoWindowManager->windows().constEnd(), [&wid](LingmoUIWindow * w) noexcept {
            return w->isValid() && w->uuid() == wid;
    });

    if (it == m_lingmoWindowManager->windows().constEnd()) {
        return nullptr;
    }

    return *it;
}

void LingmoUIWaylandInterface::handleGlobal(void *data, wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
    if(strcmp(interface, ext_idle_notifier_v1_interface.name) == 0)
    {
        u_int32_t supportedVersion = qMin(wayland_ext_idle_notify_surpported_version, version);
        ext_idle_notifier_v1* notifier = (ext_idle_notifier_v1*)wl_registry_bind(registry, name, &ext_idle_notifier_v1_interface, supportedVersion);
        if(notifier)
        {
            m_notifier = new ExtIdleNotifier(this);
            m_notifier->setup(notifier);
        }
    }
    if (strcmp(interface, wl_seat_interface.name) == 0)
    {
        m_seat =  (wl_seat *) wl_registry_bind(registry, name, &wl_seat_interface, version);

    }
    if (strcmp(interface, xdg_activation_v1_interface.name) == 0)
    {
        u_int32_t supportedVersion = qMin(wayland_xdg_activation_surpported_version, version);
        xdg_activation_v1 *xdg_activation =  (xdg_activation_v1 *) wl_registry_bind(registry, name, &xdg_activation_v1_interface, supportedVersion);
        if(xdg_activation)
        {
            m_xdgActivation = new XdgActivation(this);
            m_xdgActivation->setup(xdg_activation);
        }
    }
    if (strcmp(interface, lingmo_shell_interface.name) == 0)
    {
        u_int32_t supportedVersion = qMin(lingmo_shell_surpported_version, version);
        lingmo_shell *shell =  (lingmo_shell *) wl_registry_bind(registry, name, &lingmo_shell_interface, supportedVersion);
        if(shell)
        {
            m_lingmoShell = new LingmoUIShell(this);
            m_lingmoShell->setup(shell);
        }
    }
    if(strcmp(interface, lingmo_window_management_interface.name) == 0)
    {
        u_int32_t supportedVersion = qMin(lingmo_window_management_surpported_version, version);
        lingmo_window_management  *wm =
                (lingmo_window_management *) wl_registry_bind(registry, name, &lingmo_window_management_interface, supportedVersion);
        if(wm)
        {
            m_lingmoWindowManager = new LingmoUIWindowManagement(this);
            m_lingmoWindowManager->setup(wm);

            connect(m_lingmoWindowManager, &LingmoUIWindowManagement::showingDesktopChanged,this,&AbstractInterface::isShowingDesktopChanged);
            connect(m_lingmoWindowManager, &LingmoUIWindowManagement::windowCreated, this, &LingmoUIWaylandInterface::lingmoWindowCreatedProxy);
            connect(m_lingmoWindowManager, &LingmoUIWindowManagement::activeWindowChanged, this, [&]() noexcept
            {
                        LingmoUIWindow* w = m_lingmoWindowManager->activeWindow();
                        if(w)
                        {
                            emit activeWindowChanged(w ? w->uuid() : "");
                        }
            }, Qt::QueuedConnection);
            connect(m_lingmoWindowManager, &LingmoUIWindowManagement::windowCreated,this,
                [this](LingmoUIWindow *window)
            {
                if (!m_windows.contains(window->uuid()))
                {
                    m_windows.insert(window->uuid(), this->requestInfo(window->uuid()));
                }
                emit windowAdded(window->uuid());
            });
        }
    }
}

void LingmoUIWaylandInterface::handleGlobalRemove(void *data, wl_registry *registry, uint32_t name)
{
}

void LingmoUIWaylandInterface::lingmoWindowCreatedProxy(LingmoUIWindow *w)
{
    if (!w->isValid())
        return;

    if ((w->appId() == "lingmo-panel"))
        return;

    trackLingmoUIWindow(w);
}

void LingmoUIWaylandInterface::trackLingmoUIWindow(LingmoUIWindow *w)
{
    if(!w || w->appId() == "lingmo-panel")
        return;

    connect(w, &LingmoUIWindow::titleChanged, this, [=](){emit titleChanged(w->uuid());});
    connect(w, &LingmoUIWindow::fullscreenChanged, this, [=](){emit fullscreenChanged(w->uuid());});
    connect(w, &LingmoUIWindow::iconChanged, this, [=](){emit iconChanged(w->uuid());});
    connect(w, &LingmoUIWindow::activeChanged, this, [=](){emit activeChanged(w->uuid());});
    connect(w, &LingmoUIWindow::keepAboveChanged, this, [=](){emit keepAboveChanged(w->uuid());});
    connect(w, &LingmoUIWindow::minimizedChanged, this, [=](){emit minimizedChanged(w->uuid());});
    connect(w, &LingmoUIWindow::maximizedChanged, this, [=](){emit maximizedChanged(w->uuid());});
    connect(w, &LingmoUIWindow::onAllDesktopsChanged, this, [=](){emit onAllDesktopsChanged(w->uuid());});
    connect(w, &LingmoUIWindow::demandsAttentionChanged, this, [=](){emit demandsAttentionChanged(w->uuid());});
    connect(w, &LingmoUIWindow::skipTaskbarChanged, this, [=](){emit skipTaskbarChanged(w->uuid());});
    connect(w, &LingmoUIWindow::skipSwitcherChanged, this, [=](){emit skipSwitcherChanged(w->uuid());});
    connect(w, &LingmoUIWindow::geometryChanged, this, [=](){emit geometryChanged(w->uuid());});
}

void LingmoUIWaylandInterface::untrackLingmoUIWindow(LingmoUIWindow *w)
{
    disconnect(w, &LingmoUIWindow::titleChanged, 0, 0);
    disconnect(w, &LingmoUIWindow::iconChanged, 0, 0);
    disconnect(w, &LingmoUIWindow::activeChanged, 0, 0);
    disconnect(w, &LingmoUIWindow::keepAboveChanged, 0, 0);
    disconnect(w, &LingmoUIWindow::keepBelowChanged, 0, 0);
    disconnect(w, &LingmoUIWindow::minimizedChanged, 0, 0);
    disconnect(w, &LingmoUIWindow::maximizedChanged, 0, 0);
    disconnect(w, &LingmoUIWindow::onAllDesktopsChanged, 0, 0);
    disconnect(w, &LingmoUIWindow::demandsAttentionChanged, 0, 0);
    disconnect(w, &LingmoUIWindow::skipTaskbarChanged,  0, 0);
    disconnect(w, &LingmoUIWindow::skipSwitcherChanged, 0, 0);
    disconnect(w, &LingmoUIWindow::geometryChanged, 0, 0);
}
