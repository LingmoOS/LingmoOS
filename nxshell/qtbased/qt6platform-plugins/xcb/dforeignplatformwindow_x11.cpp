// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dforeignplatformwindow.h"
#include "dplatformintegration.h"
#include "global.h"
#include "utility.h"
#include "dxcbwmsupport.h"

#include "qxcbconnection.h"
#include "qxcbscreen.h"

#include <QDebug>
#include <QGuiApplication>

#include <private/qwindow_p.h>
#include <private/qguiapplication_p.h>

#include <xcb/xcb_icccm.h>

DPP_BEGIN_NAMESPACE
#define xcbReplyHolder(xcb_reply_type, var) QScopedPointer<xcb_reply_type, QScopedPointerPodDeleter> var

enum {
    baseEventMask
        = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_STRUCTURE_NOTIFY
            | XCB_EVENT_MASK_PROPERTY_CHANGE | XCB_EVENT_MASK_FOCUS_CHANGE,

    defaultEventMask = baseEventMask
            | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE
            | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE
            | XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW
            | XCB_EVENT_MASK_POINTER_MOTION,

    transparentForInputEventMask = baseEventMask
            | XCB_EVENT_MASK_VISIBILITY_CHANGE | XCB_EVENT_MASK_RESIZE_REDIRECT
            | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
            | XCB_EVENT_MASK_COLOR_MAP_CHANGE | XCB_EVENT_MASK_OWNER_GRAB_BUTTON
};

DForeignPlatformWindow::DForeignPlatformWindow(QWindow *window, WId winId)
    : QXcbWindow(window)
{
    QGuiApplicationPrivate::window_list.removeOne(window);
    m_window = winId;

#if QT_VERSION < QT_VERSION_CHECK(5, 9, 0)
    // init window id
    QNativeWindow::create();
#endif

    m_dirtyFrameMargins = true;

    init();
    create();

    // 因为此窗口不包含在 QGuiApplication::allWindows() 中，屏幕对象销毁时不会重置它的屏幕为主屏
    QObject::connect(qApp, &QGuiApplication::screenRemoved, window, [window] (QScreen *screen) {
        if (screen == window->screen()) {
            window->setScreen(qApp->primaryScreen());
        }
    });
}

DForeignPlatformWindow::~DForeignPlatformWindow()
{
    qt_window_private(window())->windowFlags = Qt::ForeignWindow;
    // removeWindowEventListener
    destroy();
    // do not destroy m_window
    m_window = 0;
}

QRect DForeignPlatformWindow::geometry() const
{
    xcb_connection_t *conn = DPlatformIntegration::xcbConnection()->xcb_connection();

    xcbReplyHolder(xcb_get_geometry_reply_t, geomReply)(xcb_get_geometry_reply(conn, xcb_get_geometry(conn, m_window), nullptr));
    if (!geomReply)
        return QRect();

    auto xtc_cookie = xcb_translate_coordinates(conn, m_window, DPlatformIntegration::xcbConnection()->rootWindow(), 0, 0);
    xcbReplyHolder(xcb_translate_coordinates_reply_t, translateReply)(xcb_translate_coordinates_reply(conn, xtc_cookie, nullptr));
    if (!translateReply) {
        return QRect();
    }

    const QRect result(QPoint(translateReply->dst_x, translateReply->dst_y), QSize(geomReply->width, geomReply->height));

    // auto remove _GTK_FRAME_EXTENTS
    xcb_get_property_cookie_t cookie = xcb_get_property(xcb_connection(), false, m_window,
                                                        Utility::internAtom("_GTK_FRAME_EXTENTS"), XCB_ATOM_CARDINAL, 0, 4);
    xcbReplyHolder(xcb_get_property_reply_t, reply)(xcb_get_property_reply(xcb_connection(), cookie, nullptr));
    if (reply && reply->type == XCB_ATOM_CARDINAL && reply->format == 32 && reply->value_len == 4) {
        quint32 *data = (quint32 *)xcb_get_property_value(reply.data());
        // _NET_FRAME_EXTENTS format is left, right, top, bottom
        return result.marginsRemoved(QMargins(data[0], data[2], data[1], data[3]));
    }

    return result;
}

// QXcbWindow::frameMargins会额外根据窗口的geometry来计算frame margins，在这里我们不希望使用这个fallback的逻辑
QMargins DForeignPlatformWindow::frameMargins() const
{
    if (m_dirtyFrameMargins) {
        if (DXcbWMSupport::instance()->isSupportedByWM(atom(QXcbAtom::D_QXCBATOM_WRAPPER(_NET_FRAME_EXTENTS)))) {
            xcb_get_property_cookie_t cookie = xcb_get_property(xcb_connection(), false, m_window, atom(QXcbAtom::D_QXCBATOM_WRAPPER(_NET_FRAME_EXTENTS)), XCB_ATOM_CARDINAL, 0, 4);
            xcb_get_property_reply_t *reply = xcb_get_property_reply(xcb_connection(), cookie, nullptr);

            if (reply) {
                if (reply->type == XCB_ATOM_CARDINAL && reply->format == 32 && reply->value_len == 4) {
                    quint32 *data = (quint32 *)xcb_get_property_value(reply);
                    // _NET_FRAME_EXTENTS format is left, right, top, bottom
                    m_frameMargins = QMargins(data[0], data[2], data[1], data[3]);
                }

                free(reply);
            }
        }

        m_dirtyFrameMargins = false;
    }

    return m_frameMargins;
}

#ifdef Q_OS_LINUX
void DForeignPlatformWindow::handleConfigureNotifyEvent(const xcb_configure_notify_event_t *event)
{
    bool fromSendEvent = (event->response_type & 0x80);
    QPoint pos(event->x, event->y);
    if (!QPlatformWindow::parent() && !fromSendEvent) {
        // Do not trust the position, query it instead.
        xcb_translate_coordinates_cookie_t cookie = xcb_translate_coordinates(xcb_connection(), xcb_window(),
                                                                              xcbScreen()->root(), 0, 0);
        xcb_translate_coordinates_reply_t *reply = xcb_translate_coordinates_reply(xcb_connection(), cookie, NULL);
        if (reply) {
            pos.setX(reply->dst_x);
            pos.setY(reply->dst_y);
            free(reply);
        }
    }

    QRect actualGeometry = QRect(pos, QSize(event->width, event->height));
    QPlatformScreen *newScreen = QPlatformWindow::parent() ? QPlatformWindow::parent()->screen() : screenForGeometry(actualGeometry);
    if (!newScreen)
        return;

    // auto remove _GTK_FRAME_EXTENTS
    xcb_get_property_cookie_t cookie = xcb_get_property(xcb_connection(), false, m_window,
                                                        Utility::internAtom("_GTK_FRAME_EXTENTS"), XCB_ATOM_CARDINAL, 0, 4);
    QScopedPointer<xcb_get_property_reply_t, QScopedPointerPodDeleter> reply(
        xcb_get_property_reply(xcb_connection(), cookie, NULL));
    if (reply && reply->type == XCB_ATOM_CARDINAL && reply->format == 32 && reply->value_len == 4) {
        quint32 *data = (quint32 *)xcb_get_property_value(reply.data());
        // _NET_FRAME_EXTENTS format is left, right, top, bottom
        actualGeometry = actualGeometry.marginsRemoved(QMargins(data[0], data[2], data[1], data[3]));
    }

    // Persist the actual geometry so that QWindow::geometry() can
    // be queried in the resize event.
    QPlatformWindow::setGeometry(actualGeometry);

    // FIXME: In the case of the requestedGeometry not matching the actualGeometry due
    // to e.g. the window manager applying restrictions to the geometry, the application
    // will never see a move/resize event if the actualGeometry is the same as the current
    // geometry, and may think the requested geometry was fulfilled.
    QWindowSystemInterface::handleGeometryChange(window(), actualGeometry);

    // QPlatformScreen::screen() is updated asynchronously, so we can't compare it
    // with the newScreen. Just send the WindowScreenChanged event and QGuiApplication
    // will make the comparison later.
    QWindowSystemInterface::handleWindowScreenChanged(window(), newScreen->screen());

#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
    if (m_usingSyncProtocol && m_syncState == SyncReceived)
#else
    if (connection()->hasXSync() && m_syncState == SyncReceived)
#endif
        m_syncState = SyncAndConfigureReceived;

    m_dirtyFrameMargins = true;
}

void DForeignPlatformWindow::handlePropertyNotifyEvent(const xcb_property_notify_event_t *event)
{
    connection()->setTime(event->time);

    const bool propertyDeleted = event->state == XCB_PROPERTY_DELETE;

    if (event->atom == atom(QXcbAtom::D_QXCBATOM_WRAPPER(_NET_WM_STATE)) || event->atom == atom(QXcbAtom::D_QXCBATOM_WRAPPER(WM_STATE))) {
        if (propertyDeleted)
            return;

        return updateWindowState();
    } else if (event->atom == atom(QXcbAtom::D_QXCBATOM_WRAPPER(_NET_FRAME_EXTENTS))) {
        m_dirtyFrameMargins = true;
    } else if (event->atom == atom(QXcbAtom::D_QXCBATOM_WRAPPER(_NET_WM_WINDOW_TYPE))) {
        return updateWindowTypes();
    } else if (event->atom == Utility::internAtom("_NET_WM_DESKTOP")) {
        return updateWmDesktop();
    } else if (event->atom == QXcbAtom::D_QXCBATOM_WRAPPER(_NET_WM_NAME)) {
        return updateTitle();
    } else if (event->atom == QXcbAtom::D_QXCBATOM_WRAPPER(WM_CLASS)) {
        return updateWmClass();
    }
}

QNativeWindow *DForeignPlatformWindow::toWindow()
{
    // 重写返回空，目的是防止QXcbConnection::platformWindowFromId接口能返回一个正常的QXcbWindow对象
    // 这样会导致QXcbDrag中将本窗口认为是自己窗口的窗口，从而导致drag/drop事件没有通过x11发送给对应窗口
    // 而是直接走了内部事件处理流程
    return nullptr;
}
#endif

void DForeignPlatformWindow::create()
{
    const quint32 mask = XCB_CW_EVENT_MASK;
    const quint32 values[] = {
        // XCB_CW_EVENT_MASK
        baseEventMask
    };

    connection()->addWindowEventListener(m_window, this);
    xcb_change_window_attributes(xcb_connection(), m_window, mask, values);
}

void DForeignPlatformWindow::destroy()
{
    connection()->removeWindowEventListener(m_window);
}

#if QT_VERSION < QT_VERSION_CHECK(5, 9, 0)
QPlatformScreen *DForeignPlatformWindow::screenForGeometry(const QRect &newGeometry) const
{
    QPlatformScreen *currentScreen = screen();
    QPlatformScreen *fallback = currentScreen;
    // QRect::center can return a value outside the rectangle if it's empty.
    // Apply mapToGlobal() in case it is a foreign/embedded window.
    QPoint center = newGeometry.isEmpty() ? newGeometry.topLeft()
                                          : QPoint(qRound((newGeometry.left() + newGeometry.right()) / 2.0),
                                                   qRound((newGeometry.top() + newGeometry.bottom()) / 2.0));

    if (!parent() && currentScreen && !currentScreen->geometry().contains(center)) {
        const auto screens = currentScreen->virtualSiblings();
        for (QPlatformScreen *screen : screens) {
            const QRect screenGeometry = screen->geometry();
            if (screenGeometry.contains(center))
                return screen;
            if (screenGeometry.intersects(newGeometry))
                fallback = screen;
        }
    }
    return fallback;
}
#endif

void DForeignPlatformWindow::updateTitle()
{
    xcb_get_property_reply_t *wm_name =
        xcb_get_property_reply(xcb_connection(),
            xcb_get_property_unchecked(xcb_connection(), false, m_window,
                             atom(QXcbAtom::D_QXCBATOM_WRAPPER(_NET_WM_NAME)),
                             atom(QXcbAtom::D_QXCBATOM_WRAPPER(UTF8_STRING)), 0, 1024), NULL);
    if (wm_name && wm_name->format == 8
            && wm_name->type == atom(QXcbAtom::D_QXCBATOM_WRAPPER(UTF8_STRING))) {
        const QString &title = QString::fromUtf8((const char *)xcb_get_property_value(wm_name), xcb_get_property_value_length(wm_name));

        if (title != qt_window_private(window())->windowTitle) {
            qt_window_private(window())->windowTitle = title;

            emit window()->windowTitleChanged(title);
        }
    }

    free(wm_name);
}

void DForeignPlatformWindow::updateWmClass()
{
    xcb_get_property_reply_t *wm_class =
        xcb_get_property_reply(xcb_connection(),
            xcb_get_property(xcb_connection(), 0, m_window, XCB_ATOM_WM_CLASS, XCB_ATOM_STRING, 0L, 2048L), NULL);
    if (wm_class && wm_class->format == 8
            && wm_class->type == XCB_ATOM_STRING) {
        const QByteArray wm_class_name((const char *)xcb_get_property_value(wm_class), xcb_get_property_value_length(wm_class));
        const QList<QByteArray> wm_class_name_list = wm_class_name.split('\0');

        if (!wm_class_name_list.isEmpty())
            window()->setProperty(WmClass, QString::fromLocal8Bit(wm_class_name_list.first()));
    }

    free(wm_class);
}

void DForeignPlatformWindow::updateWmDesktop()
{
    window()->setProperty(WmNetDesktop, Utility::getWorkspaceForWindow(m_window));
}

void DForeignPlatformWindow::updateWindowState()
{
    Qt::WindowState newState = Qt::WindowNoState;
    const xcb_get_property_cookie_t get_cookie =
    xcb_get_property(xcb_connection(), 0, m_window, atom(QXcbAtom::D_QXCBATOM_WRAPPER(WM_STATE)),
                     XCB_ATOM_ANY, 0, 1024);

    xcb_get_property_reply_t *reply =
        xcb_get_property_reply(xcb_connection(), get_cookie, NULL);

    if (reply && reply->format == 32 && reply->type == atom(QXcbAtom::D_QXCBATOM_WRAPPER(WM_STATE))) {
        const quint32 *data = (const quint32 *)xcb_get_property_value(reply);
        if (reply->length != 0 && XCB_ICCCM_WM_STATE_ICONIC == data[0])
            newState = Qt::WindowMinimized;
    }
    free(reply);

    if (newState != Qt::WindowMinimized) { // Something else changed, get _NET_WM_STATE.
        const NetWmStates states = netWmStates();
        if (states & NetWmStateFullScreen)
            newState = Qt::WindowFullScreen;
        else if ((states & NetWmStateMaximizedHorz) && (states & NetWmStateMaximizedVert))
            newState = Qt::WindowMaximized;
    }

    if (m_windowState == newState)
        return;

    m_windowState = newState;
    qt_window_private(window())->windowState = newState;
    emit window()->windowStateChanged(newState);
    qt_window_private(window())->updateVisibility();
}

void DForeignPlatformWindow::updateWindowTypes()
{
    auto window_types = wmWindowTypes();
    Qt::WindowFlags window_flags = Qt::WindowFlags();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    typedef QXcbWindow QXcbWindowFunctions ;
#endif
    if (window_types & QXcbWindowFunctions::Normal)
        window_flags |= Qt::Window;
    if (window_types & QXcbWindowFunctions::Desktop)
        window_flags |= Qt::Desktop;
    if (window_types & QXcbWindowFunctions::Dialog)
        window_flags |= Qt::Dialog;
    if (window_types & QXcbWindowFunctions::Utility)
        window_flags |= Qt::Tool;
    if (window_types & QXcbWindowFunctions::Tooltip)
        window_flags |= Qt::ToolTip;
    if (window_types & QXcbWindowFunctions::Splash)
        window_flags |= Qt::SplashScreen;
    // default: Qt::Widget, include dock
    if (!window_flags)
        window_flags |= Qt::Widget;

    if (window_types & QXcbWindowFunctions::KdeOverride)
        window_flags |= Qt::FramelessWindowHint;

    qt_window_private(window())->windowFlags = window_flags;
    window()->setProperty(WmWindowTypes, (quint32)window_types);
}

void DForeignPlatformWindow::updateProcessId()
{
    xcb_get_property_cookie_t cookie = xcb_get_property(xcb_connection(), false, m_window,
                                                        atom(QXcbAtom::D_QXCBATOM_WRAPPER(_NET_WM_PID)), XCB_ATOM_CARDINAL, 0, 1);
    QScopedPointer<xcb_get_property_reply_t, QScopedPointerPodDeleter> reply(
        xcb_get_property_reply(xcb_connection(), cookie, NULL));
    if (reply && reply->type == XCB_ATOM_CARDINAL && reply->format == 32 && reply->value_len == 1) {
        window()->setProperty(ProcessId, *(quint32 *)xcb_get_property_value(reply.data()));
    }
}

void DForeignPlatformWindow::init()
{
    updateTitle();
    updateWindowState();
    updateWindowTypes();
    updateWmClass();
    updateWmDesktop();
    updateProcessId();

    if (QPlatformScreen * qplatformScreen = screenForGeometry(geometry())) {
        window()->setScreen(qplatformScreen->screen());
    }

//    m_mapped = Utility::getWindows().contains(m_window);
//    qt_window_private(window())->visible = m_mapped;
}

DPP_END_NAMESPACE
