// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WACOMDEVICELIST_H
#define WACOMDEVICELIST_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtDBus/QtDBus>
#include <QDBusMetaType>

struct WacomDevice {
    QString interface;
    QString deviceType;

    bool operator ==(const WacomDevice& device);
};

typedef QList<WacomDevice> WacomDeviceList;

Q_DECLARE_METATYPE(WacomDevice)
Q_DECLARE_METATYPE(WacomDeviceList)

QDBusArgument &operator<<(QDBusArgument &argument, const WacomDevice &device);
const QDBusArgument &operator>>(const QDBusArgument &argument, WacomDevice &device);

void registerWacomDeviceListMetaType();

#endif // WACOMDEVICELIST_H
