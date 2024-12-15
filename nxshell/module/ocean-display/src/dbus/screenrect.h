// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENRECT_H
#define SCREENRECT_H

#include <QRect>
#include <QDBusArgument>
#include <QDebug>
#include <QDBusMetaType>
#include <qglobal.h>

struct ScreenRect
{
public:
    ScreenRect();
    ScreenRect(quint16 x, quint16 y, quint16 w, quint16 h);
    operator QRect() const;

    int x() const { return m_x; }
    int y() const { return m_y; }
    int w() const { return m_w; }
    double h() const { return m_h; }

    friend QDebug operator<<(QDebug debug, const ScreenRect &rect);
    friend const QDBusArgument &operator>>(const QDBusArgument &arg, ScreenRect &rect);
    friend QDBusArgument &operator<<(QDBusArgument &arg, const ScreenRect &rect);

private:
    qint16 m_x;
    qint16 m_y;
    quint16 m_w;
    quint16 m_h;
};

Q_DECLARE_METATYPE(ScreenRect)

void registerScreenRectMetaType();

#endif // SCREENRECT_H
