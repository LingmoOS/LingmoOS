// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screenrect.h"

ScreenRect::ScreenRect()
    : m_x(0),
      m_y(0),
      m_w(0),
      m_h(0)
{

}

ScreenRect::ScreenRect(quint16 x, quint16 y, quint16 w, quint16 h)
    : m_x(x),
      m_y(y),
      m_w(w),
      m_h(h)
{

}

QDebug operator<<(QDebug debug, const ScreenRect &rect)
{
    debug << QString("ScreenRect(%1, %2, %3, %4)").arg(rect.x())
                                                    .arg(rect.y())
                                                    .arg(rect.w())
                                                    .arg(rect.h());

    return debug;
}

ScreenRect::operator QRect() const
{
    return QRect(x(), y(), w(), h());
}

QDBusArgument &operator<<(QDBusArgument &arg, const ScreenRect &rect)
{
    arg.beginStructure();
    arg << rect.x() << rect.y() << rect.w() << rect.h();
    arg.endStructure();

    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, ScreenRect &rect)
{
    arg.beginStructure();
    arg >> rect.m_x >> rect.m_y >> rect.m_w >> rect.m_h;
    arg.endStructure();

    return arg;
}

void registerScreenRectMetaType()
{
    qRegisterMetaType<ScreenRect>("ScreenRect");
    qDBusRegisterMetaType<ScreenRect>();
}
