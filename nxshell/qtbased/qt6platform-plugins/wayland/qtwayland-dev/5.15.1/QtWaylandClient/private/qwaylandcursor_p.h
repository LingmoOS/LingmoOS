// Copyright (C) 2019 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDCURSOR_H
#define QWAYLANDCURSOR_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qpa/qplatformcursor.h>
#include <QtCore/QMap>
#include <QtWaylandClient/qtwaylandclientglobal.h>

#if QT_CONFIG(cursor)

struct wl_cursor;
struct wl_cursor_image;
struct wl_cursor_theme;

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandBuffer;
class QWaylandDisplay;
class QWaylandScreen;
class QWaylandShm;

class Q_WAYLAND_CLIENT_EXPORT QWaylandCursorTheme
{
public:
    static QWaylandCursorTheme *create(QWaylandShm *shm, int size);
    static QWaylandCursorTheme *create(QWaylandShm *shm, int size, const QString &themeName);
    ~QWaylandCursorTheme();
    struct wl_cursor_image *cursorImage(Qt::CursorShape shape);

private:
    enum WaylandCursor {
        ArrowCursor = Qt::ArrowCursor,
        UpArrowCursor,
        CrossCursor,
        WaitCursor,
        IBeamCursor,
        SizeVerCursor,
        SizeHorCursor,
        SizeBDiagCursor,
        SizeFDiagCursor,
        SizeAllCursor,
        BlankCursor,
        SplitVCursor,
        SplitHCursor,
        PointingHandCursor,
        ForbiddenCursor,
        WhatsThisCursor,
        BusyCursor,
        OpenHandCursor,
        ClosedHandCursor,
        DragCopyCursor,
        DragMoveCursor,
        DragLinkCursor,
        // The following are used for cursors that don't have equivalents in Qt
        ResizeNorthCursor = Qt::CustomCursor + 1,
        ResizeSouthCursor,
        ResizeEastCursor,
        ResizeWestCursor,
        ResizeNorthWestCursor,
        ResizeSouthEastCursor,
        ResizeNorthEastCursor,
        ResizeSouthWestCursor
    };

    explicit QWaylandCursorTheme(struct ::wl_cursor_theme *theme) : m_theme(theme) {}
    struct ::wl_cursor *requestCursor(WaylandCursor shape);
    struct ::wl_cursor_theme *m_theme = nullptr;
    QMap<WaylandCursor, wl_cursor *> m_cursors;
};

class Q_WAYLAND_CLIENT_EXPORT QWaylandCursor : public QPlatformCursor
{
public:
    QWaylandCursor(QWaylandScreen *screen);

    void changeCursor(QCursor *cursor, QWindow *window) override;
    void pointerEvent(const QMouseEvent &event) override;
    QPoint pos() const override;
    void setPos(const QPoint &pos) override;

    QSharedPointer<QWaylandBuffer> cursorBitmapImage(const QCursor *cursor);
    struct wl_cursor_image *cursorImage(Qt::CursorShape shape);

private:
    QWaylandDisplay *mDisplay = nullptr;
    QWaylandCursorTheme *mCursorTheme = nullptr;
    QPoint mLastPos;
};

}

QT_END_NAMESPACE

#endif // cursor
#endif // QWAYLANDCURSOR_H
