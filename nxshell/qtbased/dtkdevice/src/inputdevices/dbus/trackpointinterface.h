// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TRACKPOINTINTERFACE_H
#define TRACKPOINTINTERFACE_H
#include "dtkdevice_global.h"
#include <DDBusInterface>
#include <QDBusConnection>
#include <QDBusPendingReply>
#include <QObject>

DDEVICE_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DDBusInterface;

class LIBDTKDEVICESHARED_EXPORT TrackPointInterface : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool Exist READ Exist NOTIFY ExistChanged)
    Q_PROPERTY(bool LeftHanded READ LeftHanded WRITE SetLeftHanded NOTIFY LeftHandedChanged)
    Q_PROPERTY(bool MiddleButtonEnabled READ MiddleButtonEnabled WRITE SetMiddleButtonEnabled NOTIFY MiddleButtonEnabledChanged)
    Q_PROPERTY(bool WheelEmulation READ WheelEmulation WRITE SetWheelEmulation NOTIFY WheelEmulationChanged)
    Q_PROPERTY(bool WheelHorizScroll READ WheelHorizScroll WRITE SetWheelHorizScroll NOTIFY WheelHorizScrollChanged)
    Q_PROPERTY(double MotionAcceleration READ MotionAcceleration WRITE SetMotionAcceleration NOTIFY MotionAccelerationChanged)
    Q_PROPERTY(qint32 MiddleButtonTimeout READ MiddleButtonTimeout WRITE SetMiddleButtonTimeout NOTIFY MiddleButtonTimeoutChanged)
    Q_PROPERTY(
        qint32 WheelEmulationButton READ WheelEmulationButton WRITE SetWheelEmulationButton NOTIFY WheelEmulationButtonChanged)
    Q_PROPERTY(qint32 WheelEmulationTimeout READ WheelEmulationTimeout WRITE SetWheelEmulationTimeout NOTIFY
                   WheelEmulationTimeoutChanged)
    Q_PROPERTY(QString DeviceList READ DeviceList NOTIFY DeviceListChanged)
public:
    static inline const char *staticInterfaceName() { return "com.deepin.daemon.InputDevice.TrackPoint"; }
    TrackPointInterface(const QString &service = QStringLiteral("com.deepin.daemon.InputDevices"),
                        const QString &path = QStringLiteral("/com/deepin/daemon/InputDevice/Mouse"),
                        QDBusConnection connection = QDBusConnection::sessionBus(),
                        QObject *parent = nullptr);

    bool Exist() const;
    bool LeftHanded() const;
    bool MiddleButtonEnabled() const;
    bool WheelEmulation() const;
    bool WheelHorizScroll() const;
    double MotionAcceleration() const;
    qint32 MiddleButtonTimeout() const;
    qint32 WheelEmulationButton() const;
    qint32 WheelEmulationTimeout() const;
    QString DeviceList() const;

    void SetLeftHanded(bool leftHanded);
    void SetMiddleButtonEnabled(bool enabled);
    void SetWheelEmulation(bool emulation);
    void SetWheelHorizScroll(bool horizScroll);
    void SetMotionAcceleration(double acceleration);
    void SetMiddleButtonTimeout(qint32 timeout);
    void SetWheelEmulationButton(qint32 button);
    void SetWheelEmulationTimeout(qint32 timeout);

Q_SIGNALS:
    void ExistChanged(bool exist);
    void LeftHandedChanged(bool leftHanded);
    void MiddleButtonEnabledChanged(bool enabled);
    void WheelEmulationChanged(bool emulation);
    void WheelHorizScrollChanged(bool horizScroll);
    void MotionAccelerationChanged(double acceleration);
    void MiddleButtonTimeoutChanged(qint32 timeout);
    void WheelEmulationButtonChanged(qint32 button);
    void WheelEmulationTimeoutChanged(qint32 timeout);
    void DeviceListChanged(const QString &deviceList);
public Q_SLOTS:
    QDBusPendingReply<> Reset();

private:
    DDBusInterface *m_interface;
};

DDEVICE_END_NAMESPACE
#endif  // TRACKPOINTINTERFACE_H
