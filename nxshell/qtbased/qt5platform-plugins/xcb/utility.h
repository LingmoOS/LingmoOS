// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef UTILITY_H
#define UTILITY_H

#include <QImage>
#include <QPainterPath>

#include "global.h"

QT_BEGIN_NAMESPACE
class QXcbWindow;
QT_END_NAMESPACE

typedef uint32_t xcb_atom_t;
typedef struct xcb_connection_t xcb_connection_t;

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#define D_QXCBATOM_WRAPPER(AtomEnum) Atom##AtomEnum
#else
#define D_QXCBATOM_WRAPPER(AtomEnum) AtomEnum
#endif

DPP_BEGIN_NAMESPACE

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
        LeftEdge = 7
    };

    static QImage dropShadow(const QPixmap &px, qreal radius, const QColor &color);
    static QImage borderImage(const QPixmap &px, const QMargins &borders, const QSize &size,
                              QImage::Format format = QImage::Format_ARGB32_Premultiplied);

    static QList<QRect> sudokuByRect(const QRect &rect, QMargins borders);

    static xcb_atom_t internAtom(const char *name, bool only_if_exists = true);
    static xcb_atom_t internAtom(xcb_connection_t *connection, const char *name, bool only_if_exists = true);
    static void startWindowSystemMove(quint32 WId);
    static void cancelWindowMoveResize(quint32 WId);

    // 在触摸屏下移动窗口时，调用 startWindowSystemMove后，窗管无法grab触摸屏的touch update事件
    // 导致窗口无法移动。此处跟deepin-wm配合，使用其它方式通知窗管鼠标位置更新了
    // TODO: kwin适配udpate之后没有结束move状态，新增一个finished参数，当传入true时通知kwin结束
    static void updateMousePointForWindowMove(quint32 WId, bool finished = false);

    static void showWindowSystemMenu(quint32 WId, QPoint globalPos = QPoint());
    static void setFrameExtents(WId wid, const QMargins &margins);
    static void setShapeRectangles(quint32 WId, const QRegion &region, bool onlyInput = true, bool transparentInput = false);
    static void setShapePath(quint32 WId, const QPainterPath &path, bool onlyInput = true, bool transparentInput = false);
    static void startWindowSystemResize(quint32 WId, CornerEdge cornerEdge, const QPoint &globalPos = QPoint());
    static bool setWindowCursor(quint32 WId, CornerEdge ce);

    static QRegion regionAddMargins(const QRegion &region, const QMargins &margins, const QPoint &offset = QPoint(0, 0));

    static QByteArray windowProperty(quint32 WId, xcb_atom_t propAtom, xcb_atom_t typeAtom, quint32 len);
    static void setWindowProperty(quint32 WId, xcb_atom_t propAtom, xcb_atom_t typeAtom, const void *data, quint32 len, uint8_t format = 8);
    static void clearWindowProperty(quint32 WId, xcb_atom_t propAtom);

    static void setNoTitlebar(quint32 WId, bool on);
    static void splitWindowOnScreen(quint32 WId, quint32 type);
    static void splitWindowOnScreenByType(quint32 WId, quint32 position, quint32 type);
    static bool supportForSplittingWindow(quint32 WId);
    static bool supportForSplittingWindowByType(quint32 WId, quint32 screenSplittingType);

    struct BlurArea {
        qint32 x;
        qint32 y;
        qint32 width;
        qint32 height;
        qint32 xRadius;
        qint32 yRaduis;

        inline BlurArea operator *(qreal scale)
        {
            if (qFuzzyCompare(1.0, scale))
                return *this;

            BlurArea new_area;

            new_area.x = qRound64(x * scale);
            new_area.y = qRound64(y * scale);
            new_area.width = qRound64(width * scale);
            new_area.height = qRound64(height * scale);
            new_area.xRadius = qRound64(xRadius * scale);
            new_area.yRaduis = qRound64(yRaduis * scale);

            return new_area;
        }

        inline BlurArea &operator *=(qreal scale)
        {
            return *this = *this * scale;
        }
    };

    // by kwin
    static bool setEnableBlurWindow(const quint32 WId, bool enable);
    // by Deepin Window Manager
    static bool blurWindowBackground(const quint32 WId, const QVector<BlurArea> &areas);
    static bool blurWindowBackgroundByPaths(const quint32 WId, const QList<QPainterPath> &paths);
    static bool blurWindowBackgroundByImage(const quint32 WId, const QRect &blurRect, const QImage &maskImage);
    static void clearWindowBlur(const quint32 WId);
    static bool updateBackgroundWallpaper(const quint32 WId, const QRect &area, const quint32 bMode);
    static void clearWindowBackground(const quint32 WId);

    static qint32 getWorkspaceForWindow(quint32 WId);
    static QVector<quint32> getWindows();
    static quint32 windowFromPoint(const QPoint &p);
    static QVector<quint32> getCurrentWorkspaceWindows();

    struct QtMotifWmHints {
        quint32 flags, functions, decorations;
        qint32 input_mode;
        quint32 status;
    };

    static QtMotifWmHints getMotifWmHints(quint32 WId);
    static void setMotifWmHints(quint32 WId, QtMotifWmHints hints);
    static quint32 getNativeTopLevelWindow(quint32 WId);

    static QPoint translateCoordinates(const QPoint &pos, quint32 src, quint32 dst);
    static QRect windowGeometry(quint32 WId);

    static quint32 clientLeader();
    static quint32 createGroupWindow();
    static void destoryGroupWindow(quint32 groupLeader);
    static void setWindowGroup(quint32 window, quint32 groupLeader);

#ifdef Q_OS_LINUX
    static int XIconifyWindow(void *display, quint32 w, int screen_number);
#endif

private:
    static void sendMoveResizeMessage(quint32 WId, uint32_t action, QPoint globalPos = QPoint(), Qt::MouseButton qbutton = Qt::LeftButton);
    static QWindow *getWindowById(quint32 WId);
    static qreal getWindowDevicePixelRatio(quint32 WId);
};

DPP_END_NAMESPACE

QT_BEGIN_NAMESPACE
DPP_USE_NAMESPACE
QDebug operator<<(QDebug deg, const Utility::BlurArea &area);
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
inline QPainterPath &operator *=(QPainterPath &path, qreal scale)
{
    if (qFuzzyCompare(1.0, scale))
        return path;

    for (int i = 0; i < path.elementCount(); ++i) {
        const QPainterPath::Element &e = path.elementAt(i);

        path.setElementPositionAt(i, qRound(e.x * scale), qRound(e.y * scale));
    }

    return path;
}
inline QRect operator *(const QRect &rect, qreal scale)
{
    if (qFuzzyCompare(1.0, scale))
        return rect;

    return QRect(qRound(rect.left() * scale), qRound(rect.top() * scale),
                 qRound(rect.width() * scale), qRound(rect.height() * scale));
}
inline QMargins operator -(const QRect &r1, const QRect &r2)
{
    return QMargins(r2.left() - r1.left(), r2.top() - r1.top(),
                    r1.right() - r2.right(), r1.bottom() - r2.bottom());
}
inline QRegion operator *(const QRegion &pointRegion, qreal scale)
{
    if (qFuzzyCompare(1.0, scale))
        return pointRegion;

    QRegion pixelRegon;
    for (auto region = pointRegion.begin(); region != pointRegion.end();
         ++region) {
        pixelRegon += (*region) * scale;
    }
    return pixelRegon;
}
QT_END_NAMESPACE

#endif // UTILITY_H
