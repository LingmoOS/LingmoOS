// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef INPUTDEVICESSERVICE_H
#define INPUTDEVICESSERVICE_H
#include "dtkdevice_global.h"
#include <QObject>
DDEVICE_BEGIN_NAMESPACE
class InputDevicesService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.daemon.InputDevices")
    Q_PROPERTY(quint32 WheelSpeed MEMBER m_wheelSpeed READ WheelSpeed WRITE SetWheelSpeed NOTIFY WheelSpeedChanged)

public:
    quint32 WheelSpeed() const;
    void SetWheelSpeed(quint32 speed);

Q_SIGNALS:
    void WheelSpeedChanged(quint32 speed);

public:
    InputDevicesService(const QString &service = QStringLiteral("org.deepin.dtk.InputDevices"),
                        const QString &path = QStringLiteral("/com/deepin/daemon/InputDevices"),
                        QObject *parent = nullptr);
    ~InputDevicesService() override;

private:
    bool registerService(const QString &service, const QString &path);
    void unregisterService();

private:
    quint32 m_wheelSpeed;
    QString m_service;
    QString m_path;
};
DDEVICE_END_NAMESPACE

#endif
