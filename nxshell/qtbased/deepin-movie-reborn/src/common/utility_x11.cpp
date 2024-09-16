// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utility.h"

#include <QtWidgets>
#include <QtX11Extras/QX11Info>

#include <xcb/shape.h>
#include <xcb/xproto.h>

#include <X11/cursorfont.h>
#include <X11/Xlib.h>

#define _NET_WM_MOVERESIZE_MOVE              8   /* movement only */
#define _NET_WM_MOVERESIZE_CANCEL           11   /* cancel operation */

#define XATOM_MOVE_RESIZE "_NET_WM_MOVERESIZE"
#define XDEEPIN_BLUR_REGION "_NET_WM_DEEPIN_BLUR_REGION"
#define XDEEPIN_BLUR_REGION_ROUNDED "_NET_WM_DEEPIN_BLUR_REGION_ROUNDED"

#define _NET_WM_STATE_REMOVE        0    /* remove/unset property */
#define _NET_WM_STATE_ADD           1    /* add/set property */
#define _NET_WM_STATE_TOGGLE        2    /* toggle property  */

const char kAtomNameHidden[] = "_NET_WM_STATE_HIDDEN";
const char kAtomNameFullscreen[] = "_NET_WM_STATE_FULLSCREEN";
const char kAtomNameMaximizedHorz[] = "_NET_WM_STATE_MAXIMIZED_HORZ";
const char kAtomNameMaximizedVert[] = "_NET_WM_STATE_MAXIMIZED_VERT";
const char kAtomNameMoveResize[] = "_NET_WM_MOVERESIZE";
const char kAtomNameWmState[] = "_NET_WM_STATE";
const char kAtomNameWmStateAbove[] = "_NET_WM_STATE_ABOVE";
const char kAtomNameWmStateStaysOnTop[] = "_NET_WM_STATE_STAYS_ON_TOP";
const char kAtomNameWmSkipTaskbar[] = "_NET_WM_STATE_SKIP_TASKBAR";
const char kAtomNameWmSkipPager[] = "_NET_WM_STATE_SKIP_PAGER";

xcb_atom_t Utility::internAtom(const char *name)
{
    if (!name || *name == 0)
        return XCB_NONE;

    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(QX11Info::connection(), true, strlen(name), name);
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(QX11Info::connection(), cookie, 0);

    if (!reply)
        return XCB_NONE;

    xcb_atom_t atom = reply->atom;
    free(reply);

    return atom;
}

void Utility::startWindowSystemMove(quint32 WId)
{
    sendMoveResizeMessage(WId, _NET_WM_MOVERESIZE_MOVE);
}

void Utility::cancelWindowMoveResize(quint32 WId)
{
    sendMoveResizeMessage(WId, _NET_WM_MOVERESIZE_CANCEL);
}

void Utility::updateMousePointForWindowMove(quint32 WId, const QPoint &globalPos)
{
    xcb_client_message_event_t xev;

    xev.response_type = XCB_CLIENT_MESSAGE;
    xev.type = internAtom("_DEEPIN_MOVE_UPDATE");
    xev.window = WId;
    xev.format = 32;
    xev.data.data32[0] = globalPos.x();
    xev.data.data32[1] = globalPos.y();
    xev.data.data32[2] = 0;
    xev.data.data32[3] = 0;
    xev.data.data32[4] = 0;

    xcb_send_event(QX11Info::connection(), false, QX11Info::appRootWindow(),
                   XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY,
                   (const char *)&xev);

    xcb_flush(QX11Info::connection());
}

/*void Utility::setFrameExtents(quint32 WId, const QMargins &margins)
{
    xcb_atom_t frameExtents = internAtom("_GTK_FRAME_EXTENTS");

    if (frameExtents == XCB_NONE) {
        qWarning() << "Failed to create atom with name _GTK_FRAME_EXTENTS";
        return;
    }

    uint32_t value[4] = {
        (uint32_t)margins.left(),
        (uint32_t)margins.right(),
        (uint32_t)margins.top(),
        (uint32_t)margins.bottom()
    };

    xcb_change_property(QX11Info::connection(), XCB_PROP_MODE_REPLACE, WId, frameExtents, XCB_ATOM_CARDINAL, 32, 4, value);
}*/

void Utility::setRectangles(quint32 WId, const QRegion &region, bool onlyInput)
{
    setRectangles(WId, qregion2XcbRectangles(region), onlyInput);
}

void Utility::setRectangles(quint32 WId, const QVector<xcb_rectangle_t> &rectangles, bool onlyInput)
{
    if (rectangles.isEmpty()) {
        xcb_shape_mask(QX11Info::connection(), XCB_SHAPE_SO_SET,
                       onlyInput ? XCB_SHAPE_SK_INPUT : XCB_SHAPE_SK_BOUNDING, WId, 0, 0, XCB_NONE);

        return;
    }

    xcb_shape_rectangles(QX11Info::connection(), XCB_SHAPE_SO_SET, onlyInput ? XCB_SHAPE_SK_INPUT : XCB_SHAPE_SK_BOUNDING,
                         XCB_CLIP_ORDERING_YX_BANDED, WId, 0, 0, rectangles.size(), rectangles.constData());
}

/*void Utility::setShapePath(quint32 WId, const QPainterPath &path, bool onlyInput)
{
    if (path.isEmpty()) {
        return setRectangles(WId, QVector<xcb_rectangle_t>(), onlyInput);
    }

    QVector<xcb_rectangle_t> rectangles;

    foreach(const QPolygonF &polygon, path.toFillPolygons()) {
        foreach(const QRect &area, QRegion(polygon.toPolygon()).rects()) {
            xcb_rectangle_t rectangle;

            rectangle.x = area.x();
            rectangle.y = area.y();
            rectangle.width = area.width();
            rectangle.height = area.height();

            rectangles.append(std::move(rectangle));
        }
    }

    setRectangles(WId, rectangles, onlyInput);
}*/

void Utility::sendMoveResizeMessage(quint32 WId, uint32_t action, QPoint globalPos, Qt::MouseButton qbutton)
{
    int xbtn = qbutton == Qt::LeftButton ? XCB_BUTTON_INDEX_1 :
               qbutton == Qt::RightButton ? XCB_BUTTON_INDEX_3 :
               XCB_BUTTON_INDEX_ANY;

    if (globalPos.isNull()) {
        //QTBUG-76114
        //globalPos = QCursor::pos();
        xcb_generic_error_t** err = nullptr;
        xcb_query_pointer_reply_t* p = xcb_query_pointer_reply(QX11Info::connection(),
                                                              xcb_query_pointer(QX11Info::connection(),
                                                                                QX11Info::appRootWindow(QX11Info::appScreen())),
                                                              err);
        if (p && err == nullptr) {
            globalPos = QPoint(p->root_x, p->root_y);
        }

        if (p) {
            free(p);
        }
    }

    xcb_client_message_event_t xev;

    xev.response_type = XCB_CLIENT_MESSAGE;
    xev.type = internAtom(XATOM_MOVE_RESIZE);
    xev.window = WId;
    xev.format = 32;
    xev.data.data32[0] = globalPos.x();
    xev.data.data32[1] = globalPos.y();
    xev.data.data32[2] = action;
    xev.data.data32[3] = xbtn;
    xev.data.data32[4] = 0;

    xcb_ungrab_pointer(QX11Info::connection(), QX11Info::appTime());
    xcb_send_event(QX11Info::connection(), false, QX11Info::appRootWindow(QX11Info::appScreen()),
                   XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY,
                   (const char *)&xev);

    xcb_flush(QX11Info::connection());
}

QVector<xcb_rectangle_t> Utility::qregion2XcbRectangles(const QRegion &region)
{
    QVector<xcb_rectangle_t> rectangles;

    rectangles.reserve(region.rectCount());

    for (const QRect &rect : region.rects()) {
        xcb_rectangle_t r;

        r.x = rect.x();
        r.y = rect.y();
        r.width = rect.width();
        r.height = rect.height();

        rectangles << r;
    }

    return rectangles;
}

/*void Utility::startWindowSystemResize(quint32 WId, CornerEdge cornerEdge, const QPoint &globalPos)
{
    sendMoveResizeMessage(WId, cornerEdge, globalPos);
}*/

static xcb_cursor_t CornerEdge2Xcb_cursor_t(Utility::CornerEdge ce)
{
    switch (ce) {
    case Utility::TopEdge:
        return XC_top_side;
    case Utility::TopRightCorner:
        return XC_top_right_corner;
    case Utility::RightEdge:
        return XC_right_side;
    case Utility::BottomRightCorner:
        return XC_bottom_right_corner;
    case Utility::BottomEdge:
        return XC_bottom_side;
    case Utility::BottomLeftCorner:
        return XC_bottom_left_corner;
    case Utility::LeftEdge:
        return XC_left_side;
    case Utility::TopLeftCorner:
        return XC_top_left_corner;
    default:
        return XCB_CURSOR_NONE;
    }
}

bool Utility::setWindowCursor(quint32 WId, Utility::CornerEdge ce)
{
    const auto display = QX11Info::display();

    Cursor cursor = XCreateFontCursor(display, CornerEdge2Xcb_cursor_t(ce));

    if (!cursor) {
        qWarning() << "[ui]::setWindowCursor() call XCreateFontCursor() failed";
        return false;
    }

    const int result = XDefineCursor(display, WId, cursor);

    XFlush(display);

    return result == Success;
}

/*QRegion Utility::regionAddMargins(const QRegion &region, const QMargins &margins, const QPoint &offset)
{
    QRegion tmp;

    for (const QRect &rect : region.rects()) {
        tmp += rect.translated(offset) + margins;
    }

    return tmp;
}*/

/*QByteArray Utility::windowProperty(quint32 WId, xcb_atom_t propAtom, xcb_atom_t typeAtom, quint32 len)
{
    QByteArray data;
    xcb_connection_t* conn = QX11Info::connection();
    xcb_get_property_cookie_t cookie = xcb_get_property(conn, false, WId, propAtom, typeAtom, 0, len);
    xcb_generic_error_t* err = nullptr;
    xcb_get_property_reply_t* reply = xcb_get_property_reply(conn, cookie, &err);

    if (reply != nullptr) {
        len = xcb_get_property_value_length(reply);
        const char* buf = static_cast<const char*>(xcb_get_property_value(reply));
        data.append(buf, len);
        free(reply);
    }

    if (err != nullptr) {
        qInfo() << "get property error";
        free(err);
    }

    return data;
}*/

/*QList<xcb_atom_t> Utility::windowNetWMState(quint32 WId)
{
    QList<xcb_atom_t> res;

    const auto wmStateAtom = XInternAtom(QX11Info::display(), kAtomNameWmState, false);
    xcb_connection_t* conn = QX11Info::connection();
    xcb_get_property_cookie_t cookie = xcb_get_property(conn, false, WId,
            wmStateAtom, XCB_ATOM_ATOM, 0, 1);
    xcb_generic_error_t* err = nullptr;
    xcb_get_property_reply_t* reply = xcb_get_property_reply(conn, cookie, &err);

    if (reply != nullptr) {
        auto len = xcb_get_property_value_length(reply);
        uint32_t *data = static_cast<uint32_t*>(xcb_get_property_value(reply));
        for (int i = 0; i < len; i++) {
            res.append(data[i]);
        }
        free(reply);
    }

    if (err != nullptr) {
        qInfo() << "get property error";
        free(err);
    }

    return res;
}*/

/*void Utility::setWindowProperty(quint32 WId, xcb_atom_t propAtom, xcb_atom_t typeAtom, const void *data, quint32 len, uint8_t format)
{
    xcb_connection_t* conn = QX11Info::connection();
    xcb_change_property(conn, XCB_PROP_MODE_REPLACE, WId, propAtom, typeAtom, format, len, data);
    xcb_flush(conn);
}*/

/*void Utility::setStayOnTop(const QWidget *widget, bool on)
{
    Q_ASSERT(widget);

    const auto display = QX11Info::display();
    const auto screen = QX11Info::appScreen();

    const auto wmStateAtom = XInternAtom(display, kAtomNameWmState, false);
    const auto stateAboveAtom = XInternAtom(display, kAtomNameWmStateAbove, false);
    const auto stateStaysOnTopAtom = XInternAtom(display,
                                     kAtomNameWmStateStaysOnTop,
                                     false);

    XEvent xev;
    memset(&xev, 0, sizeof(xev));

    xev.xclient.type = ClientMessage;
    xev.xclient.message_type = wmStateAtom;
    xev.xclient.display = display;
    xev.xclient.window = widget->winId();
    xev.xclient.format = 32;

    xev.xclient.data.l[0] = on ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
    xev.xclient.data.l[1] = stateAboveAtom;
    xev.xclient.data.l[2] = stateStaysOnTopAtom;
    xev.xclient.data.l[3] = 1;

    XSendEvent(display,
               QX11Info::appRootWindow(screen),
               false,
               SubstructureRedirectMask | SubstructureNotifyMask,
               &xev);
    XFlush(display);
}*/

