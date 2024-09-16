// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MOUSEINTERFACE_H
#define MOUSEINTERFACE_H
#include "dtkdevice_global.h"
#include <DDBusInterface>
#include <QDBusConnection>
#include <QDBusPendingReply>
#include <QObject>
DDEVICE_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DDBusInterface;
class LIBDTKDEVICESHARED_EXPORT MouseInterface : public QObject
{
    Q_OBJECT
    Q_PROPERTY(
        bool AdaptiveAccelProfile READ AdaptiveAccelProfile WRITE SetAdaptiveAccelProfile NOTIFY AdaptiveAccelProfileChanged)
    Q_PROPERTY(bool DisableTpad READ DisableTpad WRITE SetDisableTpad NOTIFY DisableTpadChanged)
    Q_PROPERTY(bool Exist READ Exist NOTIFY ExistChanged)
    Q_PROPERTY(bool LeftHanded READ LeftHanded WRITE SetLeftHanded NOTIFY LeftHandedChanged)
    Q_PROPERTY(
        bool MiddleButtonEmulation READ MiddleButtonEmulation WRITE SetMiddleButtonEmulation NOTIFY MiddleButtonEmulationChanged)
    Q_PROPERTY(bool NaturalScroll READ NaturalScroll WRITE SetNaturalScroll NOTIFY NaturalScrollChanged)
    Q_PROPERTY(double MotionAcceleration READ MotionAcceleration WRITE SetMotionAcceleration NOTIFY MotionAccelerationChanged)
    Q_PROPERTY(qint32 DoubleClick READ DoubleClick WRITE SetDoubleClick NOTIFY DoubleClickChanged)
    Q_PROPERTY(qint32 DragThreshold READ DragThreshold WRITE SetDragThreshold NOTIFY DragThresholdChanged)
    Q_PROPERTY(QString DeviceList READ DeviceList NOTIFY DeviceListChanged)

public:
    static inline const char *staticInterfaceName() { return "com.deepin.daemon.InputDevice.Mouse"; }
    explicit MouseInterface(const QString &service = QStringLiteral("com.deepin.daemon.InputDevices"),
                            const QString &path = QStringLiteral("/com/deepin/daemon/InputDevice/Mouse"),
                            QDBusConnection connection = QDBusConnection::sessionBus(),
                            QObject *parent = nullptr);

    bool DisableTpad() const;
    bool AdaptiveAccelProfile() const;
    bool Exist() const;
    bool LeftHanded() const;
    bool MiddleButtonEmulation() const;
    bool NaturalScroll() const;
    double MotionAcceleration() const;
    qint32 DoubleClick() const;
    qint32 DragThreshold() const;
    QString DeviceList() const;

    void SetDisableTpad(bool disable);
    void SetAdaptiveAccelProfile(bool adaptive);
    void SetLeftHanded(bool leftHanded);
    void SetMiddleButtonEmulation(bool emulation);
    void SetNaturalScroll(bool naturalScroll);
    void SetMotionAcceleration(double acceleration);
    void SetDoubleClick(qint32 interval);
    void SetDragThreshold(qint32 threshold);

Q_SIGNALS:
    void DisableTpadChanged(bool disable);
    void AdaptiveAccelProfileChanged(bool adaptive);
    void ExistChanged(bool exist);
    void LeftHandedChanged(bool leftHanded);
    void MiddleButtonEmulationChanged(bool emulation);
    void NaturalScrollChanged(bool naturalScroll);
    void MotionAccelerationChanged(double acceleration);
    void DoubleClickChanged(qint32 interval);
    void DragThresholdChanged(qint32 threshold);
    void DeviceListChanged(const QString &deviceList);
public Q_SLOTS:
    QDBusPendingReply<> Reset();

private:
    DDBusInterface *m_interface;
};

DDEVICE_END_NAMESPACE
#endif  // MOUSEINTERFACE_H
