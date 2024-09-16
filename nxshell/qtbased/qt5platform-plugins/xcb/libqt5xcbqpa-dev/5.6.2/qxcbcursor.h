// Copyright (C) 2016 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QXCBCURSOR_H
#define QXCBCURSOR_H

#include <qpa/qplatformcursor.h>
#include "qxcbscreen.h"

QT_BEGIN_NAMESPACE

#ifndef QT_NO_CURSOR

struct QXcbCursorCacheKey
{
    explicit QXcbCursorCacheKey(const QCursor &c);
    explicit QXcbCursorCacheKey(Qt::CursorShape s) : shape(s), bitmapCacheKey(0), maskCacheKey(0) {}
    QXcbCursorCacheKey() : shape(Qt::CustomCursor), bitmapCacheKey(0), maskCacheKey(0) {}

    Qt::CursorShape shape;
    qint64 bitmapCacheKey;
    qint64 maskCacheKey;
};

inline bool operator==(const QXcbCursorCacheKey &k1, const QXcbCursorCacheKey &k2)
{
    return k1.shape == k2.shape && k1.bitmapCacheKey == k2.bitmapCacheKey && k1.maskCacheKey == k2.maskCacheKey;
}

inline uint qHash(const QXcbCursorCacheKey &k, uint seed) Q_DECL_NOTHROW
{
    return (uint(k.shape) + uint(k.bitmapCacheKey) + uint(k.maskCacheKey)) ^ seed;
}

#endif // !QT_NO_CURSOR

class QXcbCursor : public QXcbObject, public QPlatformCursor
{
public:
    QXcbCursor(QXcbConnection *conn, QXcbScreen *screen);
    ~QXcbCursor();
#ifndef QT_NO_CURSOR
    void changeCursor(QCursor *cursor, QWindow *widget) Q_DECL_OVERRIDE;
#endif
    QPoint pos() const Q_DECL_OVERRIDE;
    void setPos(const QPoint &pos) Q_DECL_OVERRIDE;

    static void queryPointer(QXcbConnection *c, QXcbVirtualDesktop **virtualDesktop, QPoint *pos, int *keybMask = 0);

private:
#ifndef QT_NO_CURSOR
    typedef QHash<QXcbCursorCacheKey, xcb_cursor_t> CursorHash;

    xcb_cursor_t createFontCursor(int cshape);
    xcb_cursor_t createBitmapCursor(QCursor *cursor);
    xcb_cursor_t createNonStandardCursor(int cshape);
#endif

    QXcbScreen *m_screen;
#ifndef QT_NO_CURSOR
    CursorHash m_cursorHash;
#endif
#if defined(XCB_USE_XLIB) && !defined(QT_NO_LIBRARY)
    static void cursorThemePropertyChanged(QXcbVirtualDesktop *screen,
                                           const QByteArray &name,
                                           const QVariant &property,
                                           void *handle);
#endif
    bool m_gtkCursorThemeInitialized;
};

QT_END_NAMESPACE

#endif
