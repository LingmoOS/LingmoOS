// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef _DMR_UTILITY_H
#define _DMR_UTILITY_H

#include <QImage>
#include <QList>

#include <xcb/xproto.h>

QT_BEGIN_NAMESPACE
class QXcbWindow;
QT_END_NAMESPACE

typedef uint32_t xcb_atom_t;

class Utility
{
public:
    enum CornerEdge {
        TopLeftCorner = 0,
        TopEdge = 1,
        TopRightCorner = 2,
        RightEdge = 3,
        BottomRightCorner = 4,
        BottomEdge = 5,
        BottomLeftCorner = 6,
        LeftEdge = 7,
        NoneEdge = -1
    };

    static xcb_atom_t internAtom(const char *name);
    static void startWindowSystemMove(quint32 WId);
    static void cancelWindowMoveResize(quint32 WId);

    // 在触摸屏下移动窗口时，调用 startWindowSystemMove后，窗管无法grab触摸屏的touch update事件
    // 导致窗口无法移动。此处跟deepin-wm配合，使用其它方式通知窗管鼠标位置更新了
    static void updateMousePointForWindowMove(quint32 WId, const QPoint &globalPos);

    static void setFrameExtents(quint32 WId, const QMargins &margins);
    static void setRectangles(quint32 WId, const QRegion &region, bool onlyInput = true);
    static void setRectangles(quint32 WId, const QVector<xcb_rectangle_t> &rectangles, bool onlyInput = true);
    static void setShapePath(quint32 WId, const QPainterPath &path, bool onlyInput = true);
    static void startWindowSystemResize(quint32 WId, CornerEdge cornerEdge, const QPoint &globalPos = QPoint());
    static bool setWindowCursor(quint32 WId, CornerEdge ce);

    static QRegion regionAddMargins(const QRegion &region, const QMargins &margins, const QPoint &offset = QPoint(0, 0));

    static QByteArray windowProperty(quint32 WId, xcb_atom_t propAtom, xcb_atom_t typeAtom, quint32 len);
    static QList<xcb_atom_t> windowNetWMState(quint32 WId);
    static void setWindowProperty(quint32 WId, xcb_atom_t propAtom, xcb_atom_t typeAtom, const void *data, quint32 len, uint8_t format = 8);
//    static void setStayOnTop(const QWidget *widget, bool on);

private:
    static void sendMoveResizeMessage(quint32 WId, uint32_t action, QPoint globalPos = QPoint(), Qt::MouseButton qbutton = Qt::LeftButton);
    static QVector<xcb_rectangle_t> qregion2XcbRectangles(const QRegion &region);
};

#endif // _DMR_UTILITY_H
