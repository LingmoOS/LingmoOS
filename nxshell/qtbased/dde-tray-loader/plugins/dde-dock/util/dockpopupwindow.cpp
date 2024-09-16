// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dockpopupwindow.h"
#include "utils.h"
#include "dockcontextmenu.h"
#include "constants.h"
#include "dock-constants.h"

#include <DForeignWindow>
#include <DGuiApplicationHelper>
#include <DPlatformWindowHandle>

#include <QAccessible>
#include <QMetaObject>
#include <qpa/qplatformwindow.h>
#include <QPainter>

DWIDGET_USE_NAMESPACE

DockPopupWindow::DockPopupWindow(QWidget* parent, bool noFocus, bool tellWmDock)
#ifdef DTKWIDGET_CLASS_DBlurEffectWithBorderWidget
    : DBlurEffectWithBorderWidget(parent)
#else
    : DBlurEffectWidget(parent)
#endif
    , m_model(false)
    , m_regionInter(new DRegionMonitor(this))
    , m_blockPressEventTimer(new QTimer(this))
    , m_lastWidget(nullptr)
    , m_platformWindowHandle(this)
    , m_innerBorderColor(QColor::Invalid)
    , m_dockInter(new PopupDBusDock("com.deepin.dde.daemon.Dock", "/com/deepin/dde/daemon/Dock", QDBusConnection::sessionBus(), this))
{
    compositeChanged();
    setWindowFlag(Qt::WindowStaysOnTopHint);
    if (Utils::IS_WAYLAND_DISPLAY) {
        // 谨慎修改层级，特别要注意对锁屏的影响
        setAttribute(Qt::WA_NativeWindow);
        if (tellWmDock && windowHandle()) {
            windowHandle()->setProperty("_d_dwayland_window-type", "dock");
        }
    } else {
        setAttribute(Qt::WA_InputMethodEnabled, false);
        if (noFocus) {
             // 这里需要覆盖窗口属性为Qt::X11BypassWindowManagerHint
             setWindowFlags(Qt::X11BypassWindowManagerHint);
        } else {
            // 不在任务栏显示图标
            setWindowFlag(Qt::Tool);
        }
    }

    if (noFocus) {
        setWindowFlag(Qt::WindowDoesNotAcceptFocus);
        setWindowFlag(Qt::WindowStaysOnTopHint);
    }

    // Fix: bug-211649, 242759 在该对象显示前调用DWindowManagerHelper::currentWorkspaceWindows会造成背景颜色异常，提前创建winId规避这个问题
    createWinId();

    connect(DWindowManagerHelper::instance(), &DWindowManagerHelper::hasBlurWindowChanged, this, &DockPopupWindow::compositeChanged);
    connect(m_dockInter, &PopupDBusDock::OpacityChanged, this, [this] {
        if (Utils::hasBlurWindow()) {
            updateWindowOpacity();
        }
    });
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &DockPopupWindow::updateBackgroundColor);

    // 阻止mouse press 事件后延时恢复，避免出现点击鼠标弹窗不消失的问题。
    // 因为不知道DRegionMonitor的 press 事件和qt 的 press 事件哪一个先收到
    // 故DRegionMonitor的的buttonPress 事件延后 100ms 处理，qt 的 press 事件延迟 200ms 重置，极限的情况也有 100ms 做缓冲。
    // 方法比较粗劣，有好办法欢迎修改👏
    m_blockPressEventTimer->setInterval(200);
    m_blockPressEventTimer->setSingleShot(true);
    connect(m_regionInter, &DRegionMonitor::buttonPress, this, [this] (const QPoint &p, const int flag) {
        QTimer::singleShot(100, this,  [this, p, flag](){
            onGlobMousePress(p, flag);
        });
    });

    setBlendMode(BlendMode::BehindWindowBlend);

    m_platformWindowHandle.setShadowOffset(QPoint(0, 4));
    m_platformWindowHandle.setShadowColor(QColor(0, 0, 0, 0.12 * 255));

    updateBackgroundColor();
}

DockPopupWindow::~DockPopupWindow()
{
}

void DockPopupWindow::setEnableSystemMove(bool enable)
{
    // 禁止窗口可以被鼠标拖动
    m_platformWindowHandle.setEnableSystemMove(enable);
}

bool DockPopupWindow::isModel() const
{
    return m_model;
}

void DockPopupWindow::setContent(QWidget* content)
{
    if (m_lastWidget) {
        m_lastWidget->removeEventFilter(this);
        m_lastWidget->setVisible(false);
    }

    content->installEventFilter(this);

    QAccessibleEvent event(this, QAccessible::NameChanged);
    QAccessible::updateAccessibility(&event);

    if (!content->objectName().trimmed().isEmpty())
        setAccessibleName(content->objectName() + "-popup");

    m_lastWidget = content;
    content->setParent(this);
    content->show();
    setFixedSize(content->size());
}

void DockPopupWindow::show(const QPoint& pos, const bool model)
{
    m_model = model;
    show(pos.x(), pos.y());

    if (m_regionInter->registered()) {
        m_regionInter->unregisterRegion();
    }

    if (m_model) {
        // 此处需要延迟注册，因为在wayland下，这里如果马上注册就会立刻触发m_regionInter的buttonPress信号（具体原因需要DRegionMonitor来分析）
        // 导致出现一系列问题，延迟注册后就不会触发buttonPress信号，保证流程正确
        QMetaObject::invokeMethod(m_regionInter, "registerRegion", Qt::QueuedConnection);
    }
}

void DockPopupWindow::show(const int x, const int y)
{
    m_lastPoint = QPoint(x, y);
    setFixedSize(m_lastWidget->size());

    QScreen* const screen = qApp->screenAt(m_lastPoint);
    if (!screen) {
        qCWarning(DOCK_APP) << "Cannot find screen the point is located, popup window not visible !" << m_lastPoint;
        DBlurEffectWidget::hide();
        return;
    }

    static const int MARGIN = 10; // 弹窗距离屏幕边缘的距离
    const QRect screenRect = screen->geometry();
    Dock::Position dockPos = qApp->property(PROP_POSITION).value<Dock::Position>();
    if (dockPos == Dock::Position::Top || dockPos == Dock::Position::Bottom) {
        int lRelativeX = x - screenRect.x() - (width() / 2); // 中心点距左边屏幕距离
        int rRelativeX = width() / 2 - (screenRect.width() - (x - screenRect.x())); // 中心点距右边屏幕距离
        int absoluteX = 0;
        if (lRelativeX < MARGIN) { // 超出 (屏幕左边 + MARGIN)
            absoluteX = screenRect.x() + MARGIN;
        } else if (rRelativeX > -MARGIN) { // 超出 (屏幕右边 - MARGIN)
            absoluteX = screenRect.x() + screenRect.width() - width() - MARGIN;
        } else {
            absoluteX = x - (width() / 2);
        }
        move(absoluteX, (dockPos == Dock::Position::Top) ? y : y - height());
    } else if (dockPos == Dock::Position::Left || dockPos == Dock::Position::Right) {
        int lRelativeY = y - screenRect.y() - height() / 2;
        int rRelativeY = y - screenRect.y() + height() / 2 - screenRect.height();
        int absoluteY = 0;
        if (lRelativeY < MARGIN) { // 超出 (屏幕上边 + MARGIN)
            absoluteY = screenRect.y() + MARGIN;
        } else if (rRelativeY > -MARGIN) { // 超出 (屏幕底边 - MARGIN)
            absoluteY = screenRect.y() + screenRect.height() - height() - MARGIN;
        } else {
            absoluteY = y - height() / 2;
        }
        move((dockPos == Dock::Position::Left) ? x : x - width(), absoluteY);
    } else {
        qCWarning(DOCK_APP) << "Dock position error, popup window display abnormality !" << dockPos;
        move(x, y);
    }

    DBlurEffectWidget::show();
}

void DockPopupWindow::hide()
{
    if (m_regionInter->registered())
        m_regionInter->unregisterRegion();

    DBlurEffectWidget::hide();
}

void DockPopupWindow::showEvent(QShowEvent* e)
{
    DBlurEffectWidget::showEvent(e);
    QTimer::singleShot(1, this, &DockPopupWindow::ensureRaised);
}

void DockPopupWindow::enterEvent(QEvent* e)
{
    DBlurEffectWidget::enterEvent(e);
    QTimer::singleShot(1, this, &DockPopupWindow::ensureRaised);
}

bool DockPopupWindow::eventFilter(QObject* o, QEvent* e)
{
    if (o != m_lastWidget || e->type() != QEvent::Resize)
        return false;

    // 情景：返回快捷面板后弹窗高度变小（比如从网络页面返回）/ 网络页面关闭网卡，弹窗缩小，
    // 此时DockPopupWindow收到 XEventMonitor 的 press 事件，判断鼠标不在弹窗内，然后隐藏了弹窗，与用户行为不符。
    // 故延时处理，暂时不响应 press 事件。
    m_blockPressEventTimer->start();
    // FIXME: ensure position move after global mouse release event
    if (isVisible()) {
        QTimer::singleShot(10, this, [=] {
            // NOTE(sbw): double check is necessary, in this time, the popup maybe already hided.
            if (isVisible())
                show(m_lastPoint, m_model);
        });
    }

    return false;
}

void DockPopupWindow::onGlobMousePress(const QPoint& mousePos, const int flag)
{
    Q_ASSERT(m_model);

    if (m_blockPressEventTimer->isActive()) {
        qCDebug(DOCK_APP) << "Press event is blocked, do not handle mouse press event";
        return;
    }

    if (!((flag == DRegionMonitor::WatchedFlags::Button_Left) || (flag == DRegionMonitor::WatchedFlags::Button_Right))) {
        return;
    }

    // 场景为点击快捷面板内的菜单，且鼠标在面板外面，点击菜单时弹窗收到了 mousePress 事件，进而隐藏了弹窗。
    // MENU_IS_VISIBLE_OR_JUST_HIDE 属性在点击菜单100ms后重置为 false
    if (qApp->property(MENU_IS_VISIBLE_OR_JUST_HIDE).toBool()) {
        qCInfo(DOCK_APP) << "Menu is visible or just hide, do not hide applet";
        return;
    }

    const QRect rect = QRect(pos(), size());
    if (rect.contains(mousePos)) {
        // 如果此时窗口没有激活，手动激活一下，否则用户无法输入
        if (!isActiveWindow() && !windowFlags().testFlag(Qt::WindowDoesNotAcceptFocus)) {
            activateWindow();
        }
        return;
    }

    // 如果点击的是屏幕键盘，则不隐藏
    QList<Dtk::Gui::DForeignWindow*> windowList = DWindowManagerHelper::instance()->currentWorkspaceWindows();
    for (auto window : windowList) {
        if (window->wmClass() == "onboard" && window->handle()->geometry().contains(scalePoint(mousePos)))
            return;
    }

    emit accept();

    // 隐藏后取消注册，不需要再次触发buttonPress信号
    m_regionInter->unregisterRegion();
}

void DockPopupWindow::compositeChanged()
{
    updateBackgroundColor();
}

void DockPopupWindow::ensureRaised()
{
    if (isVisible()) {
        QWidget* content = m_lastWidget;
        if (!content || !content->isVisible()) {
            this->setVisible(false);
        } else {
            raise();
            if (!windowFlags().testFlag(Qt::WindowDoesNotAcceptFocus))
                activateWindow();
        }
    }
}

/**
 * @brief 将缩放后的点的位置按照缩放比进行放大
 *  之所以这么复杂，是因为qt在多屏缩放的情况下，非首屏（即topLeft为(0,0)的屏幕）上的点坐标错误
 *  详见QTBUG-81695
 * @param point
 * @return QPoint
 */
QPoint DockPopupWindow::scalePoint(QPoint point)
{
    const qreal pixelRatio = qApp->devicePixelRatio();
    QScreen* const screen = qApp->screenAt(point);
    if (!screen) {
        qCWarning(DOCK_APP) << "Cannot find screen the point is located: " << point;
        return point;
    }

    const QRect& screenRect = screen->geometry();

    qreal pointX = point.x() * pixelRatio;
    if (screenRect.x() != 0) {
        pointX = (screenRect.x() / pixelRatio + point.x() - screenRect.x()) * pixelRatio;
    }

    qreal pointY = point.y() * pixelRatio;
    if (screenRect.y() != 0) {
        pointY = (screenRect.y() / pixelRatio + point.y() - screenRect.y()) * pixelRatio;
    }

    return QPoint(pointX, pointY);
}

void DockPopupWindow::blockMousePressEvent(bool block)
{
    m_blockPressEventTimer->start();
}

void DockPopupWindow::setPopupRadius(int radius)
{
    m_platformWindowHandle.setWindowRadius(Utils::hasBlurWindow() ? radius : 0);

    update();
}

void DockPopupWindow::updateBackgroundColor()
{
    QColor color;
    if (Utils::hasBlurWindow()) {
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
            color = QColor(238, 238, 238);
        } else {
            color = QColor(20, 20, 20);
        }
        setMaskColor(color);
        updateWindowOpacity();
    } else {
        setMaskColor(DBlurEffectWidget::AutoColor);
    }
}

void DockPopupWindow::updateWindowOpacity()
{
    const float dockValue = Utils::mapOpacityValue(m_dockInter->opacity());
    setMaskAlpha(quint8(dockValue * 255));
}
