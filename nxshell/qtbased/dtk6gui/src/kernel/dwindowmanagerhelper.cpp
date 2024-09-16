// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dwindowmanagerhelper.h"
#include "dforeignwindow.h"

#include <DObjectPrivate>
#include <DGuiApplicationHelper>
#include <QGuiApplication>


#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    #include <qpa/qplatformwindow_p.h>
    #define D_XCB_WINDOW_TYPE QNativeInterface::Private::QXcbWindow::WindowType
#else
    #include <QtPlatformHeaders/QXcbWindowFunctions>
    #define D_XCB_WINDOW_TYPE QXcbWindowFunctions::WmWindowType
#endif


#include <qpa/qplatformwindow.h>
#include <qpa/qplatformnativeinterface.h>

#include <functional>

DGUI_BEGIN_NAMESPACE

#define DEFINE_CONST_CHAR(Name) const char _##Name[] = "_d_" #Name
#define MWM_FUNC_ALL (1L << 0)
#define MWM_DECOR_ALL (1L << 0)

// functions
DEFINE_CONST_CHAR(hasBlurWindow);
DEFINE_CONST_CHAR(hasComposite);
DEFINE_CONST_CHAR(hasNoTitlebar);
DEFINE_CONST_CHAR(hasWallpaperEffect);
DEFINE_CONST_CHAR(windowManagerName);
DEFINE_CONST_CHAR(connectWindowManagerChangedSignal);
DEFINE_CONST_CHAR(connectHasBlurWindowChanged);
DEFINE_CONST_CHAR(connectHasCompositeChanged);
DEFINE_CONST_CHAR(connectHasNoTitlebarChanged);
DEFINE_CONST_CHAR(connectHasWallpaperEffectChanged);
DEFINE_CONST_CHAR(getCurrentWorkspaceWindows);
DEFINE_CONST_CHAR(getWindows);
DEFINE_CONST_CHAR(windowFromPoint);
DEFINE_CONST_CHAR(connectWindowListChanged);
DEFINE_CONST_CHAR(setMWMFunctions);
DEFINE_CONST_CHAR(getMWMFunctions);
DEFINE_CONST_CHAR(setMWMDecorations);
DEFINE_CONST_CHAR(getMWMDecorations);
DEFINE_CONST_CHAR(connectWindowMotifWMHintsChanged);
DEFINE_CONST_CHAR(popupSystemWindowMenu);
DEFINE_CONST_CHAR(setWMClassName);

template<typename ReturnT, typename FunctionT, typename... Args>
static inline ReturnT callPlatformFunction(const QByteArray &funcName,  Args... args)
{
    QFunctionPointer func = Q_NULLPTR;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    func = qApp->platformFunction(funcName);
#endif

    return func ? reinterpret_cast<FunctionT>(func)(args...) : ReturnT();
}

typedef bool(*func_t)(QObject *object, std::function<void ()>);
static bool connectWindowManagerChangedSignal(QObject *object, std::function<void ()> slot)
{
    return callPlatformFunction<bool, func_t>(_connectWindowManagerChangedSignal, object, slot);
}

static bool connectHasBlurWindowChanged(QObject *object, std::function<void ()> slot)
{
    return callPlatformFunction<bool, func_t>(_connectHasBlurWindowChanged, object, slot);
}

static bool connectHasCompositeChanged(QObject *object, std::function<void ()> slot)
{
    return callPlatformFunction<bool, func_t>(_connectHasCompositeChanged, object, slot);
}

static bool connectHasNoTitlebarChanged(QObject *object, std::function<void ()> slot)
{
    return callPlatformFunction<bool, func_t>(_connectHasNoTitlebarChanged, object, slot);
}

static bool connectHasWallpaperEffectChanged(QObject *object, std::function<void ()> slot)
{
    return callPlatformFunction<bool, func_t>(_connectHasWallpaperEffectChanged, object, slot);
}

static bool connectWindowListChanged(QObject *object, std::function<void ()> slot)
{
    return callPlatformFunction<bool, func_t>(_connectWindowListChanged, object, slot);
}

static bool connectWindowMotifWMHintsChanged(QObject *object, std::function<void (quint32)> slot)
{
    typedef bool(*quint32_func_t)(QObject *object, std::function<void (quint32)>);
    return callPlatformFunction<bool, quint32_func_t>(_connectWindowMotifWMHintsChanged, object, slot);
}

class DWindowManagerHelperPrivate : public DTK_CORE_NAMESPACE::DObjectPrivate
{
public:
    explicit DWindowManagerHelperPrivate(DWindowManagerHelper *qq)
        : DObjectPrivate(qq) {}

    mutable QList<DForeignWindow *> windowList;
};

class DWindowManagerHelper_ : public DWindowManagerHelper {};
Q_GLOBAL_STATIC(DWindowManagerHelper_, wmhGlobal)

/*!
  \class Dtk::Gui::DWindowManagerHelper
  \inmodule dtkgui
  \brief 提供与窗口管理器交互的接口，同 \a DPlatformWindowHandle 依赖 dxcb 插件.

  dxcb 插件抽象出所有需要和X11平台交互的接口以供上层调用，DTK 使用插件中提供的接口再
  次封装提供给应用程序使用，从设计角度讲，DTK库中不应该直接使用任何跟平台相关的接口
  （如：X11、Wayland、Windows），在这样的结构支撑下，在一个新的平台上，只需要提供和
  dxcb 同样的接口，DTK应用即可无缝迁移。
  \sa {https://github.com/linuxdeepin/qt5dxcb-plugin/}{dxcb插件}
  \sa Dtk::Widget::DApplication::loadDXcbPlugin
  \sa Dtk::Widget::DApplication::isDXcbPlatform
  \sa Dtk::Widget::DPlatformWindowHandle
 */

/*!
  \property DWindowManagerHelper::hasBlurWindow
  \brief 窗口管理器是否支持窗口背景模糊特效
  \note 在 dxcb 插件中目前只支持 deepin-wm 和 kwin 这两种窗管的模糊特效
  \note 只读
 */

/*!
  \property DWindowManagerHelper::hasComposite
  \brief 窗口管理器是否支持混成效果。如果不支持混成，则表示所有窗口的背景都不能透明，
  随之而来也不会有窗口阴影等效果，不规则窗口的边缘也会存在锯齿。
  \note 只读
 */

/*!
  \property DWindowManagerHelper::hasNoTitlebar
  \brief 窗口管理器是否支持隐藏窗口标题栏。如果支持，则 DPlatformWindowHandle::enableDXcbForWindow
  会优先使用此方法支持自定义窗口标题栏。
  \note 只读
  \sa Dtk::Gui::DPlatformHandle::setEnabledNoTitlebarForWindow
 */

/*!
  \property DWindowManagerHelper::hasWallpaperEffect
  \brief 窗口管理器是否支持窗口背景特效绘制。如果支持，则 绘制背景到透明窗口
  会使用此方法开启特效窗口壁纸背景绘制。
  \note 只读
  \sa hasWallpaperEffectChanged()
 */

/*!
  \enum Dtk::Gui::DWindowManagerHelper::MotifFunction
  MotifFunction::MotifFunction 窗口管理器对窗口所能控制的行为

  \value FUNC_RESIZE
  控制窗口大小。如果存在此标志，则窗口管理器可以改变窗口大小（如使用鼠标拖拽窗口边缘），
  否则无法通过外部行为调整窗口大小。
  \code
  DMainWindow w;

  w.resize(400, 200);
  w.show();
  DWindowManagerHelper::setMotifFunctions(w.windowHandle(), DWindowManagerHelper::FUNC_RESIZE, false);
  \endcode
  \image disable_resize_function.gif
  \note 普通窗口默认存在此标志，对于 Qt::Popup 和 Qt::BypassWindowManagerHint
  类型的窗口，不受此标志位影响
  \note 设置此标志后也会影响窗口标题栏对应功能入口的状态
  \note 对于使用系统标题栏的窗口，此功能和具体窗口管理器实现相关，deepin-wm 中设置
  此标志无效。
  \sa Dtk::Gui::DPlatformHandle::enableDXcbForWindow
  \sa Dtk::Gui::DPlatformHandle::isEnabledDXcb

  \value FUNC_MOVE
  控制窗口位置。如果存在此标志，则窗口管理器可以移动窗口（如使用鼠标拖动标题栏），否则
  无法通过外部行为移动窗口位置。
  \code
  DWindowManagerHelper::setMotifFunctions(w.windowHandle(), DWindowManagerHelper::FUNC_MOVE, false);
  \endcode
  \image disable_move_function.gif

  \value FUNC_MINIMIZE
  最小化窗口。如果存在此标志，则窗口可以被最小化（如点击标题栏的最小化按钮），否则无法
  通过外部行为最小化窗口。
  \code
  DWindowManagerHelper::setMotifFunctions(w.windowHandle(), DWindowManagerHelper::FUNC_MINIMIZE, false);
  \endcode
  \note 设置此标志后也会影响窗口标题栏对应功能入口的状态

  \value FUNC_MAXIMIZE
  最大化窗口。如果存在此标志，则窗口可以被最大化（如点击标题栏的最大化按钮），否则无法
  通过外部行为最大化窗口。
  \code
  DWindowManagerHelper::setMotifFunctions(w.windowHandle(), DWindowManagerHelper::FUNC_MAXIMIZE, false);
  \endcode
  \note 设置此标志后也会影响窗口标题栏对应功能入口的状态

  \value FUNC_CLOSE
  关闭窗口。如果存在此标志，则窗口可以被关闭（如点击标题栏的关闭按钮或使用Alt+F4快捷键），
  否则无法通过外部行为关闭窗口。
  \code
  DWindowManagerHelper::setMotifFunctions(w.windowHandle(), DWindowManagerHelper::FUNC_CLOSE, false);
  \endcode
  \note 设置此标志后也会影响窗口标题栏对应功能入口的状态

  \value FUNC_ALL
  所有功能性行为
 */

/*!
  \enum Dtk::Gui::DWindowManagerHelper::MotifDecoration
  MotifFunction::MotifDecoration 窗口管理器对窗口添加的修饰。只影响窗口上对应功能
  的入口，不影响实际的功能，比如：禁用掉 FUNC_MAXIMIZE 后，还可以使用快捷键最大化窗口

  \value DECOR_BORDER
  窗口描边。如果存在此标志，则窗口管理器会为窗口绘制描边，否则窗口没有描边。
  否则无法通过外部行为调整窗口大小。
  \note 只支持使用系统标题栏的窗口，此功能和具体窗口管理器实现相关，deepin-wm 中设置
  此标志无效。

  \value DECOR_RESIZEH
  改变窗口大小。如果存在此标志，则窗口管理器会在窗口的修饰上显示一个更改窗口大小的控件，
  否则无此控件。
  \note 只支持使用系统标题栏的窗口，此功能和具体窗口管理器实现相关，deepin-wm 中设置
  此标志无效。

  \value DECOR_TITLE
  窗口标题。如果存在此标志，则窗口管理器会在窗口的修饰上显示窗口标题，否则不显示。
  \note 只支持使用系统标题栏的窗口，此功能和具体窗口管理器实现相关，deepin-wm 中设置
  此标志无效。

  \value DECOR_MENU
  窗口菜单。如果存在此标志，则窗口管理器会在窗口的修饰上显示一个窗口菜单控件，否则不显示。
  \note 只支持使用系统标题栏的窗口，此功能和具体窗口管理器实现相关，deepin-wm 中设置
  此标志无效。

  \value DECOR_MINIMIZE
  窗口最小化。如果存在此标志，则窗口管理器会在窗口的修饰上显示一个最小化窗口控件，否则不显示。
  \note 只支持使用系统标题栏的窗口，此功能和具体窗口管理器实现相关，deepin-wm 中设置
  此标志无效。
  \sa Qt::WindowMinimizeButtonHint

  \value DECOR_MAXIMIZE
  窗口最大化。如果存在此标志，则窗口管理器会在窗口的修饰上显示一个最大化窗口控件，否则不显示。
  \note 只支持使用系统标题栏的窗口，此功能和具体窗口管理器实现相关，deepin-wm 中设置
  此标志无效。
  \sa Qt::WindowMaximizeButtonHint

  \value DECOR_ALL
  所有窗口修饰。
 */

/*!
  \enum Dtk::Gui::DWindowManagerHelper::WMName
  DWindowManagerHelper::WMName 窗口管理器类型
  \value DeepinWM
  深度系统桌面环境窗口管理器

  \value KWinWM
  KDE系统桌面环境窗口管理器

  \value OtherWM
  其它窗口管理器
 */

/*!
  \fn void DWindowManagerHelper::windowManagerChanged()
  \brief 信号会在当前环境窗口管理器变化时被发送.
 */
/*!
  \fn void DWindowManagerHelper::hasBlurWindowChanged()
  \brief 信号会在 hasBlurWindow 属性的值改变时被发送.
 */
/*!
  \fn void DWindowManagerHelper::hasCompositeChanged()
  \brief 信号会在 hasComposite 属性的值改变时被发送.
 */
/*!
  \fn void DWindowManagerHelper::hasNoTitlebarChanged()
  \brief 信号会在 hasNoTitlebar 属性的值改变时被发送.
 */
/*!
  \fn void DWindowManagerHelper::hasWallpaperEffectChanged()
  \brief 信号会在 hasWallpaperEffect 属性的值改变时被发送.
 */
/*!
  \fn void DWindowManagerHelper::windowListChanged()
  \brief 信号会在当前环境本地窗口列表变化时被发送。包含打开新窗口、关闭窗口、改变窗口的
  层叠顺序.
 */
/*!
  \fn void DWindowManagerHelper::windowMotifWMHintsChanged(quint32 winId)
  \brief 信号会在窗口功能或修饰标志改变时被发送.

  \a winId 窗口id
  \note 只对当前应用程序中的窗口有效
 */

DWindowManagerHelper::~DWindowManagerHelper()
{
    D_DC(DWindowManagerHelper);

    for (QWindow *w : d->windowList) {
        w->deleteLater();
    }
}

/*!
  \brief DWindowManagerHelper::instance
  DWindowManagerHelper 的单例对象，使用 Q_GLOBAL_STATIC 定义，在第一次调用时实例化。
  \return
 */
DWindowManagerHelper *DWindowManagerHelper::instance()
{
    return wmhGlobal;
}

/*!
  \brief DWindowManagerHelper::setMotifFunctions
  设置窗口的功能性标志，会覆盖之前的设置
  \a window
  \a hints
 */
void DWindowManagerHelper::setMotifFunctions(const QWindow *window, MotifFunctions hints)
{
    QFunctionPointer setMWMFunctions = Q_NULLPTR;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    setMWMFunctions = qApp->platformFunction(_setMWMFunctions);
#endif

    if (setMWMFunctions && window->handle()) {
        /*
  fix bug: 18775, 3391
   当所有function标志都设置时,用MWM_FUNC_ALL代替会导致窗管无法正确处理 dock栏发送的关闭消息.所以取消此设置

     if (hints == FUNC_ALL)
         hints = (MotifFunction)MWM_FUNC_ALL;
    */

        reinterpret_cast<void(*)(quint32, quint32)>(setMWMFunctions)(window->handle()->winId(), (quint32)hints);
    }
}

/*!
  \brief DWindowManagerHelper::setMotifFunctions
  设置窗口某些标志位的开启状态，不影响其它标志位
  \a window
  \a hints 要设置的标志位
  \a on 如果值为 true 则开启标志，否则关闭
  \return 返回设置后的窗口标志
 */
DWindowManagerHelper::MotifFunctions DWindowManagerHelper::setMotifFunctions(const QWindow *window, MotifFunctions hints, bool on)
{
    MotifFunctions old_hints = getMotifFunctions(window);

    if (on)
        hints |= old_hints;
    else
        hints = old_hints & ~hints;

    setMotifFunctions(window, hints);

    return hints;
}

/*!
  \brief DWindowManagerHelper::getMotifFunctions
  \a window
  \return 返回窗口当前的功能标志
 */
DWindowManagerHelper::MotifFunctions DWindowManagerHelper::getMotifFunctions(const QWindow *window)
{
    QFunctionPointer getMWMFunctions = Q_NULLPTR;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    getMWMFunctions = qApp->platformFunction(_getMWMFunctions);
#endif

    if (getMWMFunctions && window->handle()) {
        quint32 hints = reinterpret_cast<quint32(*)(quint32)>(getMWMFunctions)(window->handle()->winId());

        if (!(hints & MWM_FUNC_ALL))
            return (MotifFunctions)hints;
    }

    return FUNC_ALL;
}

/*!
  \brief DWindowManagerHelper::setMotifDecorations
  设置窗口的修饰性标志，会覆盖之前的设置
  \a window
  \a hints
 */
void DWindowManagerHelper::setMotifDecorations(const QWindow *window, MotifDecorations hints)
{
    QFunctionPointer setMWMDecorations = Q_NULLPTR;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    setMWMDecorations = qApp->platformFunction(_setMWMDecorations);
#endif

    if (setMWMDecorations && window->handle()) {
        if (hints == DECOR_ALL)
            hints = (MotifDecoration)MWM_DECOR_ALL;

        reinterpret_cast<void(*)(quint32, quint32)>(setMWMDecorations)(window->handle()->winId(), (quint32)hints);
    }
}

/*!
  \brief DWindowManagerHelper::setMotifFunctions
  设置窗口某些标志位的开启状态，不影响其它标志位
  \a window
  \a hints 要设置的标志位
  \a on 如果值为 true 则开启标志，否则关闭
  \return 返回设置后的窗口标志
 */
DWindowManagerHelper::MotifDecorations DWindowManagerHelper::setMotifDecorations(const QWindow *window, MotifDecorations hints, bool on)
{
    MotifDecorations old_hints = getMotifDecorations(window);

    if (on)
        hints |= old_hints;
    else
        hints = old_hints & ~hints;

    setMotifDecorations(window, hints);

    return hints;
}

/*!
  \brief DWindowManagerHelper::getMotifFunctions
  \a window
  \return 返回窗口当前的修饰标志
 */
DWindowManagerHelper::MotifDecorations DWindowManagerHelper::getMotifDecorations(const QWindow *window)
{
    QFunctionPointer getMWMDecorations = Q_NULLPTR;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    getMWMDecorations = qApp->platformFunction(_getMWMDecorations);
#endif

    if (getMWMDecorations && window->handle()) {
        quint32 hints = reinterpret_cast<quint32(*)(quint32)>(getMWMDecorations)(window->handle()->winId());

        if (!(hints & MWM_DECOR_ALL))
            return (MotifDecorations)hints;
    }

    return DECOR_ALL;
}

/*!
  \brief DWindowManagerHelper::setWmWindowTypes
  直接设置窗口管理器层级提供的窗口类型，如DesktopType和DockType类型也被
  桌面环境需要，但是Qt自身并没有提供对应的设置接口
  \a window
  \a types
 */
void DWindowManagerHelper::setWmWindowTypes(QWindow *window, WmWindowTypes types)
{
    const int _types = static_cast<int>(types);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    typedef QNativeInterface::Private::QXcbWindow  QXcbWindow_P;
    if (auto w = dynamic_cast<QXcbWindow_P *>(window->handle())) {
        w->setWindowType(static_cast<D_XCB_WINDOW_TYPE>(_types));
    } else {
        qWarning() << "cast" << window << "to platform window failed";
    }
#else
    QXcbWindowFunctions::setWmWindowType(window, static_cast<D_XCB_WINDOW_TYPE>(_types));
#endif
}

/*!
  \brief DWindowManagerHelper::setWmClassName
  设置x11环境上默认使用的wm class name，主要是在窗口创建时用于设置WM_CLASS窗口属性
  \a name
  \note 如果值为空，Qt将在下次使用此值时根据程序名称再次初始化此值
  \sa QCoreApplication::applicationName
 */
void DWindowManagerHelper::setWmClassName(const QByteArray &name)
{
    typedef void (*SetWmNameType)(const QByteArray&);
    return callPlatformFunction<void, SetWmNameType>(_setWMClassName, name);
}

/*!
  \brief DWindowManagerHelper::popupSystemWindowMenu
  显示窗口管理器对窗口的菜单，和有边框的窗口在标题栏上点击鼠标右键弹出的菜单内容一致。
  在 DMainWindow 的标题栏上点击鼠标右键会调用此函数打开系统菜单：
  \image window_system_menu.gif
  \a window
 */
void DWindowManagerHelper::popupSystemWindowMenu(const QWindow *window)
{
    return callPlatformFunction<void, void(*)(quint32)>(_popupSystemWindowMenu, quint32(window->handle()->winId()));
}

/*!
  \brief DWindowManagerHelper::hasBlurWindow
  \return 如果当前窗口管理器支持窗口背景模糊特效则返回 true，否则返回 false
 */
bool DWindowManagerHelper::hasBlurWindow() const
{
    return callPlatformFunction<bool, bool(*)()>(_hasBlurWindow);
}

/*!
  \brief DWindowManagerHelper::hasComposite
  \return 如果当前窗口管理器支持混成则返回 true，否则返回 false
 */
bool DWindowManagerHelper::hasComposite() const
{
    QFunctionPointer hasComposite = Q_NULLPTR;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    hasComposite = qApp->platformFunction(_hasComposite);
#endif

    if (!hasComposite) {
#ifdef Q_OS_LINUX
        if (DGuiApplicationHelper::isXWindowPlatform()) {
            QPlatformNativeInterface *native = qApp->platformNativeInterface();
            if (Q_LIKELY(native)) {
                QScreen *scr = QGuiApplication::primaryScreen();
                return native->nativeResourceForScreen(QByteArray("compositingEnabled"), scr);
            }
        }
#endif
        // 在其它平台上默认认为混成是开启的
        return true;
    }

    return callPlatformFunction<bool, bool(*)()>(_hasComposite);
}

/*!
  \brief DWindowManagerHelper::hasNoTitlebar
  \return 如果窗口管理器当前支持设置隐藏窗口标题栏则返回 true，否则返回 false
 */
bool DWindowManagerHelper::hasNoTitlebar() const
{
    return callPlatformFunction<bool, bool(*)()>(_hasNoTitlebar);
}

/*!
  \brief DWindowManagerHelper::hasWallpaperEffect
  \return 如果窗口管理器当前支持背景图片特效绘制返回 true，否则返回 false
 */
bool DWindowManagerHelper::hasWallpaperEffect() const
{
    return callPlatformFunction<bool, bool(*)()>(_hasWallpaperEffect);
}

/*!
  \brief DWindowManagerHelper::windowManagerNameString
  \return 返回窗口管理器名称。在X11平台上，此值为窗口管理器对应窗口的 _NET_WM_NAME
  的值
  \ref {https://specifications.freedesktop.org/wm-spec/1.3/ar01s03.html}{_NET_SUPPORTING_WM_CHECK}
  \ref {https://specifications.freedesktop.org/wm-spec/1.3/ar01s05.html}{_NET_WM_NAME}
 */
QString DWindowManagerHelper::windowManagerNameString() const
{
    return callPlatformFunction<QString, QString(*)()>(_windowManagerName);
}

/*!
  \brief DWindowManagerHelper::windowManagerName
  \return 返回当前窗口管理器类型
  \sa DWindowManagerHelper::windowManagerNameString
 */
DWindowManagerHelper::WMName DWindowManagerHelper::windowManagerName() const
{
    const QString &wmName = windowManagerNameString();

    if (wmName == QStringLiteral("Mutter(DeepinGala)")) {
        return DeepinWM;
    }

    if (wmName == QStringLiteral("KWin")) {
        return KWinWM;
    }

    return OtherWM;
}

/*!
  \brief DWindowManagerHelper::allWindowIdList
  \return 返回当前环境所有本地窗口的窗口id列表
  \note 顺序和窗口层叠顺序相关，显示越靠下层的窗口在列表中顺序越靠前
  \sa DWindowManagerHelper::currentWorkspaceWindowIdList
 */
QVector<quint32> DWindowManagerHelper::allWindowIdList() const
{
    return callPlatformFunction<QVector<quint32>, QVector<quint32>(*)()>(_getWindows);
}

/*!
  \brief DWindowManagerHelper::currentWorkspaceWindowIdList
  \return 返回当前工作区所有本地窗口的窗口id列表
  \note 顺序和窗口层叠顺序相关，显示越靠下层的窗口在列表中顺序越靠前
  \sa DWindowManagerHelper::allWindowIdList
 */
QVector<quint32> DWindowManagerHelper::currentWorkspaceWindowIdList() const
{
    return callPlatformFunction<QVector<quint32>, QVector<quint32>(*)()>(_getCurrentWorkspaceWindows);
}

/*!
  \brief DWindowManagerHelper::currentWorkspaceWindowIdList
  \return 返回当前工作区所有本地窗口对象列表。和 currentWorkspaceWindowIdList
  类似，只不过自动通过窗口id创建了 DForeignWindow 对象
  \note 顺序和窗口层叠顺序相关，显示越靠下层的窗口在列表中顺序越靠前
  \note 列表中对象的生命周期由 DForeignWindow 负责
  \warning 此列表中不包含由当前应用创建的窗口
  \sa DWindowManagerHelper::currentWorkspaceWindowIdList
  \sa DForeignWindow::fromWinId
 */
QList<DForeignWindow *> DWindowManagerHelper::currentWorkspaceWindows() const
{
    D_DC(DWindowManagerHelper);

    for (QWindow *w : d->windowList) {
        w->deleteLater();
    }

    d->windowList.clear();

    QList<WId> currentApplicationWindowList;
    const QWindowList &list = qApp->allWindows();

    currentApplicationWindowList.reserve(list.size());

    for (auto window : list) {
        if (window->property("_q_foreignWinId").isValid()) continue;

        currentApplicationWindowList.append(window->winId());
    }

    QVector<quint32> wmClientList = currentWorkspaceWindowIdList();

    for (WId wid : wmClientList) {
        if (currentApplicationWindowList.contains(wid))
            continue;

        if (DForeignWindow *w = DForeignWindow::fromWinId(wid)) {
            d->windowList << w;
        }
    }

    return d->windowList;
}

/*!
  \brief DWindowManagerHelper::windowFromPoint
  \return 返回 \a p 位置的窗口 Id，如果出错返回 0
  \note 可以通过 DForeignWindow::fromWinId 创建窗口对象
 */
quint32 DWindowManagerHelper::windowFromPoint(const QPoint &p)
{
    return callPlatformFunction<quint32, quint32 (*)(const QPoint &)>(_windowFromPoint, p);
}

/*!
  \brief DWindowManagerHelper::DWindowManagerHelper
  不允许直接实例化此对象
  \a parent
  \sa DWindowManagerHelper::instance
 */
DWindowManagerHelper::DWindowManagerHelper(QObject *parent)
    : QObject(parent)
    , DObject(*new DWindowManagerHelperPrivate(this))
{
    connectWindowManagerChangedSignal(this, [this] {
        Q_EMIT windowManagerChanged();
    });
    connectHasBlurWindowChanged(this, [this] {
        Q_EMIT hasBlurWindowChanged();
    });
    connectHasCompositeChanged(this, [this] {
        Q_EMIT hasCompositeChanged();
    });
    connectHasNoTitlebarChanged(this, [this] {
        Q_EMIT hasNoTitlebarChanged();
    });
    connectHasWallpaperEffectChanged(this, [this] {
        Q_EMIT hasWallpaperEffectChanged();
    });
    connectWindowListChanged(this, [this] {
        Q_EMIT windowListChanged();
    });
    connectWindowMotifWMHintsChanged(this, [this] (quint32 winId) {
        Q_EMIT windowMotifWMHintsChanged(winId);
    });
}


DGUI_END_NAMESPACE
