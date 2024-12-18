// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef INPUTDEVICESINTERFACE_H
#define INPUTDEVICESINTERFACE_H
#include "dtkdevice_global.h"
#include <DDBusInterface>
#include <QDBusConnection>
#include <QObject>
OCEANVICE_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DDBusInterface;
class LIBDTKDEVICESHARED_EXPORT InputDevicesInterface : public QObject
{
    Q_OBJECT
    Q_PROPERTY(quint32 WheelSpeed READ WheelSpeed WRITE SetWheelSpeed NOTIFY WheelSpeedChanged)
public:
    static inline const char *staticInterfaceName() { return "com.lingmo.daemon.InputDevices"; }
    explicit InputDevicesInterface(const QString &service = QStringLiteral("com.lingmo.daemon.InputDevices"),
                                   const QString &path = QStringLiteral("/com/lingmo/daemon/InputDevices"),
                                   QDBusConnection connection = QDBusConnection::sessionBus(),
                                   QObject *parent = nullptr);

    quint32 WheelSpeed() const;
    void SetWheelSpeed(quint32 speed);

Q_SIGNALS:
    void WheelSpeedChanged(quint32 speed);

private:
    DDBusInterface *m_interface;
};
OCEANVICE_END_NAMESPACE

#endif
