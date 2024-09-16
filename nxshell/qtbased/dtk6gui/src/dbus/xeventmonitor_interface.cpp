// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "xeventmonitor_interface.h"

static void registerAreaListMetaType()
{
    // FIX 取消注册qRegisterMetaType，会影响 dock 使用 RegisterAreas dbus接口
    // 因为 libdframeworkdbus-dev 也会注册 "AreaList" 为 MonitorRect
//    qRegisterMetaType<AreaList>("AreaList");
    qDBusRegisterMetaType<ComDeepinApiXEventMonitorInterface::AreaList>();
}
Q_CONSTRUCTOR_FUNCTION(registerAreaListMetaType);

QDBusArgument &operator<<(QDBusArgument &a, const ComDeepinApiXEventMonitorInterface::AreaList &rects)
{
    // com.deepin.api.XEventMonitor.RegisterAreas
    // (Array of [Struct of (Int32, Int32, Int32, Int32)] areas, Int32 flag)
    // use [x1, y1, x2, y2] instead of [x1, y1, w, h]
    a.beginArray(qRegisterMetaType<QRect>());
    for (const auto &r : rects) {
        a.beginStructure();
        a << r.left() << r.top() << r.right() << r.bottom();
        a.endStructure();
    }
    a.endArray();
    return a;
}

const QDBusArgument &operator>>(const QDBusArgument &a, ComDeepinApiXEventMonitorInterface::AreaList &rects)
{
    a.beginArray();
    rects.clear();
    int x1 = 0 , y1 = 0 , x2 = 0 , y2 = 0 ;
    while (!a.atEnd()) {
        a.beginStructure();
        a >> x1 >> y1 >> x2 >> y2;
        a.endStructure();

        rects.append(QRect(QPoint(x1, y1), QPoint(x2, y2)));
    }
    a.endArray();
    return a;
}

ComDeepinApiXEventMonitorInterface::ComDeepinApiXEventMonitorInterface(const QString &service, const QString &path,
                                                                       const char *interface, QObject *parent,
                                                                       const QDBusConnection &con)
    : QDBusAbstractInterface(service, path, interface, con, parent)
{

}

ComDeepinApiXEventMonitorInterface::~ComDeepinApiXEventMonitorInterface()
{
}

