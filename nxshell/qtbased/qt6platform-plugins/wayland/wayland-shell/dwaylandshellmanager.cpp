// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dwaylandshellmanager.h"
#include "dkeyboard.h"
#include "global.h"
#include "../xcb/utility.h"

#define protected public
#include <qwindow.h>
#undef protected

#include <QtWaylandClientVersion>
#include <QLoggingCategory>
#include <QPainterPath>

#ifndef QT_DEBUG
Q_LOGGING_CATEGORY(dwlp, "dtk.wayland.plugin" , QtInfoMsg);
#else
Q_LOGGING_CATEGORY(dwlp, "dtk.wayland.plugin");
#endif

DPP_USE_NAMESPACE

#define _DWAYALND_ "_d_dwayland_"
#define CHECK_PREFIX(key) (key.startsWith(_DWAYALND_) || key.startsWith("_d_"))
#define wlDisplay reinterpret_cast<wl_display *>(QGuiApplication::platformNativeInterface()->nativeResourceForWindow("display", nullptr))

Q_DECLARE_METATYPE(QPainterPath)

namespace QtWaylandClient {

namespace {
    // kwayland中PlasmaShell的全局对象，用于使用kwayland中的扩展协议
    PlasmaShell *kwayland_shell = nullptr;
    // kwin合成器提供的窗口边框管理器
    ServerSideDecorationManager *kwayland_ssd = nullptr;
    // 创建ddeshell
    DDEShell *ddeShell = nullptr;
    // kwayland
    Strut *kwayland_strut = nullptr;
    DDESeat *kwayland_dde_seat = nullptr;
    DDETouch *kwayland_dde_touch = nullptr;
    DDEPointer *kwayland_dde_pointer = nullptr;
    FakeInput *kwayland_dde_fake_input = nullptr;
    DDEKeyboard *kwayland_dde_keyboard = nullptr;
    BlurManager *kwayland_blur_manager = nullptr;
    Surface *kwayland_surface = nullptr;
    Compositor *kwayland_compositor = nullptr;
    PlasmaWindowManagement *kwayland_manage = nullptr;
};

QList<QPointer<QWaylandWindow>> DWaylandShellManager::send_property_window_list;

inline static wl_surface *getWindowWLSurface(QWaylandWindow *window)
{
#if QTWAYLANDCLIENT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return window->wlSurface();
#else
    return window->object();
#endif
}

static PlasmaShellSurface* createKWayland(QWaylandWindow *window)
{
    if (!window || !kwayland_shell)
        return nullptr;

    auto surface = window->shellSurface();
    return kwayland_shell->createSurface(getWindowWLSurface(window), surface);
}

static PlasmaShellSurface *ensureKWaylandSurface(QWaylandShellSurface *self)
{
    if (auto *ksurface = self->findChild<PlasmaShellSurface*>(QString(), Qt::FindDirectChildrenOnly)) {
        return ksurface;
    }

    return createKWayland(self->window());
}

static DDEShellSurface* createDDESurface(QWaylandShellSurface *shellSurface)
{
    if (!shellSurface || !ddeShell)
        return nullptr;

    return ddeShell->createShellSurface(getWindowWLSurface(shellSurface->window()), shellSurface);
}

static DDEShellSurface *ensureDDEShellSurface(QWaylandShellSurface *shellSurface)
{
    if (!shellSurface)
        return nullptr;

    if (auto *shell_surface = shellSurface->findChild<DDEShellSurface*>(QString(), Qt::FindDirectChildrenOnly)) {
        return shell_surface;
    }

    return createDDESurface(shellSurface);
}

static Surface *ensureSurface(QWaylandWindow *wlWindow)
{
    if (!kwayland_surface) {
        qCWarning(dwlp) << "invalid wayland surface";
        return nullptr;
    }
    if (!wlWindow->window()) {
        qCWarning(dwlp) << "invalid wlWindow";
        return nullptr;
    }
    return kwayland_surface->fromWindow(wlWindow->window());
}

static Blur *ensureBlur(Surface *surface, QObject *parent = nullptr)
{
    if (parent) {
        if (auto *blur = parent->findChild<Blur *>(QString(), Qt::FindDirectChildrenOnly))
            return blur;
    }
    if (!kwayland_blur_manager) {
        qCWarning(dwlp) << "invalid blur manager";
        return nullptr;
    }
    return kwayland_blur_manager->createBlur(surface, parent);
}

static Region *ensureRegion(QObject *parent = nullptr)
{
    if (parent) {
        if (auto *region = parent->findChild<Region *>(QString(), Qt::FindDirectChildrenOnly)) {
            return region;
        }
    }
    if (!kwayland_compositor) {
        qCWarning(dwlp) << "invalid wayland compositor";
        return nullptr;
    }
    return kwayland_compositor->createRegion(parent);
}

static void checkIsDWayland(const QString &function)
{
#ifndef D_DEEPIN_IS_DWAYLAND
    qCWarning(dwlp) << "This package is not compiled as dwayland, and [" << function << "] not support in this version.";
#endif
}

DWaylandShellManager::DWaylandShellManager()
    : m_registry (new Registry())
{

}

DWaylandShellManager::~DWaylandShellManager()
{

}

void DWaylandShellManager::sendProperty(QWaylandShellSurface *self, const QString &name, const QVariant &value)
{
    // 某些应用程序(比如日历，启动器)调用此方法时 self为空，导致插件崩溃
    if (Q_UNLIKELY(!self)) {
        return;
    }

    if (Q_UNLIKELY(!CHECK_PREFIX(name))) {
        HookCall(self, &QWaylandShellSurface::sendProperty, name, value);
        return;
    }

    QWaylandWindow *wlWindow = self->window();
    if (Q_UNLIKELY(!wlWindow)) {
        qCWarning(dwlp) << "Error, wlWindow is nullptr";
        return;
    }

    // 如果创建失败则说明kwaylnd_shell对象还未初始化，应当终止设置
    // 记录下本次的设置行为，kwayland_shell创建后会重新设置这些属性
    auto *ksurface = ensureKWaylandSurface(self);
    if (Q_UNLIKELY(!ksurface)) {
        send_property_window_list << wlWindow;
        return;
    }

    if (auto *dde_shell_surface = ensureDDEShellSurface(self)) {
        if (!name.compare(noTitlebar)) {
            qCDebug(dwlp()) << "Request NoTitleBar, value: " << value;
            dde_shell_surface->requestNoTitleBarProperty(value.toBool());
        }
        if (!name.compare(windowRadius)) {
            bool ok = false;
            qreal radius  = value.toInt(&ok);
            if (wlWindow->screen())
                radius *= wlWindow->screen()->devicePixelRatio();
            qCDebug(dwlp()) << "Rquest window radius, value: " << radius << value;
            if (ok)
                dde_shell_surface->requestWindowRadiusProperty({radius, radius});
            else
                qCWarning(dwlp) << "invalid property" << name << value;
        }
        if (!name.compare(splitWindowOnScreen)) {
            using KWayland::Client::DDEShellSurface;
            const auto tmp = value.toList();
            if (tmp.size() >= 2) {
                const auto &position = tmp[0].toInt();
                const auto &type = tmp[1].toInt();
                checkIsDWayland("requestSplitWindow()");
#ifdef D_DEEPIN_IS_DWAYLAND
                dde_shell_surface->requestSplitWindow(DDEShellSurface::SplitType(position), DDEShellSurface::SplitMode(type));
                qCDebug(dwlp) << "requestSplitWindow splitType: " << position << " mode: " << type;
#endif
            } else {
                qCWarning(dwlp) << "invalid property: " << name << value;
            }
            wlWindow->window()->setProperty(splitWindowOnScreen, 0);
        }
        if (!name.compare(supportForSplittingWindow)) {
            checkIsDWayland("getSplitable()");
#ifdef D_DEEPIN_IS_DWAYLAND
            wlWindow->window()->setProperty(supportForSplittingWindow, dde_shell_surface->getSplitable());
#endif
            return;
        }
        if (!name.compare(windowInWorkSpace)) {
            dde_shell_surface->requestOnAllDesktops(value.toBool());
            qCDebug(dwlp()) << "### requestOnAllDesktops" << name << value;
        }
        if (!name.compare(enableBlurWindow)) {
            qCDebug(dwlp) << "### enableBlurWindow" << name << value;
            if (!value.isValid()) {
                qCWarning(dwlp) << "invalid enableBlurWindow";
                return;
            }
            setEnableBlurWidow(wlWindow, value);
        }
        if (!name.compare(windowBlurAreas) || !name.compare(windowBlurPaths)) {
            qCDebug(dwlp) << "### requestWindowBlur" << name << value;
            updateWindowBlurAreasForWM(wlWindow, name, value);
        }
    }

    // 将popup的窗口设置为tooltop层级, 包括qmenu，combobox弹出窗口
    if (wlWindow->window()->type() == Qt::Popup)
        ksurface->setRole(PlasmaShellSurface::Role::ToolTip);

#ifdef D_DEEPIN_KWIN
    // 禁止窗口移动接口适配。
    typedef PlasmaShellSurface::Role KRole;
    if (!name.compare(enableSystemMove)) {
        ksurface->setRole(value.toBool() ? KRole::Normal : KRole::StandAlone);
        return;
    }

    if (QStringLiteral(_DWAYALND_ "global_keyevent") == name && value.toBool()) {
        if (wlWindow->findChild<DKeyboard*>(QString(), Qt::FindDirectChildrenOnly)) {
            return;
        }

        DKeyboard *keyboard = new DKeyboard(wlWindow);
        // 只有关心全局键盘事件才连接, 并且随窗口销毁而断开
        QObject::connect(kwayland_dde_keyboard, &DDEKeyboard::keyChanged,
                         keyboard, &DKeyboard::handleKeyEvent);
        QObject::connect(kwayland_dde_keyboard, &DDEKeyboard::modifiersChanged,
                         keyboard, &DKeyboard::handleModifiersChanged);
    }
#endif

    if (QStringLiteral(_DWAYALND_ "dockstrut") == name) {
        setDockStrut(self, value);
    }
    if (QStringLiteral(_DWAYALND_ "window-position") == name) {
        ksurface->setPosition(value.toPoint());
    }

    if (QStringLiteral(_DWAYALND_ "dock-appitem-geometry") == name) {
        setDockAppItemMinimizedGeometry(self, value);
    }

    static const QMap<PlasmaShellSurface::Role, QStringList> role2type = {
        {PlasmaShellSurface::Role::Normal, {"normal"}},
        {PlasmaShellSurface::Role::Desktop, {"desktop"}},
        {PlasmaShellSurface::Role::Panel, {"dock", "panel"}},
        {PlasmaShellSurface::Role::OnScreenDisplay, {"wallpaper", "onScreenDisplay"}},
        {PlasmaShellSurface::Role::Notification, {"notification"}},
        {PlasmaShellSurface::Role::ToolTip, {"tooltip"}},
    #ifdef D_DEEPIN_KWIN
        {PlasmaShellSurface::Role::StandAlone, {"launcher", "standAlone"}},
        {PlasmaShellSurface::Role::Override, {"session-shell", "menu", "wallpaper-set", "override"}},
    #endif
    };

    if (QStringLiteral(_DWAYALND_ "window-type") == name) {
        // 根据 type 设置对应的 role
        const QByteArray &type = value.toByteArray();
        for (int i = 0; i <= (int)PlasmaShellSurface::Role::ActiveFullScreen; ++i) {
            PlasmaShellSurface::Role role = PlasmaShellSurface::Role(i);
            if (role2type.value(role).contains(type)) {
                ksurface->setRole(role);
                if (type == "dock" || type == "panel") {
                    ksurface->setPanelBehavior(PlasmaShellSurface::PanelBehavior::AlwaysVisible);
                }
                break;
            }
        }
    }
    if (QStringLiteral(_DWAYALND_ "staysontop") == name) {
        setWindowStaysOnTop(self, value.toBool());
    }
}

void DWaylandShellManager::setGeometry(QPlatformWindow *self, const QRect &rect)
{
    HookCall(self, &QPlatformWindow::setGeometry, rect);
    if (self->QPlatformWindow::parent()) {
        return;
    }
    if (auto wl_window = static_cast<QWaylandWindow*>(self)) {
        wl_window->sendProperty(QStringLiteral(_DWAYALND_ "window-position"), rect.topLeft());
    }
}

void DWaylandShellManager::pointerEvent(const QPointF &pointF, QEvent::Type type)
{
    if (Q_UNLIKELY(!(type == QEvent::MouseButtonPress
                     || type == QEvent::MouseButtonRelease
                     || type == QEvent::Move))) {
        return;
    }
    // cursor()->pointerEvent 中只用到 event.globalPos(), 即 pointF 这个参数
    for (QScreen *screen : qApp->screens()) {
        if (screen && screen->handle() && screen->handle()->cursor()) {
            const QMouseEvent event(type, QPointF(), QPointF(), pointF, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            screen->handle()->cursor()->pointerEvent(event);
        }
    }
}

QWaylandShellSurface *DWaylandShellManager::createShellSurface(QWaylandShellIntegration *self, QWaylandWindow *window)
{
    auto surface = HookCall(self, &QWaylandShellIntegration::createShellSurface, window);

    HookOverride(surface, &QWaylandShellSurface::sendProperty, DWaylandShellManager::sendProperty);
    HookOverride(surface, &QWaylandShellSurface::wantsDecorations, DWaylandShellManager::disableClientDecorations);
    HookOverride(window, &QPlatformWindow::setGeometry, DWaylandShellManager::setGeometry);
    HookOverride(window, &QPlatformWindow::requestActivateWindow, DWaylandShellManager::requestActivateWindow);
    HookOverride(window, &QPlatformWindow::frameMargins, DWaylandShellManager::frameMargins);
    HookOverride(window, &QPlatformWindow::setWindowFlags, DWaylandShellManager::setWindowFlags);

    onShellSurfaceCreated(surface);

    // 设置窗口位置, 默认都需要设置，同时判断如果窗口并没有移动过，则不需要再设置位置，而是由窗管默认平铺显示
    bool bSetPosition = true;
    QWidgetWindow *widgetWin = static_cast<QWidgetWindow*>(window->window());
    if (widgetWin->inherits("QWidgetWindow") && widgetWin->widget()) {
        if (!widgetWin->widget()->testAttribute(Qt::WA_Moved)) {
            bSetPosition = false;
        }

        // 1. dabstractdialog 的 showevent 中会主动move到屏幕居中的位置, 即 setAttribute(Qt::WA_Moved)。
        // 2. 有 parent(ddialog dlg(this)) 的 window 窗管会主动调整位置，没有设置parent的才需要插件调整位置 如 ddialog dlg;
        if (window->transientParent() && !widgetWin->widget()->inherits("QMenu")) {
            bSetPosition = false;
        }
    }

    if (bSetPosition) {
        //QWaylandWindow对应surface的geometry，如果使用QWindow会导致缩放后surface坐标错误。
        window->sendProperty(_DWAYALND_ "window-position", window->geometry().topLeft());
    }

    for (const QByteArray &pname : widgetWin->dynamicPropertyNames()) {
        if (Q_LIKELY(!CHECK_PREFIX(pname)))
            continue;
        // 将窗口自定义属性记录到wayland window property中
        window->sendProperty(pname, widgetWin->property(pname.constData()));
    }

    //将拖拽图标窗口置顶，QShapedPixmapWindow是Qt中拖拽图标窗口专用类
    if (widgetWin->inherits("QShapedPixmapWindow")) {
        window->sendProperty(QStringLiteral(_DWAYALND_ "staysontop"), true);
    }

    return surface;
}

void DWaylandShellManager::createKWaylandShell(quint32 name, quint32 version)
{
    kwayland_shell = registry()->createPlasmaShell(name, version, registry()->parent());

    Q_ASSERT_X(kwayland_shell, "PlasmaShell", "Registry create PlasmaShell  failed.");

    for (QPointer<QWaylandWindow> lw_window : send_property_window_list) {
        if (!lw_window) {
            continue;
        }
        const QVariantMap &properites = lw_window->properties();
        // 当kwayland_shell被创建后，找到以_d_dwayland_开头的扩展属性将其设置一遍
        for (auto p = properites.cbegin(); p != properites.cend(); ++p) {
            if (CHECK_PREFIX(p.key()))
                sendProperty(lw_window->shellSurface(), p.key(), p.value());
        }
    }

    send_property_window_list.clear();
}

void DWaylandShellManager::createKWaylandSSD(quint32 name, quint32 version)
{
    kwayland_ssd = registry()->createServerSideDecorationManager(name, version, registry()->parent());
    Q_ASSERT_X(kwayland_ssd, "ServerSideDecorationManager", "KWayland Registry ServerSideDecorationManager failed.");
}

void DWaylandShellManager::createDDEShell(quint32 name, quint32 version)
{
    ddeShell = registry()->createDDEShell(name, version, registry()->parent());
    Q_ASSERT_X(ddeShell, "DDEShell", "Registry create DDEShell failed.");
}

void DWaylandShellManager::createDDESeat(quint32 name, quint32 version)
{
    kwayland_dde_seat = registry()->createDDESeat(name, version, registry()->parent());
    Q_ASSERT_X(kwayland_dde_seat, "DDESeat", "Registry create DDESeat failed.");
}

/*
 * @brief createStrut  创建dock区域，仅dock使用
 * @param registry
 * @param name
 * @param version
 */
void DWaylandShellManager::createStrut(quint32 name, quint32 version)
{
    kwayland_strut = registry()->createStrut(name, version, registry()->parent());
    Q_ASSERT_X(kwayland_strut, "strut", "Registry create strut failed.");
}

void DWaylandShellManager::createDDEKeyboard()
{
    //create dde keyboard
    Q_ASSERT(kwayland_dde_seat);

    kwayland_dde_keyboard = kwayland_dde_seat->createDDEKeyboard(registry()->parent());
    Q_ASSERT(kwayland_dde_keyboard);

    //刷新时间队列，等待kwin反馈消息
    if (wlDisplay){
        wl_display_roundtrip(wlDisplay);
    }
}

void DWaylandShellManager::createDDEFakeInput()
{
    kwayland_dde_fake_input = registry()->createFakeInput(registry()->interface(Registry::Interface::FakeInput).name,
                                                        registry()->interface(Registry::Interface::FakeInput).version);
    if (!kwayland_dde_fake_input || !kwayland_dde_fake_input->isValid()) {
        qCWarning(dwlp) << "fake input create failed.";
        return;
    }
    // 打开设置光标位置的开关
    kwayland_dde_fake_input->authenticate("dtk", QString("set cursor pos"));
}

void DWaylandShellManager::createDDEPointer()
{
    //create dde pointer
    Q_ASSERT(kwayland_dde_seat);

    kwayland_dde_pointer = kwayland_dde_seat->createDDePointer();
    Q_ASSERT(kwayland_dde_pointer);

    //向kwin发送获取全局坐标请求
    kwayland_dde_pointer->getMotion();

    //刷新时间队列，等待kwin反馈消息
    if (wlDisplay) {
        wl_display_roundtrip(wlDisplay);
    }

    //更新一次全局坐标
    pointerEvent(kwayland_dde_pointer->getGlobalPointerPos(), QEvent::Move);

    // mouse move
    static bool isTouchMotion = false;
    QObject::connect(kwayland_dde_pointer, &DDEPointer::motion,
            [] (const QPointF &posF) {
        if (isTouchMotion)
            return;
        pointerEvent(posF, QEvent::Move);
    });

    // 适配触屏
    // 1.25倍的缩放还是需要单独处理
    static QPointF releasePos;
    kwayland_dde_touch = kwayland_dde_seat->createDDETouch();
    QObject::connect(kwayland_dde_touch, &DDETouch::touchDown, [=] (int32_t kwaylandId, const QPointF &pos) {
        if (kwaylandId != 0) {
            return;
        }
        releasePos = pos;

        setCursorPoint(pos);
        pointerEvent(pos, QEvent::MouseButtonPress);
    });
    QObject::connect(kwayland_dde_touch, &DDETouch::touchMotion, [=] (int32_t kwaylandId, const QPointF &pos) {
        if (kwaylandId != 0) {
            return;
        }
        isTouchMotion = true;
        pointerEvent(pos, QEvent::Move);
        setCursorPoint(pos);
        releasePos = pos;
    });
    QObject::connect(kwayland_dde_touch, &DDETouch::touchUp, [=] (int32_t kwaylandId) {
        if (kwaylandId != 0) {
            return;
        }

        // 和 motion 的最后一个位置相等, 无需再更新
        if (isTouchMotion) {
            isTouchMotion = false;
            return;
        }

        setCursorPoint(releasePos);
        pointerEvent(releasePos, QEvent::MouseButtonRelease);
    });
}

void DWaylandShellManager::createBlurManager(quint32 name, quint32 version)
{
    kwayland_blur_manager = registry()->createBlurManager(name, version);
    if (!kwayland_blur_manager) {
        qCWarning(dwlp) << "kwayland_blur_manager create failed.";
        return;
    }
}

void DWaylandShellManager::createCompositor(quint32 name, quint32 version)
{
    kwayland_compositor = registry()->createCompositor(name, version);
    if (!kwayland_compositor) {
        qCWarning(dwlp) << "kwayland_compositor create failed.";
        return;
    }
}

void DWaylandShellManager::createSurface()
{
    if (!kwayland_compositor) {
        qCWarning(dwlp) << "kwayland_compositor is invalid.";
        return;
    }
    kwayland_surface = kwayland_compositor->createSurface();
    if (!kwayland_surface) {
        qCWarning(dwlp) << "kwayland_surface create failed.";
        return;
    }
}

void DWaylandShellManager::createPlasmaWindowManagement(KWayland::Client::Registry *registry, quint32 name, quint32 version)
{
    kwayland_manage = registry->createPlasmaWindowManagement(name, version, registry->parent());
}

void DWaylandShellManager::requestActivateWindow(QPlatformWindow *self)
{
    HookCall(self, &QPlatformWindow::requestActivateWindow);

    auto qwlWindow = static_cast<QWaylandWindow *>(self);
    if (self->QPlatformWindow::parent() || !ddeShell || !qwlWindow) {
        return;
    }

    if (auto *dde_shell_surface = ensureDDEShellSurface(qwlWindow->shellSurface())) {
        dde_shell_surface->requestActive();
    }
}

/*插件把qt自身的标题栏去掉了，使用的是窗管的标题栏，但是qtwaylandwindow每次传递坐标给kwin的时候，
* 都计算了（3, 30）的偏移，导致每次设置窗口属性的时候，窗口会下移,这个（3, 30）的偏移其实是qt自身
* 标题栏计算的偏移量，我们uos桌面不能带入这个偏移量
*/
QMargins DWaylandShellManager::frameMargins(QPlatformWindow *self)
{
    Q_UNUSED(self)

    return QMargins(0, 0, 0, 0);
}

void DWaylandShellManager::setWindowFlags(QPlatformWindow *self, Qt::WindowFlags flags)
{
    HookCall(self, &QPlatformWindow::setWindowFlags, flags);
    auto qwlWindow = static_cast<QWaylandWindow *>(self);
    if (!qwlWindow) {
        return;
    }
    qCDebug(dwlp) << "Qt::WindowStaysOnTopHint: " << flags.testFlag(Qt::WindowStaysOnTopHint);
    setWindowStaysOnTop(qwlWindow->shellSurface(), flags.testFlag(Qt::WindowStaysOnTopHint));
}

void DWaylandShellManager::handleGeometryChange(QWaylandShellSurface *shellSurface)
{
    auto ddeShellSurface = ensureDDEShellSurface(shellSurface);
    if (!ddeShellSurface || !shellSurface->window()) {
        return;
    }
    QObject::connect(ddeShellSurface, &DDEShellSurface::geometryChanged, shellSurface->window(),
                     [=] (const QRect &geom) {
        QRect newRect(geom.topLeft(), shellSurface->window()->geometry().size());
        QWindowSystemInterface::handleGeometryChange(shellSurface->window()->window(), newRect);
    });
}

typedef DDEShellSurface KCDFace;
Qt::WindowStates getwindowStates(KCDFace *surface)
{
    // handleWindowStateChanged 不能传 WindowActive 状态，单独处理
    Qt::WindowStates state = Qt::WindowNoState;
    if (surface->isMinimized())
        state = Qt::WindowMinimized;
    else if (surface->isFullscreen())
        state = Qt::WindowFullScreen;
    else if (surface->isMaximized())
        state = Qt::WindowMaximized;

    return state;
}

void DWaylandShellManager::handleWindowStateChanged(QWaylandShellSurface *shellSurface)
{
    if (!shellSurface) {
        qCWarning(dwlp) << "shellSurface is null";
        return;
    }
    QWaylandWindow *window = shellSurface->window();
    auto ddeShellSurface = ensureDDEShellSurface(shellSurface);
    if (!ddeShellSurface)
        return;

#define d_oldState QStringLiteral("_d_oldState")

#define STATE_CHANGED(sig)                                                                      \
    QObject::connect(ddeShellSurface, &KCDFace::sig, window, [window, ddeShellSurface](){       \
        qCDebug(dwlp) << "==== "#sig ;                                                          \
        const Qt::WindowStates &newState = getwindowStates(ddeShellSurface);                    \
        const int &oldState = window->property(d_oldState).toInt();                             \
        QWindowSystemInterface::handleWindowStateChanged(window->window(), newState, oldState); \
        window->setProperty(d_oldState, static_cast<int>(newState));                            \
    })

    window->setProperty(d_oldState, (int)getwindowStates(ddeShellSurface));

    STATE_CHANGED(minimizedChanged);
    STATE_CHANGED(maximizedChanged);
    STATE_CHANGED(fullscreenChanged);

    STATE_CHANGED(activeChanged);
    QObject::connect(ddeShellSurface, &KCDFace::activeChanged, window, [window, ddeShellSurface](){
        if (QWindow *w = ddeShellSurface->isActive() ? window->window() : nullptr)
#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
            QWindowSystemInterface::handleWindowActivated(w, Qt::FocusReason::ActiveWindowFocusReason);
#else
            QWindowSystemInterface::handleFocusWindowChanged(w, Qt::FocusReason::ActiveWindowFocusReason);
#endif
    });

#define SYNC_FLAG(sig, enableFunc, flag)                                                    \
    QObject::connect(ddeShellSurface, &KCDFace::sig, window, [window, ddeShellSurface](){   \
        qCDebug(dwlp) << "==== "#sig << (enableFunc);                                       \
        window->window()->setFlag(flag, enableFunc);                                        \
    })

    // SYNC_FLAG(keepAboveChanged, ddeShellSurface->isKeepAbove(), Qt::WindowStaysOnTopHint);
    QObject::connect(ddeShellSurface, &KCDFace::keepAboveChanged, window, [window, ddeShellSurface](){
        bool isKeepAbove = ddeShellSurface->isKeepAbove();
        qCDebug(dwlp) << "==== keepAboveChanged" << isKeepAbove;
        window->window()->setProperty(_DWAYALND_ "staysontop", isKeepAbove);
    });
    SYNC_FLAG(keepBelowChanged, ddeShellSurface->isKeepBelow(), Qt::WindowStaysOnBottomHint);
    SYNC_FLAG(minimizeableChanged, ddeShellSurface->isMinimizeable(), Qt::WindowMinimizeButtonHint);
    SYNC_FLAG(maximizeableChanged, ddeShellSurface->isMinimizeable(), Qt::WindowMaximizeButtonHint);
    SYNC_FLAG(closeableChanged, ddeShellSurface->isCloseable(), Qt::WindowCloseButtonHint);
    SYNC_FLAG(fullscreenableChanged, ddeShellSurface->isFullscreenable(), Qt::WindowFullscreenButtonHint);

    // TODO: not support yet
    //SYNC_FLAG(movableChanged, ddeShellSurface->isMovable(), Qt::??);
    //SYNC_FLAG(resizableChanged, ddeShellSurface->isResizable(), Qt::??);
    //SYNC_FLAG(acceptFocusChanged, ddeShellSurface->isAcceptFocus(), Qt::??);
    //SYNC_FLAG(modalityChanged, ddeShellSurface->isModal(), Qt::??);
}

/*
 * @brief setWindowStaysOnTop  设置窗口置顶
 * @param surface
 * @param state true:设置窗口置顶，false:取消置顶
 */
void DWaylandShellManager::setWindowStaysOnTop(QWaylandShellSurface *surface, const bool state)
{
    if (auto *dde_shell_surface = ensureDDEShellSurface(surface)) {
        dde_shell_surface->requestKeepAbove(state);
    }
}

/*
 * @brief setDockStrut  设置窗口有效区域
 * @param surface
 * @param var[0]:dock位置 var[1]:dock高度或者宽度 var[2]:start值 var[3]:end值
 */
void DWaylandShellManager::setDockStrut(QWaylandShellSurface *surface, const QVariant var)
{
    deepinKwinStrut dockStrut;
    switch (var.toList()[0].toInt()) {
    case 0:
        dockStrut.left = var.toList()[1].toInt();
        dockStrut.left_start_y = var.toList()[2].toInt();
        dockStrut.left_end_y = var.toList()[3].toInt();
        break;
    case 1:
        dockStrut.top = var.toList()[1].toInt();
        dockStrut.top_start_x = var.toList()[2].toInt();
        dockStrut.top_end_x = var.toList()[3].toInt();
        break;
    case 2:
        dockStrut.right = var.toList()[1].toInt();
        dockStrut.right_start_y = var.toList()[2].toInt();
        dockStrut.right_end_y = var.toList()[3].toInt();
        break;
    case 3:
        dockStrut.bottom = var.toList()[1].toInt();
        dockStrut.bottom_start_x = var.toList()[2].toInt();
        dockStrut.bottom_end_x = var.toList()[3].toInt();
        break;
    default:
        break;
    }

    kwayland_strut->setStrutPartial(getWindowWLSurface(surface->window()), dockStrut);
}

/*
 * @brief setDockAppItemMinimizedGeometry 设置驻留任务栏应用窗口最小化的位置信息
 * @param surface
 * @param var[0]:应用 窗口Id
 *        var[1]:app window x
 *        var[2]:app window y
 *        var[3]:app window width
 *        var[4]:app window height
 */
void DWaylandShellManager::setDockAppItemMinimizedGeometry(QWaylandShellSurface *surface, const QVariant var)
{
    if (!surface) {
        return;
    }

    for (auto w : kwayland_manage->windows()){
        if (w->windowId() == var.toList()[0].toUInt()) {
            auto x = var.toList()[1].toInt();
            auto y = var.toList()[2].toInt();
            auto width = var.toList()[3].toInt();
            auto height = var.toList()[4].toInt();

            QRect r(QPoint(x, y), QSize(width, height));

            if (w) {
                auto s = ensureSurface(surface->window());
                if (!s) {
                    qCWarning(dwlp) << "invalid surface";
                    return;
                }

                w->setMinimizedGeometry(s, r);
            }

            return;
        }
    }
}

void DWaylandShellManager::onShellSurfaceCreated(QWaylandShellSurface *shellSurface)
{
    handleGeometryChange(shellSurface);
    handleWindowStateChanged(shellSurface);
    createServerDecoration(shellSurface);
}

/*
 * @brief setCursorPoint  设置光标在屏幕中的绝对位置
 * @param pos
 */
void DWaylandShellManager::setCursorPoint(QPointF pos) {
    if (!kwayland_dde_fake_input) {
        qCCritical(dwlp) << "kwayland_dde_fake_input is nullptr";
        return;
    }
    if (!kwayland_dde_fake_input->isValid()) {
        qCCritical(dwlp) << "kwayland_dde_fake_input is invalid";
        return;
    }
    kwayland_dde_fake_input->requestPointerMoveAbsolute(pos);
}

void DWaylandShellManager::setEnableBlurWidow(QWaylandWindow *wlWindow, const QVariant &value)
{
    auto surface = ensureSurface(wlWindow);
    if (value.toBool()) {
        auto blur = ensureBlur(surface, surface);
        if (!blur) {
            qCWarning(dwlp) << "invalid blur";
            return;
        }
        auto region = ensureRegion(surface);
        if (!region) {
            qCWarning(dwlp) << "invalid region";
            return;
        }
        blur->setRegion(region);
        blur->commit();
        if (!kwayland_surface) {
            qCWarning(dwlp) << "invalid kwayland_surface";
            return;
        }
        kwayland_surface->commit(Surface::CommitFlag::None);
    } else {
        if (!kwayland_blur_manager) {
            qCWarning(dwlp) << "invalid kwayland_blur_manager";
            return;
        }
        kwayland_blur_manager->removeBlur(surface);
        if (!kwayland_surface) {
            qCWarning(dwlp) << "invalid kwayland_surface";
            return;
        }
        kwayland_surface->commit(Surface::CommitFlag::None);
        // 取消模糊效果的更新需要主动调用应用侧的窗口
        if (QWidgetWindow *widgetWin = static_cast<QWidgetWindow*>(wlWindow->window())) {
            if (auto widget = widgetWin->widget()) {
                widget->update();
            }
        }
    }
}

void DWaylandShellManager::updateWindowBlurAreasForWM(QWaylandWindow *wlWindow, const QString &name, const QVariant &value)
{
    if (!wlWindow->waylandScreen()) {
        return;
    }
    auto screen = wlWindow->waylandScreen()->screen();
    if (!screen) {
        return;
    }
    auto devicePixelRatio = screen->devicePixelRatio();

    auto surface = ensureSurface(wlWindow);
    if (!surface) {
        qCWarning(dwlp) << "invalid surface";
        return;
    }
    auto blur = ensureBlur(surface, surface);
    if (!blur) {
        qCWarning(dwlp) << "invalid blur";
        return;
    }
    auto region = kwayland_compositor->createRegion(surface);

    if (!name.compare(windowBlurAreas)) {
        const QVector<quint32> &tmpV = qvariant_cast<QVector<quint32>>(value);
        const QVector<Utility::BlurArea> &blurAreas = *(reinterpret_cast<const QVector<Utility::BlurArea>*>(&tmpV));
        if (blurAreas.isEmpty()) {
            qCWarning(dwlp) << "invalid BlurAreas";
            return;
        }
        for (auto ba : blurAreas) {
            ba *= devicePixelRatio;
            QPainterPath path;
            path.addRoundedRect(ba.x, ba.y, ba.width, ba.height, ba.xRadius, ba.yRaduis);
            region->add(path.toFillPolygon().toPolygon());
        }
    } else {
        const QList<QPainterPath> paths = qvariant_cast<QList<QPainterPath>>(value);
        if (paths.isEmpty()) {
            qCWarning(dwlp) << "invalid BlurPaths";
            return;
        }
        for (auto path : paths) {
            path *= devicePixelRatio;
            QPolygon polygon(path.toFillPolygon().toPolygon());
            region->add(polygon);
        }
    }
    blur->setRegion(region);
    blur->commit();
    kwayland_surface->commit(Surface::CommitFlag::None);
}

bool DWaylandShellManager::disableClientDecorations(QWaylandShellSurface *surface)
{
    Q_UNUSED(surface)

    // 禁用qtwayland自带的bradient边框（太丑了）
    return false;
}

void DWaylandShellManager::createServerDecoration(QWaylandShellSurface *shellSurface)
{
    if (!shellSurface) {
        qCWarning(dwlp) << "shellSurface is null";
        return;
    }
    QWaylandWindow *window = shellSurface->window();
    // 如果kwayland的server窗口装饰已转变完成，则为窗口创建边框
    if (!kwayland_ssd) {
        qDebug()<<"====kwayland_ssd creat failed";
        return;
    }

    // 通过窗口属性控制是否显示最小化和最大化按钮
    if (shellSurface) {
        auto *dde_shell_surface = ensureDDEShellSurface(shellSurface);
        if (dde_shell_surface) {
            if (!(window->window()->flags() & Qt::WindowMinimizeButtonHint)) {
                dde_shell_surface->requestMinizeable(false);
            }
            if (!(window->window()->flags() & Qt::WindowMaximizeButtonHint)) {
                dde_shell_surface->requestMaximizeable(false);
            }
            if ((window->window()->flags() & Qt::WindowStaysOnTopHint)) {
                dde_shell_surface->requestKeepAbove(true);
            }
            if ((window->window()->flags() & Qt::WindowDoesNotAcceptFocus)) {
                dde_shell_surface->requestAcceptFocus(false);
            }
            if (window->window()->modality() != Qt::NonModal) {
                dde_shell_surface->requestModal(true);
            }
        }
    }

    bool decoration = false;
    switch (window->window()->type()) {
        case Qt::Window:
        case Qt::Widget:
        case Qt::Dialog:
        case Qt::Tool:
        case Qt::Drawer:
            decoration = true;
            break;
        default:
            break;
    }
    if (window->window()->flags() & Qt::FramelessWindowHint)
        decoration = false;
    if (window->window()->flags() & Qt::BypassWindowManagerHint)
        decoration = false;

    qCDebug(dwlp) << "create decoration ?" << decoration;
    if (!decoration)
        return;

    auto *surface = getWindowWLSurface(window);

    if (!surface)
        return;

    // 创建由kwin server渲染的窗口边框对象
    if (auto ssd = kwayland_ssd->create(surface, shellSurface)) {
        ssd->requestMode(ServerSideDecoration::Mode::Server);
    }
}

}
