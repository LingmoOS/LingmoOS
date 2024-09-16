// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INPUTDEVICELIST_H
#define INPUTDEVICELIST_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtDBus/QtDBus>
#include <QDBusMetaType>

struct InputDevice {
    QString interface;
    QString deviceType;

    bool operator ==(const InputDevice& device);
};

typedef QList<InputDevice> InputDeviceList;

Q_DECLARE_METATYPE(InputDevice)
Q_DECLARE_METATYPE(InputDeviceList)

QDBusArgument &operator<<(QDBusArgument &argument, const InputDevice &device);
const QDBusArgument &operator>>(const QDBusArgument &argument, InputDevice &device);

void registerInputDeviceListMetaType();

#endif // INPUTDEVICELIST_H
