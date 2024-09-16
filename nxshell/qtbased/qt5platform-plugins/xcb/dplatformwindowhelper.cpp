// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dplatformwindowhelper.h"
#include "dplatformintegration.h"
#include "dframewindow.h"
#include "vtablehook.h"
#include "dwmsupport.h"

#ifdef Q_OS_LINUX
#include "xcbnativeeventfilter.h"

#include "qxcbwindow.h"

#include <xcb/xcb_icccm.h>
#include <xcb/composite.h>
#include <xcb/damage.h>
#endif

#include <private/qwindow_p.h>
#include <private/qguiapplication_p.h>
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 2)
#include <private/qeventpoint_p.h>
#endif
#include <qpa/qplatformcursor.h>

#include <QPainterPath>

Q_DECLARE_METATYPE(QPainterPath)
Q_DECLARE_METATYPE(QMargins)

DPP_BEGIN_NAMESPACE

#define HOOK_VFPTR(Fun) VtableHook::overrideVfptrFun(window, &QPlatformWindow::Fun, this, &DPlatformWindowHelper::Fun)
#define CALL this->window()->QNativeWindow

PUBLIC_CLASS(QWindow, DPlatformWindowHelper);
PUBLIC_CLASS(QMouseEvent, DPlatformWindowHelper);
PUBLIC_CLASS(QDropEvent, DPlatformWindowHelper);
PUBLIC_CLASS(QNativeWindow, DPlatformWindowHelper);
PUBLIC_CLASS(QWheelEvent, DPlatformWindowHelper);

QHash<const QPlatformWindow*, DPlatformWindowHelper*> DPlatformWindowHelper::mapped;

DPlatformWindowHelper::DPlatformWindowHelper(QNativeWindow *window)
    : QObject(window->window())
    , m_nativeWindow(window)
{
    mapped[window] = this;

    m_frameWindow = new DFrameWindow(window->window());
    m_frameWindow->setFlags((window->window()->flags() | Qt::FramelessWindowHint | Qt::CustomizeWindowHint | Qt::NoDropShadowWindowHint) & ~Qt::WindowMinMaxButtonsHint);
    m_frameWindow->create();
    m_frameWindow->installEventFilter(this);
    m_frameWindow->setShadowRadius(getShadowRadius());
    m_frameWindow->setShadowColor(m_shadowColor);
    m_frameWindow->setShadowOffset(m_shadowOffset);
    m_frameWindow->setBorderWidth(getBorderWidth());
    m_frameWindow->setBorderColor(getBorderColor());
    m_frameWindow->setEnableSystemMove(m_enableSystemMove);
    m_frameWindow->setEnableSystemResize(m_enableSystemResize);
    // 防止被自动更新窗口大小(参见：qdeepintheme.cpp, https://github.com/linuxdeepin/qt5integration)
    m_frameWindow->setProperty("_d_disable_update_geometry_for_scale", true);

    window->setParent(m_frameWindow->handle());
    window->window()->installEventFilter(this);
    window->window()->setScreen(m_frameWindow->screen());
    window->window()->setProperty("_d_real_winId", window->winId());
    window->window()->setProperty(::frameMargins, QVariant::fromValue(m_frameWindow->contentMarginsHint()));

#ifdef Q_OS_LINUX
    if (windowRedirectContent(window->window())) {
        xcb_composite_redirect_window(window->xcb_connection(), window->xcb_window(), XCB_COMPOSITE_REDIRECT_MANUAL);
        damage_id = xcb_generate_id(window->xcb_connection());
        xcb_damage_create(window->xcb_connection(), damage_id, window->xcb_window(), XCB_DAMAGE_REPORT_LEVEL_NON_EMPTY);
    }
#endif

    updateClipPathByWindowRadius(window->window()->size());

    updateClipPathFromProperty();
    updateFrameMaskFromProperty();
    updateWindowRadiusFromProperty();
    updateBorderWidthFromProperty();
    updateBorderColorFromProperty();
    updateShadowRadiusFromProperty();
    updateShadowOffsetFromProperty();
    updateShadowColorFromProperty();
    updateEnableSystemResizeFromProperty();
    updateEnableSystemMoveFromProperty();
    updateEnableBlurWindowFromProperty();
    updateWindowBlurAreasFromProperty();
    updateWindowBlurPathsFromProperty();
    updateAutoInputMaskByClipPathFromProperty();

    HOOK_VFPTR(setGeometry);
    HOOK_VFPTR(geometry);
    HOOK_VFPTR(normalGeometry);
    HOOK_VFPTR(frameMargins);
    HOOK_VFPTR(setVisible);
    HOOK_VFPTR(setWindowFlags);
    HOOK_VFPTR(setWindowState);
    HOOK_VFPTR(winId);
    HOOK_VFPTR(setParent);
    HOOK_VFPTR(setWindowTitle);
    HOOK_VFPTR(setWindowFilePath);
    HOOK_VFPTR(setWindowIcon);
    HOOK_VFPTR(raise);
    HOOK_VFPTR(lower);
//    HOOK_VFPTR(isExposed);
    HOOK_VFPTR(isEmbedded);
    HOOK_VFPTR(setOpacity);
    HOOK_VFPTR(propagateSizeHints);
    HOOK_VFPTR(requestActivateWindow);
//    HOOK_VFPTR(setKeyboardGrabEnabled);
//    HOOK_VFPTR(setMouseGrabEnabled);
    HOOK_VFPTR(setWindowModified);
//    HOOK_VFPTR(windowEvent);
    HOOK_VFPTR(startSystemResize);
    HOOK_VFPTR(setFrameStrutEventsEnabled);
    HOOK_VFPTR(frameStrutEventsEnabled);
    HOOK_VFPTR(setAlertState);
    HOOK_VFPTR(isAlertState);

    connect(m_frameWindow, &DFrameWindow::contentMarginsHintChanged,
            this, &DPlatformWindowHelper::onFrameWindowContentMarginsHintChanged);
    connect(DWMSupport::instance(), &DXcbWMSupport::hasCompositeChanged,
            this, &DPlatformWindowHelper::onWMHasCompositeChanged);
    connect(DWMSupport::instance(), &DXcbWMSupport::windowManagerChanged,
            this, &DPlatformWindowHelper::updateWindowBlurAreasForWM);
    // 必须保证两个窗口QScreen对象一致
    connect(m_frameWindow, &DFrameWindow::screenChanged,
            this, &DPlatformWindowHelper::onScreenChanged);
    connect(window->window(), &QWindow::screenChanged,
            m_frameWindow, &DFrameWindow::setScreen);
    connect(m_frameWindow, &DFrameWindow::contentOrientationChanged,
            window->window(), &QWindow::reportContentOrientationChange);

    static_cast<QPlatformWindow*>(window)->propagateSizeHints();
}

DPlatformWindowHelper::~DPlatformWindowHelper()
{
    mapped.remove(m_nativeWindow);
    m_frameWindow->deleteLater();

#ifdef Q_OS_LINUX
    // clear damage
    xcb_damage_destroy(DPlatformIntegration::xcbConnection()->xcb_connection(), damage_id);
#endif
}

DPlatformWindowHelper *DPlatformWindowHelper::me() const
{
    return DPlatformWindowHelper::mapped.value(window());
}

void DPlatformWindowHelper::setGeometry(const QRect &rect)
{
    DPlatformWindowHelper *helper = me();

    qreal device_pixel_ratio = helper->m_frameWindow->devicePixelRatio();

    // update clip path
    helper->updateClipPathByWindowRadius(rect.size() / device_pixel_ratio);

    const QMargins &content_margins = helper->m_frameWindow->contentMarginsHint() * device_pixel_ratio;

    qt_window_private(helper->m_frameWindow)->positionAutomatic = qt_window_private(helper->m_nativeWindow->window())->positionAutomatic;
    helper->m_frameWindow->handle()->setGeometry(rect + content_margins);
    // NOTE(zccrs): 此处必须要更新内容窗口的大小，因为frame窗口大小改变后可能不会触发resize事件调用updateContentWindowGeometry()
    //              就会导致内容窗口大小不对，此问题可在文件管理器复制文件对话框重现（多试几次）
    helper->setNativeWindowGeometry(rect, true);
    helper->m_nativeWindow->QPlatformWindow::setGeometry(rect);
}

QRect DPlatformWindowHelper::geometry() const
{
    DPlatformWindowHelper *helper = me();
    const QRect &geometry = helper->m_frameWindow->handle()->geometry();

    if (geometry.topLeft() == QPoint(0, 0) && geometry.size() == QSize(0, 0))
        return geometry;

    QRect rect = geometry - helper->m_frameWindow->contentMarginsHint() * helper->m_frameWindow->devicePixelRatio();

    rect.setSize(helper->m_nativeWindow->QNativeWindow::geometry().size());

    return rect;
}

QRect DPlatformWindowHelper::normalGeometry() const
{
    return me()->m_frameWindow->handle()->normalGeometry();
}

QMargins DPlatformWindowHelper::frameMargins() const
{
    return me()->m_frameWindow->handle()->frameMargins();
}

QWindow *topvelWindow(QWindow *w)
{
    QWindow *tw = w;

    while (tw->parent())
        tw = tw->parent();

    DPlatformWindowHelper *helper = DPlatformWindowHelper::mapped.value(tw->handle());

    return helper ? helper->m_frameWindow : tw;
}

void DPlatformWindowHelper::setVisible(bool visible)
{
    DPlatformWindowHelper *helper = me();

    if (visible) {
        QWindow *tp = helper->m_nativeWindow->window()->transientParent();
        helper->m_nativeWindow->window()->setTransientParent(helper->m_frameWindow);

        if (tp) {
            QWindow *tw = topvelWindow(tp);

            if (tw != helper->m_frameWindow)
                helper->m_frameWindow->setTransientParent(tw);
        }

#ifdef Q_OS_LINUX
        // reupdate _MOTIF_WM_HINTS
        DQNativeWindow *window = static_cast<DQNativeWindow*>(helper->m_frameWindow->handle());

        Utility::QtMotifWmHints mwmhints = Utility::getMotifWmHints(window->m_window);

        if (window->window()->modality() != Qt::NonModal) {
            switch (window->window()->modality()) {
            case Qt::WindowModal:
                mwmhints.input_mode = DXcbWMSupport::MWM_INPUT_PRIMARY_APPLICATION_MODAL;
                break;
            case Qt::ApplicationModal:
            default:
                mwmhints.input_mode = DXcbWMSupport::MWM_INPUT_FULL_APPLICATION_MODAL;
                break;
            }
            mwmhints.flags |= DXcbWMSupport::MWM_HINTS_INPUT_MODE;
        } else {
            mwmhints.input_mode = DXcbWMSupport::MWM_INPUT_MODELESS;
            mwmhints.flags &= ~DXcbWMSupport::MWM_HINTS_INPUT_MODE;
        }

        QWindow *content_window = helper->m_nativeWindow->window();
        Utility::QtMotifWmHints cw_hints = Utility::getMotifWmHints(helper->m_nativeWindow->QNativeWindow::winId());
        bool size_fixed = content_window->minimumSize() == content_window->maximumSize();

        if (size_fixed) {
            // fixed size, remove the resize handle (since mwm/dtwm
            // isn't smart enough to do it itself)
            mwmhints.flags |= DXcbWMSupport::MWM_HINTS_FUNCTIONS;
            if (mwmhints.functions & DXcbWMSupport::MWM_FUNC_ALL) {
                mwmhints.functions = DXcbWMSupport::MWM_FUNC_MOVE;
            } else {
                mwmhints.functions &= ~DXcbWMSupport::MWM_FUNC_RESIZE;
            }

            if (mwmhints.decorations & DXcbWMSupport::MWM_DECOR_ALL) {
                mwmhints.flags |= DXcbWMSupport::MWM_HINTS_DECORATIONS;
                mwmhints.decorations = (DXcbWMSupport::MWM_DECOR_BORDER
                                        | DXcbWMSupport::MWM_DECOR_TITLE
                                        | DXcbWMSupport::MWM_DECOR_MENU);
            } else {
                mwmhints.decorations &= ~DXcbWMSupport::MWM_DECOR_RESIZEH;
            }

            // set content window decoration hints
            cw_hints.flags |= DXcbWMSupport::MWM_HINTS_DECORATIONS;
            cw_hints.decorations = DXcbWMSupport::MWM_DECOR_MINIMIZE;
        }

        if (content_window->flags() & Qt::WindowMinimizeButtonHint) {
            mwmhints.functions |= DXcbWMSupport::MWM_FUNC_MINIMIZE;

            cw_hints.decorations |= DXcbWMSupport::MWM_DECOR_MINIMIZE;
        }
        if (content_window->flags() & Qt::WindowMaximizeButtonHint) {
            mwmhints.functions |= DXcbWMSupport::MWM_FUNC_MAXIMIZE;

            if (!size_fixed)
                cw_hints.decorations |= DXcbWMSupport::MWM_DECOR_MAXIMIZE;
        }
        if (content_window->flags() & Qt::WindowCloseButtonHint) {
            mwmhints.functions |= DXcbWMSupport::MWM_FUNC_CLOSE;
        }
        if (content_window->flags() & Qt::WindowTitleHint) {
            cw_hints.decorations |= DXcbWMSupport::MWM_DECOR_TITLE;
        }
        if (content_window->flags() & Qt::WindowSystemMenuHint) {
            cw_hints.decorations |= DXcbWMSupport::MWM_DECOR_MENU;
        }
#endif

        helper->m_frameWindow->setVisible(visible);
        helper->updateContentWindowGeometry();
        helper->m_nativeWindow->QNativeWindow::setVisible(visible);
        helper->updateWindowBlurAreasForWM();

        // restore
        if (tp) {
            helper->m_nativeWindow->window()->setTransientParent(tp);
        }
#ifdef Q_OS_LINUX
        else {
            xcb_delete_property(window->xcb_connection(), window->m_window, XCB_ATOM_WM_TRANSIENT_FOR);
        }

        // Fix the window can't show minimized if window is fixed size
        Utility::setMotifWmHints(window->m_window, mwmhints);
        Utility::setMotifWmHints(helper->m_nativeWindow->QNativeWindow::winId(), cw_hints);

        if (helper->m_nativeWindow->window()->modality() != Qt::NonModal) {
#if QT_VERSION < QT_VERSION_CHECK(5, 12, 1)
            window->setNetWmStates(window->netWmStates() | QNativeWindow::NetWmStateModal);
#else
            window->setNetWmState(true, window->atom(QXcbAtom::D_QXCBATOM_WRAPPER(_NET_WM_STATE_MODAL)));
#endif
        }
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
        // 当Qt版本在5.9及以上时, 如果窗口设置了BypassWindowManagerHint标志, 窗口就无法通过鼠标点击获得焦点
        if (helper->m_nativeWindow->window()->flags().testFlag(Qt::BypassWindowManagerHint)
                && QGuiApplication::modalWindow() == helper->m_nativeWindow->window()) {
            helper->m_nativeWindow->requestActivateWindow();
        }
#endif

        return;
    }

    helper->m_frameWindow->setVisible(visible);
    helper->m_nativeWindow->QNativeWindow::setVisible(visible);
    helper->updateWindowBlurAreasForWM();
}

void DPlatformWindowHelper::setWindowFlags(Qt::WindowFlags flags)
{
    me()->m_frameWindow->setFlags((flags | Qt::FramelessWindowHint | Qt::CustomizeWindowHint | Qt::NoDropShadowWindowHint) & ~Qt::WindowMinMaxButtonsHint);
    window()->QNativeWindow::setWindowFlags(flags);
}

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
void DPlatformWindowHelper::setWindowState(Qt::WindowState state)
#else
void DPlatformWindowHelper::setWindowState(Qt::WindowStates state)
#endif
{
#ifdef Q_OS_LINUX
    DQNativeWindow *window = static_cast<DQNativeWindow*>(me()->m_frameWindow->handle());

    if (window->m_windowState == state)
        return;

    if (state == Qt::WindowMinimized
            && (window->m_windowState == Qt::WindowMaximized
                || window->m_windowState == Qt::WindowFullScreen)) {
#if QT_VERSION < QT_VERSION_CHECK(5, 12, 1)
        window->changeNetWmState(true, Utility::internAtom("_NET_WM_STATE_HIDDEN"));
#else
        window->setNetWmState(true, Utility::internAtom("_NET_WM_STATE_HIDDEN"));
#endif
        Utility::XIconifyWindow(window->connection()->xlib_display(),
                                window->m_window,
                                window->connection()->primaryScreenNumber());
        window->connection()->sync();
        window->m_windowState = state;
    } else
#endif
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
        me()->m_frameWindow->setWindowState(state);
#else
        me()->m_frameWindow->setWindowStates(state);
#endif
    }
}

WId DPlatformWindowHelper::winId() const
{
    return me()->m_frameWindow->handle()->winId();
}

void DPlatformWindowHelper::setParent(const QPlatformWindow *window)
{
    me()->m_frameWindow->handle()->setParent(window);
}

void DPlatformWindowHelper::setWindowTitle(const QString &title)
{
    me()->m_frameWindow->handle()->setWindowTitle(title);
}

void DPlatformWindowHelper::setWindowFilePath(const QString &title)
{
    me()->m_frameWindow->handle()->setWindowFilePath(title);
}

void DPlatformWindowHelper::setWindowIcon(const QIcon &icon)
{
    me()->m_frameWindow->handle()->setWindowIcon(icon);
}

void DPlatformWindowHelper::raise()
{
    me()->m_frameWindow->handle()->raise();
}

void DPlatformWindowHelper::lower()
{
    me()->m_frameWindow->handle()->lower();
}

bool DPlatformWindowHelper::isExposed() const
{
    return me()->m_frameWindow->handle()->isExposed();
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
bool DPlatformWindowHelper::isEmbedded() const
{
    return me()->m_frameWindow->handle()->isEmbedded();
}
#else
bool DPlatformWindowHelper::isEmbedded(const QPlatformWindow *parentWindow) const
{
    return me()->m_frameWindow->handle()->isEmbedded(parentWindow);
}
#endif

void DPlatformWindowHelper::propagateSizeHints()
{
    me()->updateSizeHints();

    const QWindow *window = this->window()->window();

    if (window->maximumSize() == window->minimumSize()) {
        Utility::QtMotifWmHints cw_hints = Utility::getMotifWmHints(this->window()->QNativeWindow::winId());

        cw_hints.flags |= DXcbWMSupport::MWM_HINTS_DECORATIONS;
        cw_hints.decorations = DXcbWMSupport::MWM_DECOR_MINIMIZE;

        if (window->flags() & Qt::WindowTitleHint) {
            cw_hints.decorations |= DXcbWMSupport::MWM_DECOR_TITLE;
        }
        if (window->flags() & Qt::WindowSystemMenuHint) {
            cw_hints.decorations |= DXcbWMSupport::MWM_DECOR_MENU;
        }

        Utility::setMotifWmHints(this->window()->QNativeWindow::winId(), cw_hints);
    }
}

void DPlatformWindowHelper::setOpacity(qreal level)
{
    me()->m_frameWindow->setOpacity(level);
}

void DPlatformWindowHelper::requestActivateWindow()
{
    DPlatformWindowHelper *helper = me();

#ifdef Q_OS_LINUX
    // 窗口管理器不支持混成时，最小化窗口会导致窗口被unmap
    if (!DXcbWMSupport::instance()->hasComposite() && helper->m_frameWindow->windowState() == Qt::WindowMinimized) {
#ifdef Q_XCB_CALL
        Q_XCB_CALL(xcb_map_window(DPlatformIntegration::xcbConnection()->xcb_connection(), helper->m_frameWindow->winId()));
#else
        xcb_map_window(DPlatformIntegration::xcbConnection()->xcb_connection(), helper->m_frameWindow->winId());
#endif
    }
#endif

    helper->m_frameWindow->handle()->requestActivateWindow();
#ifdef Q_OS_LINUX
    // 对于有parent的窗口，需要调用此接口让其获得输入焦点
    xcb_set_input_focus(DPlatformIntegration::xcbConnection()->xcb_connection(),
                        XCB_INPUT_FOCUS_PARENT, helper->m_nativeWindow->QNativeWindow::winId(),
                        DPlatformIntegration::xcbConnection()->time());
#endif
}

bool DPlatformWindowHelper::setKeyboardGrabEnabled(bool grab)
{
    return me()->m_frameWindow->handle()->setKeyboardGrabEnabled(grab);
}

bool DPlatformWindowHelper::setMouseGrabEnabled(bool grab)
{
    return me()->m_frameWindow->handle()->setMouseGrabEnabled(grab);
}

bool DPlatformWindowHelper::setWindowModified(bool modified)
{
    return me()->m_frameWindow->handle()->setWindowModified(modified);
}

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
bool DPlatformWindowHelper::startSystemResize(const QPoint &pos, Qt::Corner corner)
{
    return me()->m_frameWindow->handle()->startSystemResize(pos, corner);
}
#else
bool DPlatformWindowHelper::startSystemResize(Qt::Edges edges)
{
    return me()->m_frameWindow->handle()->startSystemResize(edges);
}
#endif

void DPlatformWindowHelper::setFrameStrutEventsEnabled(bool enabled)
{
    me()->m_frameWindow->handle()->setFrameStrutEventsEnabled(enabled);
}

bool DPlatformWindowHelper::frameStrutEventsEnabled() const
{
    return me()->m_frameWindow->handle()->frameStrutEventsEnabled();
}

void DPlatformWindowHelper::setAlertState(bool enabled)
{
    me()->m_frameWindow->handle()->setAlertState(enabled);
}

bool DPlatformWindowHelper::isAlertState() const
{
    return me()->m_frameWindow->handle()->isAlertState();
}

bool DPlatformWindowHelper::windowRedirectContent(QWindow *window)
{
    // 环境变量的值最优先
    static const QByteArray env = qgetenv("DXCB_REDIRECT_CONTENT");

    if (env == "true") {
        return true;
    } else if (env == "false") {
        return false;
    }

    // 判断在2D模式下是否允许重定向窗口绘制的内容，此环境变量默认不设置，因此默认需要禁用2D下的重定向
    // 修复dde-dock在某些2D环境（如HW云桌面）中不显示窗口内容
    if (!DXcbWMSupport::instance()->hasComposite()
            && qEnvironmentVariableIsEmpty("DXCB_REDIRECT_CONTENT_WITH_NO_COMPOSITE")) {
        return false;
    }

    const QVariant &value = window->property(redirectContent);

    if (value.type() == QVariant::Bool)
        return value.toBool();

    return window->surfaceType() == QSurface::OpenGLSurface;
}

bool DPlatformWindowHelper::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_frameWindow) {
        switch ((int)event->type()) {
        case QEvent::Close:
            m_nativeWindow->window()->close();
            return true;
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        case QEvent::WindowDeactivate:
            QCoreApplication::sendEvent(m_nativeWindow->window(), event);
            return true;
        case QEvent::Move: {
            QRect geometry = m_frameWindow->handle()->geometry();

            if (geometry.topLeft() != QPoint(0, 0) || geometry.size() != QSize(0, 0)) {
                geometry.translate(-m_frameWindow->contentOffsetHint());
                geometry.setSize(m_nativeWindow->QNativeWindow::geometry().size());
            }

            m_nativeWindow->QPlatformWindow::setGeometry(geometry);
            QWindowSystemInterface::handleGeometryChange(m_nativeWindow->window(), geometry);
            return true;
        }
        case QEvent::FocusIn:
#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
            QWindowSystemInterface::handleWindowActivated(m_nativeWindow->window(), static_cast<QFocusEvent*>(event)->reason());
#else
            QWindowSystemInterface::handleFocusWindowChanged(m_nativeWindow->window(), static_cast<QFocusEvent*>(event)->reason());
#endif
            return true;
        case QEvent::WindowActivate:
#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
            QWindowSystemInterface::handleWindowActivated(m_nativeWindow->window(), Qt::OtherFocusReason);
#else
            QWindowSystemInterface::handleFocusWindowChanged(m_nativeWindow->window(), Qt::OtherFocusReason);
#endif
            return true;
        case QEvent::Resize: {
            updateContentWindowGeometry();
            break;
        }
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseMove: {
            DQMouseEvent *e = static_cast<DQMouseEvent*>(event);
            const QRectF rectF(m_windowValidGeometry);
            const QPointF posF(e->localPos() - m_frameWindow->contentOffsetHint());

            // QRectF::contains中判断时加入了右下边界
            if (!qFuzzyCompare(posF.x(), rectF.width())
                    && !qFuzzyCompare(posF.y(), rectF.height())
                    && rectF.contains(posF)) {
                m_frameWindow->unsetCursor();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                e->l = e->w = m_nativeWindow->window()->mapFromGlobal(e->globalPos());
                qApp->sendEvent(m_nativeWindow->window(), e);
#elif QT_VERSION <= QT_VERSION_CHECK(6, 2, 4)
                QScopedPointer<QMutableSinglePointEvent> mevent(QMutableSinglePointEvent::from(e->clone()));
                mevent->mutablePoint().setPosition(m_nativeWindow->window()->mapFromGlobal(e->globalPosition()));
                mevent->mutablePoint().setScenePosition(m_nativeWindow->window()->mapFromGlobal(e->globalPosition()));
                qApp->sendEvent(m_nativeWindow->window(), mevent.data());
#else
                QScopedPointer<QMutableSinglePointEvent> mevent(QMutableSinglePointEvent::from(e->clone()));
                QMutableEventPoint::setPosition(mevent->point(0), m_nativeWindow->window()->mapFromGlobal(e->globalPosition()));
                QMutableEventPoint::setScenePosition(mevent->point(0), m_nativeWindow->window()->mapFromGlobal(e->globalPosition()));
                qApp->sendEvent(m_nativeWindow->window(), mevent.data());
#endif
                return true;
            }

            break;
        }
        case QEvent::WindowStateChange: {
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
            Qt::WindowState old_state = qt_window_private(m_nativeWindow->window())->windowState;
            Qt::WindowState new_state = m_frameWindow->windowState();
#else
            Qt::WindowStates old_state = qt_window_private(m_nativeWindow->window())->windowState;
            Qt::WindowStates new_state = m_frameWindow->windowState();
#endif

            qt_window_private(m_nativeWindow->window())->windowState = new_state;
            QCoreApplication::sendEvent(m_nativeWindow->window(), event);
            updateClipPathByWindowRadius(m_nativeWindow->window()->size());

            // need repaint content window
            if ((old_state == Qt::WindowFullScreen || old_state == Qt::WindowMaximized)
                    && new_state == Qt::WindowNoState) {
                if (m_frameWindow->m_paintShadowOnContentTimerId < 0 && m_frameWindow->m_contentBackingStore) {
                    m_frameWindow->m_paintShadowOnContentTimerId = m_frameWindow->startTimer(0, Qt::PreciseTimer);
                }
            }
            break;
        }
        case QEvent::DragEnter:
        case QEvent::DragMove:
        case QEvent::Drop: {
            DQDropEvent *e = static_cast<DQDropEvent*>(event);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            e->m_pos -= m_frameWindow->contentOffsetHint();
#else
            e->p -= m_frameWindow->contentOffsetHint();
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
            Q_FALLTHROUGH();
#endif
        } // deliberate
        case QEvent::DragLeave:
            QCoreApplication::sendEvent(m_nativeWindow->window(), event);
            return true;
        case QEvent::PlatformSurface: {
            const QPlatformSurfaceEvent *e = static_cast<QPlatformSurfaceEvent*>(event);

            if (e->surfaceEventType() == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed)
                m_nativeWindow->window()->destroy();

            break;
        }
        case QEvent::Expose: {
            // ###(zccrs): 在频繁切换窗口管理器时，可能会出现frame窗口被外部（可能是窗管）调用map
            //             但是content窗口还是隐藏状态，所以在此做状态检查和同步
            if (m_frameWindow->handle()->isExposed() && !m_nativeWindow->isExposed()) {
                m_nativeWindow->setVisible(true);
            }
        }
        default: break;
        }
    } else if (watched == m_nativeWindow->window()) {
        switch ((int)event->type()) {
        case QEvent::MouseMove: {
            if (qApp->mouseButtons() != Qt::LeftButton)
                break;

            static QEvent *last_event = NULL;

            if (last_event == event) {
                last_event = NULL;

                return false;
            }

            last_event = event;
            QCoreApplication::sendEvent(watched, event);

            if (!event->isAccepted()) {
                DQMouseEvent *e = static_cast<DQMouseEvent*>(event);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                e->l = e->w = m_frameWindow->mapFromGlobal(e->globalPos());
                QGuiApplicationPrivate::setMouseEventSource(e, Qt::MouseEventSynthesizedByQt);
                m_frameWindow->mouseMoveEvent(e);
#elif QT_VERSION <= QT_VERSION_CHECK(6, 2, 4)
                QScopedPointer<QMutableSinglePointEvent> mevent(QMutableSinglePointEvent::from(e->clone()));
                mevent->mutablePoint().setPosition(m_frameWindow->mapFromGlobal(e->globalPosition()));
                mevent->mutablePoint().setScenePosition(m_frameWindow->mapFromGlobal(e->globalPosition()));
                qApp->sendEvent(m_frameWindow, mevent.data());
#else
                QScopedPointer<QMutableSinglePointEvent> mevent(QMutableSinglePointEvent::from(e->clone()));
                QMutableEventPoint::setPosition(mevent->point(0), m_frameWindow->mapFromGlobal(e->globalPosition()));
                QMutableEventPoint::setScenePosition(mevent->point(0), m_frameWindow->mapFromGlobal(e->globalPosition()));
                qApp->sendEvent(m_frameWindow, mevent.data());
#endif
                return true;
            }
            break;
        }
#if QT_VERSION > QT_VERSION_CHECK(5, 6, 1)
        // NOTE(zccrs): https://codereview.qt-project.org/#/c/162027/ (QTBUG-53993)
        case QEvent::MouseButtonPress: {
            QWindow *w = m_nativeWindow->window();

            if (w->flags().testFlag(Qt::BypassWindowManagerHint)) {
                const QMouseEvent *me = static_cast<QMouseEvent*>(event);

                if (me->button() == Qt::LeftButton && w != QGuiApplication::focusWindow()) {
                    if (!(w->flags() & (Qt::WindowDoesNotAcceptFocus))
                            && w->type() != Qt::ToolTip
                            && w->type() != Qt::Popup) {
                        w->requestActivate();
                    }
                }
            }

            break;
        }
#endif
        case QEvent::MouseButtonRelease: {
            if (m_frameWindow->m_isSystemMoveResizeState) {
                Utility::cancelWindowMoveResize(Utility::getNativeTopLevelWindow(m_frameWindow->winId()));
                m_frameWindow->m_isSystemMoveResizeState = false;
            }

            break;
        }
        case QEvent::PlatformSurface: {
            const QPlatformSurfaceEvent *e = static_cast<QPlatformSurfaceEvent*>(event);

            if (e->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated)
                m_frameWindow->create();

            break;
        }
        case QEvent::Resize:
            if (m_isUserSetClipPath) {
                // 窗口大小改变时要强制更新模糊区域，所以需要将第二个参数传递为true
                setWindowValidGeometry(m_clipPath.boundingRect().toRect() & QRect(QPoint(0, 0), static_cast<QResizeEvent*>(event)->size()), true);
            } else {
                updateClipPathByWindowRadius(static_cast<QResizeEvent*>(event)->size());
            }
            break;
// ###(zccrs): 在9b1a28e6这个提交中因为调用了内部窗口的setVisible，所以需要过滤掉visible为false时产生的不必要的Leave事件
//             这段代码会引起窗口被其它窗口覆盖时的Leave事件丢失
//             因为已经移除了setVisible相关的代码，故先注释掉这部分代码，看是否有不良影响
//        case QEvent::Leave: {
//            QWindow::Visibility visibility = m_nativeWindow->window()->visibility();

//            if (visibility == QWindow::Hidden || visibility == QWindow::Minimized || !m_nativeWindow->window()->isActive())
//                break;

//            const QPoint &pos = Utility::translateCoordinates(QPoint(0, 0), m_nativeWindow->winId(),
//                                                              DPlatformIntegration::instance()->defaultConnection()->rootWindow());
//            const QPoint &cursor_pos = qApp->primaryScreen()->handle()->cursor()->pos();

//            return m_clipPath.contains(QPointF(cursor_pos - pos) / m_nativeWindow->window()->devicePixelRatio());
//        }
#ifdef Q_OS_LINUX
        case QEvent::Wheel: {
            // ###(zccrs): Qt的bug，QWidgetWindow往QWidget中转发QWheelEvent时没有调用setTimestamp
            //             暂时使用这种低等的方案绕过此问题
            DQWheelEvent *ev = static_cast<DQWheelEvent*>(event);
            const DeviceType type = DPlatformIntegration::instance()->eventFilter()->xiEventSource(ev);

            if (type != UnknowDevice)
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
                ev->m_mouseState |= Qt::MaxMouseButton | Qt::MouseButton(type);
#else
                ev->mouseState |= Qt::MaxMouseButton | Qt::MouseButton(type);
#endif
            break;
        }
#endif
        default: break;
        }
    }

    return false;
}

void DPlatformWindowHelper::setNativeWindowGeometry(const QRect &rect, bool onlyResize)
{
    qt_window_private(m_nativeWindow->window())->parentWindow = m_frameWindow;
    qt_window_private(m_nativeWindow->window())->positionAutomatic = onlyResize;
    // 对于非顶层窗口，Qt会根据新的geometry获取窗口所在屏幕，并将屏幕设置给此窗口
    // 因此此处设置native window对于的parent window，避免它被判断为顶层窗口。
    // note: 有父窗口的窗口则会使用父窗口的screen对象。
    QWindowPrivate::get(m_nativeWindow->window())->parentWindow = m_frameWindow;
    m_nativeWindow->QNativeWindow::setGeometry(rect);
    QWindowPrivate::get(m_nativeWindow->window())->parentWindow = nullptr;
    qt_window_private(m_nativeWindow->window())->parentWindow = 0;
    qt_window_private(m_nativeWindow->window())->positionAutomatic = false;
    updateWindowNormalHints();
}

void DPlatformWindowHelper::updateClipPathByWindowRadius(const QSize &windowSize)
{
    if (!m_isUserSetClipPath) {
        // 第二个参数传递true，强制更新模糊区域
        setWindowValidGeometry(QRect(QPoint(0, 0), windowSize), true);

        int window_radius = getWindowRadius();

        QPainterPath path;

        path.addRoundedRect(m_windowValidGeometry, window_radius, window_radius);

        setClipPath(path);
    }
}

void DPlatformWindowHelper::setClipPath(const QPainterPath &path)
{
    if (m_clipPath == path)
        return;

    m_clipPath = path;

    if (m_isUserSetClipPath) {
        setWindowValidGeometry(m_clipPath.boundingRect().toRect() & QRect(QPoint(0, 0), m_nativeWindow->window()->size()));
    }

    updateWindowShape();
    updateWindowBlurAreasForWM();
    updateContentPathForFrameWindow();
}

void DPlatformWindowHelper::setWindowValidGeometry(const QRect &geometry, bool force)
{
    if (!force && geometry == m_windowValidGeometry)
        return;

    m_windowValidGeometry = geometry;

    // The native window geometry may not update now, we need to wait for resize
    // event to proceed.
    QTimer::singleShot(0, this, &DPlatformWindowHelper::updateWindowBlurAreasForWM);
}

bool DPlatformWindowHelper::updateWindowBlurAreasForWM()
{
    // NOTE(zccrs): 当窗口unmap时清理模糊区域的属性，否则可能会导致窗口已经隐藏，但模糊区域没有消失的问题。因为窗口管理器不是绝对根据窗口是否map来绘制
    //              模糊背景，当窗口unmap但是窗口的WM State值为Normal时也会绘制模糊背景（这种情况可能出现在连续多次快速切换窗口管理器时）
    if (!m_nativeWindow->window()->isVisible() || (!m_enableBlurWindow && m_blurAreaList.isEmpty() && m_blurPathList.isEmpty())) {
        Utility::clearWindowBlur(m_frameWindow->winId());

        return true;
    }

    qreal device_pixel_ratio = m_nativeWindow->window()->devicePixelRatio();
    const QRect &windowValidRect = m_windowValidGeometry * device_pixel_ratio;

    if (windowValidRect.isEmpty() || !m_nativeWindow->window()->isVisible())
        return false;

    quint32 top_level_w = Utility::getNativeTopLevelWindow(m_frameWindow->winId());
    QPoint offset = m_frameWindow->contentOffsetHint() * device_pixel_ratio;

    if (top_level_w != m_frameWindow->winId()) {
        offset += Utility::translateCoordinates(QPoint(0, 0), m_frameWindow->winId(), top_level_w);
    }

    QVector<Utility::BlurArea> newAreas;

    if (m_enableBlurWindow) {
        if (m_isUserSetClipPath) {
            QList<QPainterPath> list;

            list << (m_clipPath * device_pixel_ratio).translated(offset);

            return Utility::blurWindowBackgroundByPaths(top_level_w, list);
        }

        Utility::BlurArea area;

        area.x = windowValidRect.x() + offset.x();
        area.y = windowValidRect.y() + offset.y();
        area.width = windowValidRect.width();
        area.height = windowValidRect.height();
        area.xRadius = getWindowRadius() * device_pixel_ratio;
        area.yRaduis = getWindowRadius() * device_pixel_ratio;

        newAreas.append(std::move(area));

        return Utility::blurWindowBackground(top_level_w, newAreas);
    }

    QPainterPath window_valid_path;

    window_valid_path.addRect(QRect(offset, m_nativeWindow->QPlatformWindow::geometry().size()));
    window_valid_path &= (m_clipPath * device_pixel_ratio).translated(offset);

    if (m_blurPathList.isEmpty()) {
        if (m_blurAreaList.isEmpty())
            return true;

        newAreas.reserve(m_blurAreaList.size());

        foreach (Utility::BlurArea area, m_blurAreaList) {
            area *= device_pixel_ratio;

            area.x += offset.x();
            area.y += offset.y();

            QPainterPath path;

            path.addRoundedRect(area.x, area.y, area.width, area.height,
                                area.xRadius, area.yRaduis);

            if (!window_valid_path.contains(path)) {
                const QPainterPath valid_blur_path = window_valid_path & path;
                const QRectF valid_blur_rect = valid_blur_path.boundingRect();

                if (path.boundingRect() != valid_blur_rect) {
                    area.x = valid_blur_rect.x();
                    area.y = valid_blur_rect.y();
                    area.width = valid_blur_rect.width();
                    area.height = valid_blur_rect.height();

                    path = QPainterPath();
                    path.addRoundedRect(valid_blur_rect.x(), valid_blur_rect.y(),
                                        valid_blur_rect.width(), valid_blur_rect.height(),
                                        area.xRadius, area.yRaduis);

                    if (valid_blur_path != path) {
                        break;
                    }
                } else if (valid_blur_path != path) {
                    break;
                }
            }

            newAreas.append(std::move(area));
        }

        if (newAreas.size() == m_blurAreaList.size())
            return Utility::blurWindowBackground(top_level_w, newAreas);
    }

    QList<QPainterPath> newPathList;

    newPathList.reserve(m_blurAreaList.size());

    foreach (Utility::BlurArea area, m_blurAreaList) {
        QPainterPath path;

        area *= device_pixel_ratio;
        path.addRoundedRect(area.x + offset.x(), area.y + offset.y(), area.width, area.height,
                            area.xRadius, area.yRaduis);
        path = path.intersected(window_valid_path);

        if (!path.isEmpty())
            newPathList << path;
    }

    if (!m_blurPathList.isEmpty()) {
        newPathList.reserve(newPathList.size() + m_blurPathList.size());

        foreach (const QPainterPath &path, m_blurPathList) {
            newPathList << (path * device_pixel_ratio).translated(offset).intersected(window_valid_path);
        }
    }

    if (newPathList.isEmpty())
        return true;

    return Utility::blurWindowBackgroundByPaths(top_level_w, newPathList);
}

void DPlatformWindowHelper::updateSizeHints()
{
    const QMargins &content_margins = m_frameWindow->contentMarginsHint();
    const QSize extra_size(content_margins.left() + content_margins.right(),
                           content_margins.top() + content_margins.bottom());

    qt_window_private(m_frameWindow)->minimumSize = m_nativeWindow->window()->minimumSize() + extra_size;
    qt_window_private(m_frameWindow)->maximumSize = m_nativeWindow->window()->maximumSize() + extra_size;
    qt_window_private(m_frameWindow)->baseSize = m_nativeWindow->window()->baseSize() + extra_size;
    qt_window_private(m_frameWindow)->sizeIncrement = m_nativeWindow->window()->sizeIncrement();

    m_frameWindow->handle()->propagateSizeHints();
    updateWindowNormalHints();
}

void DPlatformWindowHelper::updateContentPathForFrameWindow()
{
    if (m_isUserSetClipPath) {
        m_frameWindow->setContentPath(m_clipPath);
    } else {
        m_frameWindow->setContentRoundedRect(m_windowValidGeometry, getWindowRadius());
    }
}

void DPlatformWindowHelper::updateContentWindowGeometry()
{
    const auto windowRatio = m_nativeWindow->window()->devicePixelRatio();
    const auto &contentMargins = m_frameWindow->contentMarginsHint();
    const auto &contentPlatformMargins = contentMargins * windowRatio;
    const QSize &size = m_frameWindow->handle()->geometry().marginsRemoved(contentPlatformMargins).size();

    // update the content window geometry
    setNativeWindowGeometry(QRect(contentPlatformMargins.left(),
                                  contentPlatformMargins.top(),
                                  size.width(), size.height()));
}

#ifdef Q_OS_LINUX
void DPlatformWindowHelper::updateWindowNormalHints()
{
    // update WM_NORMAL_HINTS
    xcb_size_hints_t hints;
    memset(&hints, 0, sizeof(hints));

    xcb_icccm_size_hints_set_resize_inc(&hints, 1, 1);
    xcb_icccm_set_wm_normal_hints(m_nativeWindow->xcb_connection(),
                                  m_nativeWindow->xcb_window(), &hints);

    QSize size_inc = m_frameWindow->sizeIncrement();

    if (size_inc.isEmpty())
        size_inc = QSize(1, 1);

    xcb_get_property_cookie_t cookie = xcb_icccm_get_wm_normal_hints(m_nativeWindow->xcb_connection(), m_frameWindow->winId());

    if (xcb_get_property_reply_t *reply = xcb_get_property_reply(m_nativeWindow->xcb_connection(), cookie, 0)) {
        xcb_icccm_get_wm_size_hints_from_reply(&hints, reply);
        free(reply);

        if (hints.width_inc == 1 && hints.height_inc == 1) {
            return;
        }
    } else {
        return;
    }

    xcb_icccm_size_hints_set_resize_inc(&hints, size_inc.width(), size_inc.height());
    xcb_icccm_set_wm_normal_hints(m_nativeWindow->xcb_connection(),
                                  m_frameWindow->winId(), &hints);
}

void DPlatformWindowHelper::updateWindowShape()
{
    const QPainterPath &real_path = m_clipPath * m_nativeWindow->window()->devicePixelRatio();
    QPainterPathStroker stroker;

    stroker.setJoinStyle(Qt::MiterJoin);
    stroker.setWidth(4 * m_nativeWindow->window()->devicePixelRatio());

    Utility::setShapePath(m_nativeWindow->QNativeWindow::winId(),
                          stroker.createStroke(real_path).united(real_path),
                          m_frameWindow->m_redirectContent || !m_isUserSetClipPath,
                          m_nativeWindow->window()->flags().testFlag(Qt::WindowTransparentForInput));
}
#endif

int DPlatformWindowHelper::getWindowRadius() const
{
//#ifdef Q_OS_LINUX
//    QNativeWindow::NetWmStates states = static_cast<DQNativeWindow*>(m_frameWindow->handle())->netWmStates();

//    if (states.testFlag(QNativeWindow::NetWmStateFullScreen)
//            || states.testFlag(QNativeWindow::NetWmState(QNativeWindow::NetWmStateMaximizedHorz
//                                                         | QNativeWindow::NetWmStateMaximizedVert))) {
//        return 0;
//    }
//#else
    if (m_frameWindow->windowState() == Qt::WindowFullScreen
            || m_frameWindow->windowState() == Qt::WindowMaximized)
        return 0;
//#endif

    return (m_isUserSetWindowRadius || DWMSupport::instance()->hasWindowAlpha()) ? m_windowRadius : 0;
}

int DPlatformWindowHelper::getShadowRadius() const
{
    return DWMSupport::instance()->hasWindowAlpha() ? m_shadowRadius : 0;
}

int DPlatformWindowHelper::getBorderWidth() const
{
    if (m_isUserSetBorderWidth || DWMSupport::instance()->hasWindowAlpha())
        return m_borderWidth;

    return m_frameWindow->canResize() ? 2 : m_borderWidth;
}

static QColor colorBlend(const QColor &color1, const QColor &color2)
{
    QColor c2 = color2.toRgb();

    if (c2.alpha() >= 255)
        return c2;

    QColor c1 = color1.toRgb();
    qreal c1_weight = 1 - c2.alphaF();

    int r = c1_weight * c1.red() + c2.alphaF() * c2.red();
    int g = c1_weight * c1.green() + c2.alphaF() * c2.green();
    int b = c1_weight * c1.blue() + c2.alphaF() * c2.blue();

    return QColor(r, g, b);
}

QColor DPlatformWindowHelper::getBorderColor() const
{
    return DWMSupport::instance()->hasWindowAlpha() ? m_borderColor : colorBlend(QColor("#e0e0e0"), m_borderColor);
}

void DPlatformWindowHelper::updateWindowRadiusFromProperty()
{
    const QVariant &v = m_nativeWindow->window()->property(windowRadius);

    if (!v.isValid()) {
        m_nativeWindow->window()->setProperty(windowRadius, getWindowRadius());

        return;
    }

    bool ok;
    int radius = v.toInt(&ok);

    if (ok && radius != m_windowRadius) {
        m_windowRadius = radius;
        m_isUserSetWindowRadius = true;
        m_isUserSetClipPath = false;

        updateClipPathByWindowRadius(m_nativeWindow->window()->size());
    }
}

void DPlatformWindowHelper::updateBorderWidthFromProperty()
{
    const QVariant &v = m_nativeWindow->window()->property(borderWidth);

    if (!v.isValid()) {
        m_nativeWindow->window()->setProperty(borderWidth, getBorderWidth());

        return;
    }

    bool ok;
    int width = v.toInt(&ok);

    if (ok && width != m_borderWidth) {
        m_borderWidth = width;
        m_isUserSetBorderWidth = true;
        m_frameWindow->setBorderWidth(width);
    }
}

void DPlatformWindowHelper::updateBorderColorFromProperty()
{
    const QVariant &v = m_nativeWindow->window()->property(borderColor);

    if (!v.isValid()) {
        m_nativeWindow->window()->setProperty(borderColor, m_borderColor);

        return;
    }

    const QColor &color = qvariant_cast<QColor>(v);

    if (color.isValid() && m_borderColor != color) {
        m_borderColor = color;
        m_frameWindow->setBorderColor(getBorderColor());
    }
}

void DPlatformWindowHelper::updateShadowRadiusFromProperty()
{
    const QVariant &v = m_nativeWindow->window()->property(shadowRadius);

    if (!v.isValid()) {
        m_nativeWindow->window()->setProperty(shadowRadius, m_shadowRadius);

        return;
    }

    bool ok;
    int radius = qMax(v.toInt(&ok), 0);

    if (ok && radius != m_shadowRadius) {
        m_shadowRadius = radius;

        if (DWMSupport::instance()->hasWindowAlpha())
            m_frameWindow->setShadowRadius(radius);
    }
}

void DPlatformWindowHelper::updateShadowOffsetFromProperty()
{
    const QVariant &v = m_nativeWindow->window()->property(shadowOffset);

    if (!v.isValid()) {
        m_nativeWindow->window()->setProperty(shadowOffset, m_shadowOffset);

        return;
    }

    const QPoint &offset = v.toPoint();

    if (offset != m_shadowOffset) {
        m_shadowOffset = offset;
        m_frameWindow->setShadowOffset(offset);
    }
}

void DPlatformWindowHelper::updateShadowColorFromProperty()
{
    const QVariant &v = m_nativeWindow->window()->property(shadowColor);

    if (!v.isValid()) {
        m_nativeWindow->window()->setProperty(shadowColor, m_shadowColor);

        return;
    }

    const QColor &color = qvariant_cast<QColor>(v);

    if (color.isValid() && m_shadowColor != color) {
        m_shadowColor = color;
        m_frameWindow->setShadowColor(color);
    }
}

void DPlatformWindowHelper::updateEnableSystemResizeFromProperty()
{
    const QVariant &v = m_nativeWindow->window()->property(enableSystemResize);

    if (!v.isValid()) {
        m_nativeWindow->window()->setProperty(enableSystemResize, m_enableSystemResize);

        return;
    }

    if (m_enableSystemResize == v.toBool())
        return;

    m_enableSystemResize = v.toBool();
    m_frameWindow->setEnableSystemResize(m_enableSystemResize);
}

void DPlatformWindowHelper::updateEnableSystemMoveFromProperty()
{
    const QVariant &v = m_nativeWindow->window()->property(enableSystemMove);

    if (!v.isValid()) {
        m_nativeWindow->window()->setProperty(enableSystemMove, m_enableSystemMove);

        return;
    }

    m_enableSystemMove = v.toBool();
    m_frameWindow->setEnableSystemMove(m_enableSystemMove);
}

void DPlatformWindowHelper::updateEnableBlurWindowFromProperty()
{
    const QVariant &v = m_nativeWindow->window()->property(enableBlurWindow);

    if (!v.isValid()) {
        m_nativeWindow->window()->setProperty(enableBlurWindow, m_enableBlurWindow);

        return;
    }

    if (m_enableBlurWindow != v.toBool()) {
        m_enableBlurWindow = v.toBool();

        if (m_enableBlurWindow) {
            QObject::connect(DWMSupport::instance(), &DWMSupport::windowManagerChanged,
                             this, &DPlatformWindowHelper::updateWindowBlurAreasForWM);
        } else {
            QObject::disconnect(DWMSupport::instance(), &DWMSupport::windowManagerChanged,
                                this, &DPlatformWindowHelper::updateWindowBlurAreasForWM);
        }

        updateWindowBlurAreasForWM();
    }
}

void DPlatformWindowHelper::updateWindowBlurAreasFromProperty()
{
    const QVariant &v = m_nativeWindow->window()->property(windowBlurAreas);
    const QVector<quint32> &tmpV = qvariant_cast<QVector<quint32>>(v);
    const QVector<Utility::BlurArea> &a = *(reinterpret_cast<const QVector<Utility::BlurArea>*>(&tmpV));

    if (a.isEmpty() && m_blurAreaList.isEmpty())
        return;

    m_blurAreaList = a;

    updateWindowBlurAreasForWM();
}

void DPlatformWindowHelper::updateWindowBlurPathsFromProperty()
{
    const QVariant &v = m_nativeWindow->window()->property(windowBlurPaths);
    const QList<QPainterPath> paths = qvariant_cast<QList<QPainterPath>>(v);

    if (paths.isEmpty() && m_blurPathList.isEmpty())
        return;

    m_blurPathList = paths;

    updateWindowBlurAreasForWM();
}

void DPlatformWindowHelper::updateAutoInputMaskByClipPathFromProperty()
{
    const QVariant &v = m_nativeWindow->window()->property(autoInputMaskByClipPath);

    if (!v.isValid()) {
        m_nativeWindow->window()->setProperty(autoInputMaskByClipPath, m_autoInputMaskByClipPath);

        return;
    }

    if (m_autoInputMaskByClipPath != v.toBool()) {
        m_autoInputMaskByClipPath = v.toBool();
    }

    m_frameWindow->m_enableAutoInputMaskByContentPath = m_autoInputMaskByClipPath;
}

void DPlatformWindowHelper::setWindowProperty(QWindow *window, const char *name, const QVariant &value)
{
    const QVariant &old_value = window->property(name);

    if (old_value == value)
        return;

    if (value.typeName() == QByteArray("QPainterPath")) {
        const QPainterPath &old_path = qvariant_cast<QPainterPath>(old_value);
        const QPainterPath &new_path = qvariant_cast<QPainterPath>(value);

        if (old_path == new_path) {
            return;
        }
    }

    window->setProperty(name, value);

    if (!mapped.value(window->handle()))
        return;

    QByteArray name_array(name);

    if (!name_array.startsWith("_d_"))
        return;

    // to upper
    name_array[3] = name_array.at(3) & ~0x20;

    const QByteArray slot_name = "update" + name_array.mid(3) + "FromProperty";

    if (!QMetaObject::invokeMethod(mapped.value(window->handle()),
                                   slot_name.constData(),
                                   Qt::DirectConnection)) {
        qWarning() << "Failed to update property:" << slot_name;
    }
}

void DPlatformWindowHelper::onFrameWindowContentMarginsHintChanged(const QMargins &oldMargins)
{
    updateWindowBlurAreasForWM();
    updateSizeHints();

    const QMargins &contentMargins = m_frameWindow->contentMarginsHint();

    m_nativeWindow->window()->setProperty(::frameMargins, QVariant::fromValue(contentMargins));
    m_frameWindow->setGeometry(m_frameWindow->geometry() + contentMargins - oldMargins);

    updateContentWindowGeometry();
}

void DPlatformWindowHelper::onWMHasCompositeChanged()
{
    const QSize &window_size = m_nativeWindow->window()->size();

    updateClipPathByWindowRadius(window_size);

    if (!DXcbWMSupport::instance()->hasWindowAlpha())
        m_frameWindow->disableRepaintShadow();

    m_frameWindow->setShadowRadius(getShadowRadius());
    m_frameWindow->enableRepaintShadow();

//    QPainterPath clip_path = m_clipPath * m_nativeWindow->window()->devicePixelRatio();

//    if (DXcbWMSupport::instance()->hasWindowAlpha()) {
//        QPainterPathStroker stroker;

//        stroker.setJoinStyle(Qt::MiterJoin);
//        stroker.setWidth(1);
//        clip_path = stroker.createStroke(clip_path).united(clip_path);
//    }

    m_frameWindow->updateMask();
    m_frameWindow->setBorderWidth(getBorderWidth());
    m_frameWindow->setBorderColor(getBorderColor());

    if (m_nativeWindow->window()->inherits("QWidgetWindow")) {
        QEvent event(QEvent::UpdateRequest);
        qApp->sendEvent(m_nativeWindow->window(), &event);
    } else {
        QMetaObject::invokeMethod(m_nativeWindow->window(), "update");
    }
}

void DPlatformWindowHelper::onDevicePixelRatioChanged()
{
    updateWindowShape();
    updateFrameMaskFromProperty();

    m_frameWindow->onDevicePixelRatioChanged();
}

void DPlatformWindowHelper::onScreenChanged(QScreen *screen)
{
    QScreen *old_screen = m_nativeWindow->window()->screen();

    if (old_screen != screen) {
        m_nativeWindow->window()->setScreen(screen);
    }

    onDevicePixelRatioChanged();
}

void DPlatformWindowHelper::updateClipPathFromProperty()
{
    const QVariant &v = m_nativeWindow->window()->property(clipPath);
    QPainterPath path;

    path = qvariant_cast<QPainterPath>(v);

    if (!m_isUserSetClipPath && path.isEmpty())
        return;

    m_isUserSetClipPath = !path.isEmpty();

    if (m_isUserSetClipPath)
        setClipPath(path);
    else
        updateClipPathByWindowRadius(m_nativeWindow->window()->size());
}

void DPlatformWindowHelper::updateFrameMaskFromProperty()
{
    const QVariant &v = m_nativeWindow->window()->property(frameMask);

    if (!v.isValid()) {
        return;
    }

    QRegion region = qvariant_cast<QRegion>(v);

    m_frameWindow->setMask(region * m_nativeWindow->window()->devicePixelRatio());
    m_isUserSetFrameMask = !region.isEmpty();
    m_frameWindow->m_enableAutoFrameMask = !m_isUserSetFrameMask;
}

DPP_END_NAMESPACE
