/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     lingmo <lingmo@lingmo.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "windowhelper.h"

#include <cstring>
#include <QApplication>
#include <QGuiApplication>
#include <QCursor>

#include <xcb/xcb.h>

static uint qtEdgesToXcbMoveResizeDirection(Qt::Edges edges)
{
    if (edges == (Qt::TopEdge | Qt::LeftEdge))
        return 0;
    if (edges == Qt::TopEdge)
        return 1;
    if (edges == (Qt::TopEdge | Qt::RightEdge))
        return 2;
    if (edges == Qt::RightEdge)
        return 3;
    if (edges == (Qt::RightEdge | Qt::BottomEdge))
        return 4;
    if (edges == Qt::BottomEdge)
        return 5;
    if (edges == (Qt::BottomEdge | Qt::LeftEdge))
        return 6;
    if (edges == Qt::LeftEdge)
        return 7;

    return 0;
}

static bool isCompositingActive(xcb_connection_t *connection)
{
    // Check _NET_WM_CM_Sn atom on root window to detect compositing manager
    const xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
    xcb_window_t rootWindow = screen->root;

    // Build atom name: _NET_WM_CM_Sn where n is the screen number
    QByteArray atomName = QByteArray("_NET_WM_CM_Sn") + QByteArray::number(0);
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, false, atomName.size(), atomName.constData());
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, cookie, nullptr);
    if (!reply)
        return false;

    xcb_atom_t atom = reply->atom;
    free(reply);

    // Check if the compositing manager owns this atom
    xcb_get_selection_owner_cookie_t selCookie = xcb_get_selection_owner(connection, atom);
    xcb_get_selection_owner_reply_t *selReply = xcb_get_selection_owner_reply(connection, selCookie, nullptr);
    if (!selReply) {
        return false;
    }

    bool active = selReply->owner != XCB_NONE;
    free(selReply);
    return active;
}

WindowHelper::WindowHelper(QObject *parent)
    : QObject(parent)
    , m_moveResizeAtom(0)
    , m_compositing(false)
{
    // create move-resize atom
    // ref: https://github.com/qt/qtbase/blob/9db7cc79a26ced4997277b5c206ca15949133240/src/plugins/platforms/xcb/qxcbwindow.cpp
    auto *x11App = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    xcb_connection_t* connection(x11App->connection());
    const QString atomName(QStringLiteral("_NET_WM_MOVERESIZE"));
    xcb_intern_atom_cookie_t cookie(xcb_intern_atom(connection, false, atomName.size(), qPrintable(atomName)));
    QScopedPointer<xcb_intern_atom_reply_t> reply(xcb_intern_atom_reply(connection, cookie, nullptr));
    m_moveResizeAtom = reply ? reply->atom : 0;

    onCompositingChanged(isCompositingActive(connection));
}

bool WindowHelper::compositing() const
{
    return m_compositing;
}

void WindowHelper::startSystemMove(QWindow *w)
{
    doStartSystemMoveResize(w, 16);
}

void WindowHelper::startSystemResize(QWindow *w, Qt::Edges edges)
{
    doStartSystemMoveResize(w, edges);
}

void WindowHelper::minimizeWindow(QWindow *w)
{
    auto *x11App = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    xcb_connection_t *connection = x11App->connection();
    xcb_window_t winId = w->winId();

    // Use _NET_WM_STATE_HIDDEN to minimize via EWMH
    xcb_client_message_event_t xev;
    memset(&xev, 0, sizeof(xev));
    xev.response_type = XCB_CLIENT_MESSAGE;
    xev.format = 32;
    xev.window = winId;

    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, false, sizeof("_NET_WM_STATE_HIDDEN") - 1, "_NET_WM_STATE_HIDDEN");
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, cookie, nullptr);
    if (reply) {
        xcb_intern_atom_cookie_t stateCookie = xcb_intern_atom(connection, false, sizeof("_NET_WM_STATE") - 1, "_NET_WM_STATE");
        xcb_intern_atom_reply_t *stateReply = xcb_intern_atom_reply(connection, stateCookie, nullptr);
        if (stateReply) {
            xev.type = stateReply->atom;
            xev.data.data32[0] = 1; // _NET_WM_STATE_ADD
            xev.data.data32[1] = reply->atom;
            xev.data.data32[2] = 0;
            xev.data.data32[3] = 1;

            const xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
            xcb_send_event(connection, false, screen->root,
                           XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY,
                           (const char *)&xev);
            free(stateReply);
        }
        free(reply);
    }
}

void WindowHelper::doStartSystemMoveResize(QWindow *w, int edges)
{
    const qreal dpiRatio = qApp->devicePixelRatio();

    auto *x11App = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    xcb_connection_t *connection(x11App->connection());
    xcb_client_message_event_t xev;
    xev.response_type = XCB_CLIENT_MESSAGE;
    xev.type = m_moveResizeAtom;
    xev.sequence = 0;
    xev.window = w->winId();
    xev.format = 32;
    xev.data.data32[0] = QCursor::pos().x() * dpiRatio;
    xev.data.data32[1] = QCursor::pos().y() * dpiRatio;

    if (edges == 16)
        xev.data.data32[2] = 8; // move
    else
        xev.data.data32[2] = qtEdgesToXcbMoveResizeDirection(Qt::Edges(edges));

    xev.data.data32[3] = XCB_BUTTON_INDEX_1;
    xev.data.data32[4] = 0;

    // Get root window from xcb
    const xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
    xcb_window_t rootWindow = screen->root;

    xcb_ungrab_pointer(connection, XCB_CURRENT_TIME);
    xcb_send_event(connection, false, rootWindow,
                   XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY,
                   (const char *)&xev);
}

void WindowHelper::onCompositingChanged(bool enabled)
{
    if (enabled != m_compositing) {
        m_compositing = enabled;
        emit compositingChanged();
    }
}
