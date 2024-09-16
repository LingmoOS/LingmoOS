// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dguiapplicationhelper.h"
#include "dplatformhandle.h"
#include "dplatformtheme.h"
#include "dwindowmanagerhelper.h"
#include "wayland/dcontextshellwindow.h"
#include <private/qwaylandwindow_p.h>

#include <QWindow>
#include <QGuiApplication>
#include <QDebug>
#include <QPlatformSurfaceEvent>

DGUI_BEGIN_NAMESPACE

#define DXCB_PLUGIN_KEY "dxcb"
#define DXCB_PLUGIN_SYMBOLIC_PROPERTY "_d_isDxcb"

#define DEFINE_CONST_CHAR(Name) const char _##Name[] = "_d_" #Name

DEFINE_CONST_CHAR(useDxcb);
DEFINE_CONST_CHAR(redirectContent);
DEFINE_CONST_CHAR(netWmStates);
DEFINE_CONST_CHAR(windowRadius);
DEFINE_CONST_CHAR(borderWidth);
DEFINE_CONST_CHAR(borderColor);
DEFINE_CONST_CHAR(windowEffect);
DEFINE_CONST_CHAR(windowStartUpEffect);
DEFINE_CONST_CHAR(shadowRadius);
DEFINE_CONST_CHAR(shadowOffset);
DEFINE_CONST_CHAR(shadowColor);
DEFINE_CONST_CHAR(clipPath);
DEFINE_CONST_CHAR(frameMask);
DEFINE_CONST_CHAR(frameMargins);
DEFINE_CONST_CHAR(translucentBackground);
DEFINE_CONST_CHAR(enableSystemResize);
DEFINE_CONST_CHAR(enableSystemMove);
DEFINE_CONST_CHAR(enableBlurWindow);
DEFINE_CONST_CHAR(windowBlurAreas);
DEFINE_CONST_CHAR(windowBlurPaths);
DEFINE_CONST_CHAR(windowWallpaperParas);
DEFINE_CONST_CHAR(autoInputMaskByClipPath);

DEFINE_CONST_CHAR(resolve_mask);
enum PropRole {
    WindowRadius,

    // TO BE CONTINUE
};

// functions
DEFINE_CONST_CHAR(setWmBlurWindowBackgroundArea);
DEFINE_CONST_CHAR(setWmBlurWindowBackgroundPathList);
DEFINE_CONST_CHAR(setWmBlurWindowBackgroundMaskImage);
DEFINE_CONST_CHAR(setWmWallpaperParameter);
DEFINE_CONST_CHAR(setWindowProperty);
DEFINE_CONST_CHAR(pluginVersion);
DEFINE_CONST_CHAR(disableOverrideCursor);
DEFINE_CONST_CHAR(enableDxcb);
DEFINE_CONST_CHAR(isEnableDxcb);
DEFINE_CONST_CHAR(setEnableNoTitlebar);
DEFINE_CONST_CHAR(isEnableNoTitlebar);
DEFINE_CONST_CHAR(clientLeader);

static void resolve(QObject *obj, PropRole role)
{
    int mask = obj->property(_resolve_mask).toInt();
    obj->setProperty(_resolve_mask, (mask |= 1 << role));
}

static bool resolved(QObject *obj, PropRole role)
{
    int mask = obj->property(_resolve_mask).toInt();
    return mask & (1 << role);
}

static void setWindowProperty(QWindow *window, const char *name, const QVariant &value)
{
    if (!window)
        return;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    static QFunctionPointer setWindowProperty = qApp->platformFunction(_setWindowProperty);
#else
    constexpr QFunctionPointer setWindowProperty = nullptr;
#endif

    if (!setWindowProperty) {
        window->setProperty(name, value);

        return;
    }

    reinterpret_cast<void(*)(QWindow *, const char *, const QVariant &)>(setWindowProperty)(window, name, value);
}

static bool isTreeLand()
{
    return qEnvironmentVariable("DDE_CURRENT_COMPOSITOR") == "TreeLand";
};

/*!
  \class Dtk::Gui::DPlatformHandle
  \inmodule dtkgui
  \brief 一个和Qt dxcb平台插件交互的工具类.

  实质性的功能皆在dxcb插件中实现，此插件目前只
  支持X11平台，在其它平台上使用这个类不会有任何效果。关于dxcb：它介于Qt应用和Qt xcb平台
  插件之间，通过覆写xcb插件中某些对象的虚函数来改变它的一些行为，本质上来讲是Qt xcb插件的
  扩展，在X11平台上为DTK应用提供了一些改变窗口效果的功能（比如自定义窗口的边框）、其它和平
  台密切相关的实现（比如修复Qt应用在X11平台的一些bug），不能脱离Qt xcb插件独立运行。dxcb
  通过重载 QPlatformNativeInterface 提供接口，DPlatformHandle 中使用
  QGuiApplication::platformFunction 调用这些接口。Application、dxcb、qt xcb 之间
  的关系：
  \raw HTML
  <pre style="font-family: FreeMono, Consolas, Menlo, 'Noto Mono', 'Courier New', Courier, monospace;line-height: 100%;">
        ┏━━━━━━━━━━━━━━━━┓
        ┃   Application  ┃
        ┗━━━━━━━━━━━━━━━━┛
                ⇅
      ┏━━━━━━━━━━━━━━━━━━━━┓
      ┃     dxcb plugin    ┃
      ┗━━━━━━━━━━━━━━━━━━━━┛
                ⇅
   ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
   ┃      qt xcb platform      ┃
   ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
  </pre>
  \endraw

  \ref {http://doc.qt.io/qt-5/qpa.html}{QPA}
  \ref {https://github.com/linuxdeepin/qt5dxcb-plugin/}{dxcb插件}
  \sa Dtk::Gui::DPlatformHandle::isDXcbPlatform
  \sa QGuiApplication::platformNativeInterface
  \sa Dtk::Widget::DMainWindow
  \sa DWindowManagerHelper
  \warning 使用此工具前要确保应用加载了dxcb插件
  \warning 因为 QGuiApplication::platformFunction 是在 Qt 5.4.0 版本引入的新接口，
  所以 DPlatformHandle 不支持 Qt 5.4.0 以下版本。
 */

/*!
  \property DPlatformHandle::windowRadius
  \brief 窗口的圆角半径。默认情况下，窗口管理器支持混成时，圆角半径为4，否则为0，并且
  会随着窗口管理器开启/关闭混成效果而变化
  \note 可读可写
  \note 窗口为半屏、全屏或最大化状态时此值不生效
  \warning 手动设置值后将无法再随着窗口管理器是否支持混成而自动更新边框宽度
  \sa DWindowManagerHelper::hasComposite
  */

/*!
  \property DPlatformHandle::borderWidth
  \brief 窗口的外边框宽度。默认情况下，窗口管理器支持混成时，边框宽度为1，否则对于可以
  改变大小的窗口其值为2，否则为1，并且会随着窗口管理器开启/关闭混成效果而变化
  \note 可读可写
  \warning 手动设置值后将无法再随着窗口管理器是否支持混成而自动更新边框宽度
  \sa DWindowManagerHelper::hasComposite
  */

/*!
  \property DPlatformHandle::borderColor
  \brief 窗口外边框的颜色。默认情况下，窗口管理器支持混成时，颜色为 QColor(0, 0, 0, 255 * 0.15)，
  否则为边框颜色和 #e0e0e0 的混合，并且会随着窗口管理器开启/关闭混成效果而变化
  \note 可读可写
  \sa DWindowManagerHelper::hasComposite
  */

/*!
  \property DPlatformHandle::shadowRadius
  \brief 窗口的阴影半径。默认为 60
  \note 可读可写
  \note 窗口管理器不支持混成时此值无效
  \sa DWindowManagerHelper::hasComposite
  */

/*!
  \property DPlatformHandle::shadowOffset
  \brief 窗口阴影的偏移量。默认为 QPoint(0，16)
  \note 可读可写
  \note 窗口管理器不支持混成时此值无效
  \sa DWindowManagerHelper::hasComposite
  */

/*!
  \property DPlatformHandle::shadowColor
  \brief 窗口阴影的颜色。默认为 QColor(0, 0, 0, 255 * 0.6)
  \note 可读可写
  \note 窗口管理器不支持混成时此值无效
  \sa DWindowManagerHelper::hasComposite
  */

/*!
  \property DPlatformHandle::clipPath
  \brief 窗口的裁剪区域。处于路径内部的区域为窗口有效区域，非有效区域内的窗口内容
  将无法显示，并且无法收到鼠标和触摸事件。示例：
  \code
  QWidget w;
  QPainterPath path;
  QFont font;

  font.setPixelSize(100);
  path.addText(0, 150, font, "deepin");

  DPlatformHandle handle(&w);

  handle.setClipPath(path);
  w.resize(400, 200);
  w.show();
  \endcode
  \image clip_window_demo.gif
  \note 可读可写
  \note 窗口的阴影和外边框绘制和其有效区域密切相关
  \warning 设置此属性后将导致 DPlatformHandle::windowRadius 失效
  */

/*!
  \property DPlatformHandle::frameMask
  \brief 设置 Frame Window 的遮罩，和 \a clipPath 不同的是，它的裁剪包括阴影
  部分。示例：
  \code
  QWidget w;
  DPlatformHandle handle(&w);

  // 为何更好的观察效果，此处将阴影改为蓝色
  handle.setShadowColor(Qt::blue);
  w.resize(400, 200);
  w.show();
  QRect frame_rect = w.rect() + handle.frameMargins();
  frame_rect.moveTopLeft(QPoint(0, 0));
  handle.setFrameMask(QRegion(frame_rect, QRegion::Ellipse));
  \endcode
  \image frame_mask_demo.png
  \note 可读可写
  \note 由于实现机制限制，使用此属性裁剪 Frame Window 时，无法去除边缘产生的锯齿
  */

/*!
  \property DPlatformHandle::frameMargins
  \brief Sub Window 相对于 Frame Window 的边距
  \image frame_margins.png
  \note 只读
  \warning 在窗口隐藏时不保证此值的正确性
  */

/*!
  \property DPlatformHandle::translucentBackground
  \brief 如果此属性值为 true，则在更新窗口绘制内容之前会先清空要更新区域内的图像，
  否则不清空，默认为 false
  \note 可读可写
  */

/*!
  \property DPlatformHandle::enableSystemResize
  \brief 如果此属性值为 true，则允许外界改变窗口的大小（如使用鼠标拖拽窗口边框），
  否则不允许。默认为 true
  \note 无论属性值是多少，Qt::Popup 和 Qt::BypassWindowManagerHint 类型的
  窗口都不允许改变大小
  \note 可读可写
  \note 此属性仅仅控制 dxcb 中的行为，不会影响窗口管理器的行为
  \sa QWidget::setFixedSize
  \sa QWindow::setMinimumSize
  \sa QWindow::setMaximumSize
  \sa DWindowManagerHelper::FUNC_RESIZE
  */

/*!
  \property DPlatformHandle::enableSystemMove
  \brief 如果此属性值为 ture，则允许外界移动窗口的位置（如使用鼠标拖拽移动窗口），
  否则不允许。默认为 true
  \note 无论属性值是多少，Qt::Popup 和 Qt::BypassWindowManagerHint 类型的
  窗口都不允许改变大小
  \note 可读可写
  \note 此属性仅仅控制 dxcb 中的行为，不会影响窗口管理器的行为
  \sa DWindowManagerHelper::FUNC_MOVE
  */

/*!
  \property DPlatformHandle::enableBlurWindow
  \brief 如果此属性为 true，则窗口有效区域内的背景将呈现出模糊效果，否则无特效。
  默认为 false
  \note 可读可写
  \sa DPlatformHandle::setWindowBlurAreaByWM
  */

/*!
  \property DPlatformHandle::autoInputMaskByClipPath
  \brief 如果此属性值为 true，则窗口可输入区域跟随其 \a clipPath 属性，否则不
  跟随。默认为 true
  \note 可输入区域指可接收鼠标或触摸事件的区域
  \note 可读可写
  */

/*!
  \property DPlatformHandle::realWindowId
  \brief Sub Window 的窗口 id，直接使用 QWindow::winId 或 QWidget::winId
  获取到的是 Frame Window 的窗口 id
  \note 只读
  */

/*!
  \fn void DPlatformHandle::frameMarginsChanged()
  \brief 信号会在 frameMargins 属性的值改变时被发送.
 */
/*!
  \fn void DPlatformHandle::windowRadiusChanged()
  \brief 信号会在 windowRadius 属性的值改变时被发送
 */
/*!
  \fn void DPlatformHandle::borderWidthChanged()
  \brief 信号会在 borderWidth 属性的值改变时被发送
 */
/*!
  \fn void DPlatformHandle::borderColorChanged()
  \brief 信号会在 borderColor 属性的值改变时被发送
 */
/*!
  \fn void DPlatformHandle::shadowRadiusChanged()
  \brief 信号会在 shadowRadius 属性的值改变时被发送
 */
/*!
  \fn void DPlatformHandle::shadowOffsetChanged()
  \brief 信号会在 shadowOffset 属性的值改变时被发送
 */
/*!
  \fn void DPlatformHandle::shadowColorChanged()
  \brief 信号会在 shadowColor 属性的值改变时被发送
 */
/*!
  \fn void DPlatformHandle::clipPathChanged()
  \brief 信号会在 clipPath 属性的值改变时被发送
 */
/*!
  \fn void DPlatformHandle::frameMaskChanged()
  \brief 信号会在 frameMask 属性的值改变时被发送
 */
/*!
  \fn void DPlatformHandle::translucentBackgroundChanged()
  \brief 信号会在 translucentBackground 属性的值改变时被发送
 */
/*!
  \fn void DPlatformHandle::enableSystemResizeChanged()
  \brief 信号会在 enableSystemResize 属性的值改变时被发送
  */
/*!
  \fn void DPlatformHandle::enableSystemMoveChanged()
  \brief 信号会在 enableSystemMove 属性的值改变时被发送
 */
/*!
  \fn void DPlatformHandle::enableBlurWindowChanged()
  \brief 信号会在 enableBlurWindow 属性的值改变时被发送
 */
/*!
  \fn void DPlatformHandle::autoInputMaskByClipPathChanged()
  \brief 信号会在 autoInputMaskByClipPath 属性的值改变时被发送
 */

/*!
  \class Dtk::Gui::DPlatformHandle::WMBlurArea
  \inmodule dtkgui

  \brief 描述窗口背景模糊区域的数据结构，包含位置、大小、圆角半径等信息.

  \var x
  水平方向的坐标
  \var y
  竖直方向的坐标
  \var width
  区域的宽度
  \var height
  区域的高度
  \var xRadius
  水平方向的圆角半径
  \var yRaduis
  竖直方向的圆角半径
*/

/*!
  \brief DPlatformHandle::DPlatformHandle
  将 \a window 对象传递给 enableDXcbForWindow
  \a window 要开启DTK风格的主窗口
  \a parent DPlatformHandle 对象的父对象
  \sa DPlatformHandle::enableDXcbForWindow(QWindow *)
 */
DPlatformHandle::DPlatformHandle(QWindow *window, QObject *parent)
    : QObject(parent)
    , m_window(window)
{
    enableDXcbForWindow(window);

    window->installEventFilter(this);
}

/*!
  \brief DPlatformHandle::pluginVersion
  \return 返回dxcb插件的版本
  \note 在旧版dxcb插件中未实现获取版本的接口，将会返回一个空的 QString 对象
 */
QString DPlatformHandle::pluginVersion()
{
    QFunctionPointer pv = 0;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    pv = qApp->platformFunction(_pluginVersion);
#endif

    if (Q_UNLIKELY(!pv))
        return QString();

    return reinterpret_cast<QString(*)()>(pv)();
}

/*!
  \brief DPlatformHandle::isDXcbPlatform 检查当前程序是否使用了dxcb平台插件。
  \return 正在使用返回 true，否则返回 false。
 */
bool DPlatformHandle::isDXcbPlatform()
{
    if (!qApp)
        return false;

    static bool _is_dxcb = qApp->platformName() == DXCB_PLUGIN_KEY || qApp->property(DXCB_PLUGIN_SYMBOLIC_PROPERTY).toBool();

    return _is_dxcb;
}

/*!
  \brief DPlatformHandle::enableDXcbForWindow
  将 QWindow 的窗口装饰设置为 DTK 风格，这将使用 Qt::FramelessWindowHint 去除本地窗口管理器
  给窗口附加的边框修饰以及窗口阴影效果，并且，会创建一个对应的本地窗口（在X11平台就是指X Window）
  作为此窗口的父窗口，父窗口（Frame Window）中将根据子窗口（Sub Window）的有效区域绘制阴影和边
  框等效果，默认情况下，子窗口的有效区域为一个圆角矩形，结构如下：
  \raw HTML
  <pre style="font-family: FreeMono, Consolas, Menlo, 'Noto Mono', 'Courier New', Courier, monospace;line-height: 100%;">
  ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓
  ┃    Frame Window             ┃
  ┃                             ┃
  ┃                             ┃
  ┃     ╭┅┅┅┅┅┅┅┅┅┅┅┅┅┅┅┅┅╮     ┃
  ┃     ┋    Sub Window   ┋     ┃
  ┃     ┋                 ┋     ┃
  ┃     ┋                 ┋     ┃
  ┃     ┋                 ┋     ┃
  ┃     ┋                 ┋     ┃
  ┃     ┋                 ┋     ┃
  ┃     ┋                 ┋     ┃
  ┃     ┋                 ┋     ┃
  ┃     ┋                 ┋     ┃
  ┃     ╰┅┅┅┅┅┅┅┅┅┅┅┅┅┅┅┅┅╯     ┃
  ┃                             ┃
  ┃                             ┃
  ┃                             ┃
  ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛
  </pre>
  \endraw

  但是，如果窗口管理器自身支持隐藏窗口标题栏，则此方法将优先调用 enableNoTitlebarForWindow 实现同样的效果。

  例子：
  \code
  QWidget w1;

  w1.setWindowTitle("使用系统边框的窗口");
  w1.show();

  DMainWindow w2;
  QWidget w3;

  w2.titlebar()->setTitle("使用DTK风格边框带标题栏的窗口");
  w3.setWindowTitle("使用DTK风格边框没有标题栏的窗口");
  w2.show();

  DPlatformHandle::enableDXcbForWindow(&w3);
  w3.show(); // 因为这个窗口没有标题栏，所以不会显示窗口标题

  \endcode
  \image dtk_and_system_window.jpeg
  开启了dxcb的窗口，在窗口外边缘10像素的范围按下鼠标左键可以触发改变窗口大小的行为，
  而且会自动将鼠标吸附到对应的窗口边缘，增强了拖拽改变窗口大小的体验。效果：
  \image dtk_window_cursor_effect.gif
  另外，所有到达主窗口的鼠标移动事件如果没有调用 QEvent::accepted ，则会触发主窗
  口的移动效果，默认情况下，一个没有子控件的DTK窗口，如果没有重写 QWidget::mouseMoveEvent ，
  则使用鼠标左键在窗口的任意地方按住并移动都会触发移动窗口的动作。如：
  \code
  class Window : public QWidget
  {
  public:
     explicit Window() {

     }

  protected:
     void mouseMoveEvent(QMouseEvent *event) override {
         event->accept();
     }
  };
  \endcode
  \code
  Window w;
  DPlatformHandle::enableDXcbForWindow(&w);
  w.show();
  \endcode
  将无法使用鼠标移动窗口w

  窗口管理器（如X11平台上的Window Manager）是否支持混成会影响dxcb插件对窗口添加的默认装饰。
  \note 在 Deepin 桌面环境中，打开窗口特效则支持混成，关闭窗口特效则不支持混成

  支持混成：
  \image enable_composite.png
  不支持混成：
  \image disable_composite.png
  并且，在不支持混成的窗口管理器中，上述“窗口边缘的鼠标吸附”效果也会被禁用。可以使用
  DWindowManagerHelper::hasComposite 或 QX11Info::isCompositingManagerRunning
  判断当前运行的窗口管理器是否支持混成。
  \a window
  \sa Dtk::Gui::DPlatformHandle::setEnabledNoTitlebarForWindow()
 */
void DPlatformHandle::enableDXcbForWindow(QWindow *window)
{
    // 优先使用窗口管理器中实现的no titlebar接口实现自定义窗口修饰器的效果
    if (setEnabledNoTitlebarForWindow(window, true)) {
        return;
    }

    if (!isDXcbPlatform())
        return;

    QFunctionPointer enable_dxcb = nullptr;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    enable_dxcb = qApp->platformFunction(_enableDxcb);
#endif

    if (enable_dxcb) {
        (*reinterpret_cast<bool(*)(QWindow*)>(enable_dxcb))(window);
    } else if (window->handle()) {
        Q_ASSERT_X(window->property(_useDxcb).toBool(), "DPlatformHandler:",
                   "Must be called before window handle has been created. See also QWindow::handle()");
    } else {
        window->setProperty(_useDxcb, true);
    }
}

/*!
  \brief DPlatformHandle::enableDXcbForWindow
  功能上和 DPlatformHandle::enableDXcbForWindow(QWindow *) 一致.

  \a window
  \a redirectContent 如果值为 true，Sub Window 将不可见，且它的绘制内容会
  被合成到外层的 Frame Window（它的父窗口），否则 Sub Window 和 Frame Window
  会分开绘制和显示。默认情况下只需要使用 DPlatformHandle::enableDXcbForWindow(QWindow *)，
  dxcb插件中会自动根据窗口渲染类型选择使用更合适的实现方式，使用 OpenGL 渲染的窗口将开启
  redirectContent 模式。
  \note 如果窗口内嵌入了其它的本地窗口（如X11平台的X Window），默认情况下，这个窗口
  绘制的内容不受dxcb插件的控制，它的绘制内容可能会超过 Sub Window 的有效区域，这种
  情况下，应该使用此接口，并将 redirectContent 指定为 true。

  \ref {https://www.x.org/releases/X11R7.5/doc/damageproto/damageproto.txt}{X11 Damage}
 */
void DPlatformHandle::enableDXcbForWindow(QWindow *window, bool redirectContent)
{
    window->setProperty(_redirectContent, redirectContent);

    enableDXcbForWindow(window);
}

/*!
  \brief DPlatformHandle::isEnabledDXcb.

  \return 如果窗口 \a window 开启了DTK风格的窗口修饰则返回 true，否则返回 false
  \sa DPlatformHandle::isEnabledNoTitlebar()
 */
bool DPlatformHandle::isEnabledDXcb(const QWindow *window)
{
    if (isEnabledNoTitlebar(window))
        return true;

    QFunctionPointer is_enable_dxcb = nullptr;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    is_enable_dxcb = qApp->platformFunction(_isEnableDxcb);
#endif

    if (is_enable_dxcb) {
        return (*reinterpret_cast<bool(*)(const QWindow*)>(is_enable_dxcb))(window);
    }

    return window->property(_useDxcb).toBool();
}

static void initWindowRadius(QWindow *window)
{
    if (window->property(_windowRadius).isValid())
        return;

    auto theme = DGuiApplicationHelper::instance()->systemTheme();
    int radius = theme->windowRadius(18); //###(zccrs): 暂时在此处给窗口默认设置为18px的圆角

    setWindowProperty(window, _windowRadius, radius);
    // Qt::UniqueConnection will report a warning
    // to `unique connections require a pointer to member function of a QObject subclass`.
    const char *uniqueueConnectionFlag("_d_uniqueueConnectionFlag");
    bool connected = window->property(uniqueueConnectionFlag).toBool();
    if (!connected) {
        window->setProperty(uniqueueConnectionFlag, true);
        window->connect(theme, &DPlatformTheme::windowRadiusChanged, window, [window] (int radius) {
            if (!resolved(window, PropRole::WindowRadius))
                setWindowProperty(window, _windowRadius, radius);
        });
    }
}

class Q_DECL_HIDDEN CreatorWindowEventFile : public QObject {
    bool m_windowMoving = false;
public:
    CreatorWindowEventFile(QObject *par= nullptr): QObject(par){}

public:
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (event->type() == QEvent::PlatformSurface) {
            QPlatformSurfaceEvent *se = static_cast<QPlatformSurfaceEvent*>(event);
            if (se->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated) {  // 若收到此信号， 则 WinID 已被创建
                initWindowRadius(qobject_cast<QWindow *>(watched));
                deleteLater();
            }
        }

        if (auto *w = qobject_cast<QWindow *>(watched); w && isTreeLand()) {
            if(DContextShellWindow *window = DContextShellWindow::get(qobject_cast<QWindow *>(watched))) {
                bool is_mouse_move = event->type() == QEvent::MouseMove && static_cast<QMouseEvent*>(event)->buttons() == Qt::LeftButton;

                if (event->type() == QEvent::MouseButtonRelease) {
                    m_windowMoving = false;
                }

                // workaround for kwin: Qt receives no release event when kwin finishes MOVE operation,
                // which makes app hang in windowMoving state. when a press happens, there's no sense of
                // keeping the moving state, we can just reset ti back to normal.
                if (event->type() == QEvent::MouseButtonPress) {
                    m_windowMoving = false;
                }

                // FIXME: We need to check whether the event is accepted.
                //        Only when the upper control does not accept the event,
                //        the window should be moved through the window.
                //        But every event here has been accepted. I don't know what happened.
                if (is_mouse_move && w->geometry().contains(static_cast<QMouseEvent*>(event)->globalPos())) {
                    if (!m_windowMoving && window->noTitlebar()) {
                        m_windowMoving = true;

                        event->accept();
                        static_cast<QtWaylandClient::QWaylandWindow *>(w->handle())->startSystemMove();
                    }
                }
            }
        }

        return QObject::eventFilter(watched, event);
    }
};

/*!
  \brief DPlatformHandle::setEnabledNoTitlebarForWindow.

  使用窗口管理器提供的方式隐藏窗口的标题栏，目前已适配 DDE KWin 窗管，在窗口管理器支持的前提下，
  此方法将通过设置窗口属性 _DEEPIN_SCISSOR_WINDOW 的值为 1 来开启无标题栏效果。
  \a window 被设置的 QWindow 实例.
  \a enable 是否开启无标题属性.
  \return 设置成功返回 true,否则返回false.

  \sa DPlatformHandle::enableDXcbForWindow()
  \sa DWindowManagerHelper::hasNoTitlebar()
 */
bool DPlatformHandle::setEnabledNoTitlebarForWindow(QWindow *window, bool enable)
{
    auto isDWaylandPlatform = [] {
        return qApp->platformName() == "dwayland" || qApp->property("_d_isDwayland").toBool();
    };
    if (!(isDXcbPlatform() || isDWaylandPlatform() || isTreeLand()))
        return false;

    if (window && isTreeLand()) {
        DContextShellWindow *contextWindow = DContextShellWindow::get(window);
        if (contextWindow->noTitlebar() == enable)
            return true;
        contextWindow->setNoTitlebar(enable);
        window->installEventFilter(new CreatorWindowEventFile(window));
        return true;
    }

    if (isEnabledNoTitlebar(window) == enable)
        return true;

    QFunctionPointer enable_no_titlear = nullptr;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    enable_no_titlear = qApp->platformFunction(_setEnableNoTitlebar);
#endif

    if (enable_no_titlear) {
        bool ok = (*reinterpret_cast<bool(*)(QWindow*, bool)>(enable_no_titlear))(window, enable);
        if (ok && enable) {
            if (window->handle()) {
                initWindowRadius(window);
            } else {
                window->installEventFilter(new CreatorWindowEventFile(window));
            }
        }

        return ok;
    }

    return false;
}

/*!
  \brief DPlatformHandle::isEnableNoTitlebar
  \a window
  \return 如果窗口使用窗管提供的方式隐藏了标题栏则返回 true，否则返回 false
  \sa DPlatformHandle::isEnabledDXcb
 */
bool DPlatformHandle::isEnabledNoTitlebar(const QWindow *window)
{
    QFunctionPointer is_enable_no_titlebar = nullptr;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    is_enable_no_titlebar = qApp->platformFunction(_isEnableNoTitlebar);
#endif

    if (is_enable_no_titlebar) {
        return (*reinterpret_cast<bool(*)(const QWindow*)>(is_enable_no_titlebar))(window);
    }

    return false;
}

inline DPlatformHandle::WMBlurArea operator *(const DPlatformHandle::WMBlurArea &area, qreal scale)
{
    if (qFuzzyCompare(scale, 1.0))
        return area;

    DPlatformHandle::WMBlurArea new_area;

    new_area.x = qRound64(area.x * scale);
    new_area.y = qRound64(area.y * scale);
    new_area.width = qRound64(area.width * scale);
    new_area.height = qRound64(area.height * scale);
    new_area.xRadius = qRound64(area.xRadius * scale);
    new_area.yRaduis = qRound64(area.yRaduis * scale);

    return new_area;
}

/*!
  \brief DPlatformHandle::setWindowBlurAreaByWM
  设置窗口背景的模糊区域，示例：
  \code
  QWindow w;
  QVector<DPlatformHandle::WMBlurArea> area_list;
  DPlatformHandle::WMBlurArea area;

  area.x = 50;
  area.y = 50;
  area.width = 200;
  area.height = 200;
  area.xRadius = 10;
  area.yRaduis = 10;
  area_list.append(area);

  DPlatformHandle::setWindowBlurAreaByWM(&w, area_list);

  QSurfaceFormat format = w.format();
  format.setAlphaBufferSize(8);

  w.setFormat(format);
  w.resize(300, 300);
  w.show();

  \endcode
  \image blur_window_demo1.png
  \a window 目标窗口对象
  \a area 模糊区域，此区域范围内的窗口背景将填充为窗口后面内容模糊之后的图像
  \return 如果设置成功则返回 true，否则返回 false
  \note 对于需要显示模糊背景的窗口，需要将其 QSurfaceFormat 的 alpha 通道设置为8
  \note 调用此接口设置窗口背景模糊区域后将覆盖之前所设置的区域，包括调用
  setWindowBlurAreaByWM(QWindow *, const QList<QPainterPath> &)
  所设置的区域
  \note 建议使用 DBlurEffectWidget 实现窗口背景模糊效果
  \note 此功能依赖于窗口管理器的实现，目前仅支持 deepin-wm 和 kwin 这两个窗口管理器
  \sa Dtk::Widget::DBlurEffectWidget
  \sa QSurfaceFormat::setAlphaBufferSize
  \sa QWindow::setFormat
  \sa DWindowManagerHelper::hasBlurWindow
  \sa DPlatformHandle::setWindowBlurAreaByWM(QWindow *, const QList<QPainterPath> &)
 */
bool DPlatformHandle::setWindowBlurAreaByWM(QWindow *window, const QVector<DPlatformHandle::WMBlurArea> &area)
{
    if (!window) {
        return false;
    }

    if (isEnabledDXcb(window)) {
        QVector<quint32> areas;
        for (auto item : area)
            areas << item.x << item.y << item.width << item.height << item.xRadius << item.yRaduis;
        setWindowProperty(window, _windowBlurAreas, QVariant::fromValue(areas));
        return true;
    }

    QFunctionPointer setWmBlurWindowBackgroundArea = Q_NULLPTR;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    setWmBlurWindowBackgroundArea = qApp->platformFunction(_setWmBlurWindowBackgroundArea);
#endif

    if (!setWmBlurWindowBackgroundArea) {
        qWarning("setWindowBlurAreaByWM is not support");

        return false;
    }

    QSurfaceFormat format = window->format();

    format.setAlphaBufferSize(8);
    window->setFormat(format);

    const qreal device_ratio = window->devicePixelRatio();

    if (qFuzzyCompare(device_ratio, 1.0)) {
        return reinterpret_cast<bool(*)(const quint32, const QVector<WMBlurArea>&)>(setWmBlurWindowBackgroundArea)(window->winId(), area);
    }

    QVector<WMBlurArea> new_areas;

    new_areas.reserve(area.size());

    for (const WMBlurArea &a : area) {
        new_areas.append(a * device_ratio);
    }

    return reinterpret_cast<bool(*)(const quint32, const QVector<WMBlurArea>&)>(setWmBlurWindowBackgroundArea)(window->winId(), new_areas);
}

inline QPainterPath operator *(const QPainterPath &path, qreal scale)
{
    if (qFuzzyCompare(1.0, scale))
        return path;

    QPainterPath new_path = path;

    for (int i = 0; i < path.elementCount(); ++i) {
        const QPainterPath::Element &e = path.elementAt(i);

        new_path.setElementPositionAt(i, qRound(e.x * scale), qRound(e.y * scale));
    }

    return new_path;
}

/*!
  \brief DPlatformHandle::setWindowBlurAreaByWM
  设置窗口背景的模糊区域，使用 QPainterPath 描述模糊区域，使用起来更加的灵活，可以
  实现任何形状，但是性能要低于使用 QVector<DPlatformHandle::WMBlurArea>
  描述模糊区域。示例：
  \code
  QWindow w;
  QList<QPainterPath> path_list;
  QPainterPath path;
  QFont font;

  font.setPixelSize(100);
  font.setBold(true);
  path.addText(0, 150, font, "deepin");
  path_list.append(path);

  DPlatformHandle::setWindowBlurAreaByWM(&w, path_list);

  QSurfaceFormat format = w.format();
  format.setAlphaBufferSize(8);

  w.setFormat(format);
  w.resize(300, 300);
  w.show();

  \endcode
  \image blur_window_demo2.png
  \a window 目标窗口对象
  \a paths 模糊区域，此区域范围内的窗口背景将填充为窗口后面内容模糊之后的图像
  \return 如果设置成功则返回 true，否则返回 false
  \note 调用此接口设置窗口背景模糊区域后将覆盖之前所设置的区域，包括调用
  setWindowBlurAreaByWM(QWindow *, QVector<DPlatformHandle::WMBlurArea> &)
  设置的窗口背景模糊路径
  \note 对于需要显示模糊背景的窗口，需要将其 QSurfaceFormat 的 alpha 通道设置为8
  \note 建议使用 DBlurEffectWidget 实现窗口背景模糊效果
  \note 此功能依赖于窗口管理器的实现，目前仅支持 deepin-wm 和 kwin 这两个窗口管理器
  \warning setWindowBlurAreaByWM(QWindow *, QVector<DPlatformHandle::WMBlurArea> &)
  能满足需求请不要使用此接口
  \sa Dtk::Widget::DBlurEffectWidget
  \sa QSurfaceFormat::setAlphaBufferSize
  \sa QWindow::setFormat
  \sa DWindowManagerHelper::hasBlurWindow
  \sa DPlatformHandle::setWindowBlurAreaByWM()
 */
bool DPlatformHandle::setWindowBlurAreaByWM(QWindow *window, const QList<QPainterPath> &paths)
{
    if (!window) {
        return false;
    }

    if (isEnabledDXcb(window)) {
        setWindowProperty(window, _windowBlurPaths, QVariant::fromValue(paths));

        return true;
    }

    QFunctionPointer setWmBlurWindowBackgroundPathList = Q_NULLPTR;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    setWmBlurWindowBackgroundPathList = qApp->platformFunction(_setWmBlurWindowBackgroundPathList);
#endif

    if (!setWmBlurWindowBackgroundPathList) {
        qWarning("setWindowBlurAreaByWM is not support");

        return false;
    }

    QSurfaceFormat format = window->format();

    format.setAlphaBufferSize(8);
    window->setFormat(format);

    const qreal device_ratio = window->devicePixelRatio();

    if (qFuzzyCompare(device_ratio, 1.0)) {
        return reinterpret_cast<bool(*)(const quint32, const QList<QPainterPath>&)>(setWmBlurWindowBackgroundPathList)(window->winId(), paths);
    }

    QList<QPainterPath> new_paths;

    new_paths.reserve(paths.size());

    for (const QPainterPath &p : paths) {
        new_paths.append(p * device_ratio);
    }

    return reinterpret_cast<bool(*)(const quint32, const QList<QPainterPath>&)>(setWmBlurWindowBackgroundPathList)(window->winId(), new_paths);
}

/*!
  \brief DPlatformHandle::setWindowWallpaperParaByWM
  设置窗口背景壁纸，示例：
  \code
  QWindow w;
  QRect area;
  WallpaperScaleMode sMode
  WallpaperFillMode fMode

  area.setRect(50, 50, 200, 200);
  bMode = WallpaperScaleFlag::FollowWindow | WallpaperFillFlag::PreserveAspectCrop;

  DPlatformHandle::setWindowWallpaperParaByWM(&w, area, bMode);

  QSurfaceFormat format = w.format();
  format.setAlphaBufferSize(8);

  w.setFormat(format);
  w.resize(300, 300);
  w.show();

  \endcode
  \a window 目标窗口对象
  \a area 壁纸区域，此区域范围内的窗口背景将填充为用户设置的当前工作区窗口壁纸
  \~Chinese \a sMode 控制壁纸缩放是随屏幕还是随窗口
  \~Chinese \a fMode 控制壁纸是缩放还是裁剪
  \return 如果设置成功则返回 true，否则返回 false
  \note 对于需要显示opengl壁纸特效的窗口，需要将其 QSurfaceFormat 的 alpha 通道设置为8
  \note 需要在window handle有效之后调用否则3d下失效
  \note 调用此接口设置窗口背景壁纸区域后将覆盖之前所设置的区域
  \note 此功能依赖于窗口管理器的实现，目前仅支持 kwin 窗口管理器
  \sa Dtk::Widget::DBlurEffectWidget
  \sa QSurfaceFormat::setAlphaBufferSize
  \sa QWindow::setFormat
  \sa DWindowManagerHelper::hasBlurWindow
  \sa DPlatformHandle::setWindowBlurAreaByWM(QWindow *, const QList<QPainterPath> &)
 */
bool DPlatformHandle::setWindowWallpaperParaByWM(QWindow *window, const QRect &area, WallpaperScaleMode sMode, WallpaperFillMode fMode)
{
    if (!window) {
        return false;
    }

    QFunctionPointer setWmWallpaperParameter = Q_NULLPTR;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    setWmWallpaperParameter = qApp->platformFunction(_setWmWallpaperParameter);
#endif

    if (!setWmWallpaperParameter) {
        qWarning("setWindowWallpaperParaByWM is not support");

        return false;
    }

    QSurfaceFormat format = window->format();

    format.setAlphaBufferSize(8);
    window->setFormat(format);

    quint32 bMode = sMode | fMode;

    // 激活 backing store
    window->setProperty("_d_dxcb_wallpaper", QVariant::fromValue(QPair<QRect, int>(area, bMode)));

    if (!window->handle())  {
        return true;
    } else {
        qWarning() << "because the window handle has been created, so 2D mode will have no effect";
    }

    const qreal device_ratio = window->devicePixelRatio();
    if (qFuzzyCompare(device_ratio, 1.0) || !area.isValid()) {
        return reinterpret_cast<bool(*)(const quint32, const QRect&, const quint32)>(setWmWallpaperParameter)(window->winId(), area, bMode);
    }

    QRect new_area(area.x() * device_ratio,
                   area.y() * device_ratio,
                   area.width() * device_ratio,
                   area.height() * device_ratio);

    return reinterpret_cast<bool(*)(const quint32, const QRect&, const quint32)>(setWmWallpaperParameter)(window->winId(), new_area, bMode);
}

/*!
  \brief DPlatformHandle::connectWindowManagerChangedSignal
  将窗口管理器变化的信号链接到 \a object 对象的 \a slot 槽，建议使用　DWindowManager::windowManagerChanged
  \a object
  \a slot
  \return 如果链接成功则返回 true，否则返回 false
  \sa DWindowManagerHelper::windowManagerChanged()
 */
bool DPlatformHandle::connectWindowManagerChangedSignal(QObject *object, std::function<void ()> slot)
{
    if (object) {
        return QObject::connect(DWindowManagerHelper::instance(), &DWindowManagerHelper::windowManagerChanged, object, slot);
    }

    return QObject::connect(DWindowManagerHelper::instance(), &DWindowManagerHelper::windowManagerChanged, slot);
}

/*!
  \brief DPlatformHandle::connectHasBlurWindowChanged
  将窗口管理器是否支持背景模糊的信号链接到 object 对象的 slot 槽，建议使用
  DWindowManager::hasBlurWindowChanged
  \a object
  \a slot
  \return 如果链接成功则返回 true，否则返回 false
  \sa DWindowManagerHelper::hasBlurWindowChanged
 */
bool DPlatformHandle::connectHasBlurWindowChanged(QObject *object, std::function<void ()> slot)
{
    if (object) {
        return QObject::connect(DWindowManagerHelper::instance(), &DWindowManagerHelper::hasBlurWindowChanged, object, slot);
    }

    return QObject::connect(DWindowManagerHelper::instance(), &DWindowManagerHelper::hasBlurWindowChanged, slot);
}

/*!
  \brief DPlatformHandle::setWindowBlurAreaByWM
  这只是一个重载的函数，将调用 setWindowBlurAreaByWM(const QList<QPainterPath> &paths)
  并将构造对象时传递的主窗口当做第一个参数
  \a area
  \return
  \sa DPlatformHandle::setWindowBlurAreaByWM(const QList<QPainterPath> &paths)
 */
bool DPlatformHandle::setWindowBlurAreaByWM(const QVector<DPlatformHandle::WMBlurArea> &area)
{
    return setWindowBlurAreaByWM(m_window, area);
}

/*!
  \brief DPlatformHandle::setWindowBlurAreaByWM
  这只是一个重载的函数，将调用 setWindowBlurAreaByWM(const QVector<DPlatformHandle::WMBlurArea> &area)
  并将构造对象时传递的主窗口当做第一个参数
  \a paths
  \return
  \sa DPlatformHandle::setWindowBlurAreaByWM()
 */
bool DPlatformHandle::setWindowBlurAreaByWM(const QList<QPainterPath> &paths)
{
    return setWindowBlurAreaByWM(m_window, paths);
}

/*!
  \brief DPlatformHandle::setDisableWindowOverrideCursor
  如果 \a disable 为 true，则禁止窗口 \a window 改变光标样式，否则允许改变光标样式。
  窗口被禁止改变光标样式后，使用 QWindow::setCursor 将不会产生任何效果。
  \a window
  \a disable
 */
void DPlatformHandle::setDisableWindowOverrideCursor(QWindow *window, bool disable)
{
    window->setProperty(_disableOverrideCursor, disable);
}

int DPlatformHandle::windowRadius() const
{
    return m_window->property(_windowRadius).toInt();
}

int DPlatformHandle::borderWidth() const
{
    return m_window->property(_borderWidth).toInt();
}

QColor DPlatformHandle::borderColor() const
{
    return qvariant_cast<QColor>(m_window->property(_borderColor));
}

int DPlatformHandle::shadowRadius() const
{
    return m_window->property(_shadowRadius).toInt();
}

QPoint DPlatformHandle::shadowOffset() const
{
    return m_window->property(_shadowOffset).toPoint();
}

QColor DPlatformHandle::shadowColor() const
{
    return qvariant_cast<QColor>(m_window->property(_shadowColor));
}

DPlatformHandle::EffectScene DPlatformHandle::windowEffect()
{
    return qvariant_cast<EffectScene>(m_window->property(_windowEffect));
}

DPlatformHandle::EffectType DPlatformHandle::windowStartUpEffect()
{
    return qvariant_cast<EffectType>(m_window->property(_windowStartUpEffect));
}

QPainterPath DPlatformHandle::clipPath() const
{
    return qvariant_cast<QPainterPath>(m_window->property(_clipPath));
}

QRegion DPlatformHandle::frameMask() const
{
    return qvariant_cast<QRegion>(m_window->property(_frameMask));
}

QMargins DPlatformHandle::frameMargins() const
{
    return qvariant_cast<QMargins>(m_window->property(_frameMargins));
}

bool DPlatformHandle::translucentBackground() const
{
    return m_window->property(_translucentBackground).toBool();
}

bool DPlatformHandle::enableSystemResize() const
{
    return m_window->property(_enableSystemResize).toBool();
}

bool DPlatformHandle::enableSystemMove() const
{
    return m_window->property(_enableSystemMove).toBool();
}

bool DPlatformHandle::enableBlurWindow() const
{
    return m_window->property(_enableBlurWindow).toBool();
}

bool DPlatformHandle::autoInputMaskByClipPath() const
{
    return m_window->property(_autoInputMaskByClipPath).toBool();
}

WId DPlatformHandle::realWindowId() const
{
    return qvariant_cast<WId>(m_window->property("_d_real_content_window"));
}

WId DPlatformHandle::windowLeader()
{
    QFunctionPointer clientLeader = Q_NULLPTR;

#if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    clientLeader = qApp->platformFunction(_clientLeader);
#endif

    if (!clientLeader) {
        return 0;
    }

    return reinterpret_cast<quint32(*)()>(clientLeader)();
}

void DPlatformHandle::setWindowRadius(int windowRadius)
{
    setWindowProperty(m_window, _windowRadius, windowRadius);
    resolve(m_window, PropRole::WindowRadius);
}

void DPlatformHandle::setBorderWidth(int borderWidth)
{
    setWindowProperty(m_window, _borderWidth, borderWidth);
}

void DPlatformHandle::setBorderColor(const QColor &borderColor)
{
    setWindowProperty(m_window, _borderColor, QVariant::fromValue(borderColor));
}

void DPlatformHandle::setWindowEffect(DPlatformHandle::EffectScenes effectScene)
{
    setWindowProperty(m_window, _windowEffect, static_cast<quint32>(effectScene));
}

void DPlatformHandle::setWindowStartUpEffect(DPlatformHandle::EffectTypes effectType)
{
    setWindowProperty(m_window, _windowStartUpEffect, static_cast<quint32>(effectType));
}

void DPlatformHandle::setShadowRadius(int shadowRadius)
{
    setWindowProperty(m_window, _shadowRadius, shadowRadius);
}

void DPlatformHandle::setShadowOffset(const QPoint &shadowOffset)
{
    setWindowProperty(m_window, _shadowOffset, shadowOffset);
}

void DPlatformHandle::setShadowColor(const QColor &shadowColor)
{
    setWindowProperty(m_window, _shadowColor, QVariant::fromValue(shadowColor));
}

void DPlatformHandle::setClipPath(const QPainterPath &clipPath)
{
    setWindowProperty(m_window, _clipPath, QVariant::fromValue(clipPath));
}

void DPlatformHandle::setFrameMask(const QRegion &frameMask)
{
    setWindowProperty(m_window, _frameMask, QVariant::fromValue(frameMask));
}

void DPlatformHandle::setTranslucentBackground(bool translucentBackground)
{
    setWindowProperty(m_window, _translucentBackground, translucentBackground);
}

void DPlatformHandle::setEnableSystemResize(bool enableSystemResize)
{
    setWindowProperty(m_window, _enableSystemResize, enableSystemResize);
}

void DPlatformHandle::setEnableSystemMove(bool enableSystemMove)
{
    setWindowProperty(m_window, _enableSystemMove, enableSystemMove);
}

void DPlatformHandle::setEnableBlurWindow(bool enableBlurWindow)
{
    setWindowProperty(m_window, _enableBlurWindow, enableBlurWindow);
}

void DPlatformHandle::setAutoInputMaskByClipPath(bool autoInputMaskByClipPath)
{
    setWindowProperty(m_window, _autoInputMaskByClipPath, autoInputMaskByClipPath);
}

bool DPlatformHandle::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_window) {
        if (event->type() == QEvent::DynamicPropertyChange) {
            QDynamicPropertyChangeEvent *e = static_cast<QDynamicPropertyChangeEvent *>(event);

            if (e->propertyName() == _windowRadius) {
                Q_EMIT windowRadiusChanged();
            } else if (e->propertyName() == _borderWidth) {
                Q_EMIT borderWidthChanged();
            } else if (e->propertyName() == _borderColor) {
                Q_EMIT borderColorChanged();
            } else if (e->propertyName() == _shadowRadius) {
                Q_EMIT shadowRadiusChanged();
            } else if (e->propertyName() == _shadowOffset) {
                Q_EMIT shadowOffsetChanged();
            } else if (e->propertyName() == _shadowColor) {
                Q_EMIT shadowColorChanged();
            } else if (e->propertyName() == _clipPath) {
                Q_EMIT clipPathChanged();
            } else if (e->propertyName() == _frameMask) {
                Q_EMIT frameMaskChanged();
            } else if (e->propertyName() == _frameMargins) {
                Q_EMIT frameMarginsChanged();
            } else if (e->propertyName() == _translucentBackground) {
                Q_EMIT translucentBackgroundChanged();
            } else if (e->propertyName() == _enableSystemResize) {
                Q_EMIT enableSystemResizeChanged();
            } else if (e->propertyName() == _enableSystemMove) {
                Q_EMIT enableSystemMoveChanged();
            } else if (e->propertyName() == _enableBlurWindow) {
                Q_EMIT enableBlurWindowChanged();
            } else if (e->propertyName() == _autoInputMaskByClipPath) {
                Q_EMIT autoInputMaskByClipPathChanged();
            }
        }
    }

    return false;
}

DGUI_END_NAMESPACE

QT_BEGIN_NAMESPACE
QDebug operator<<(QDebug deg, const DPlatformHandle::WMBlurArea &area)
{
    QDebugStateSaver saver(deg);
    Q_UNUSED(saver)

    deg.setAutoInsertSpaces(true);
    deg << "x:" << area.x
        << "y:" << area.y
        << "width:" << area.width
        << "height:" << area.height
        << "xRadius:" << area.xRadius
        << "yRadius:" << area.yRaduis;

    return deg;
}
QT_END_NAMESPACE
