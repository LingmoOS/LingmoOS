// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TRACKPOINTADAPTOR_H
#define TRACKPOINTADAPTOR_H

#include "dtkdevice_global.h"
#include <QDBusAbstractAdaptor>

DDEVICE_BEGIN_NAMESPACE

class TrackPointAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.daemon.InputDevice.TrackPoint")
    Q_PROPERTY(bool Exist MEMBER m_exist READ Exist NOTIFY ExistChanged)
    Q_PROPERTY(bool LeftHanded MEMBER m_leftHanded READ LeftHanded WRITE SetLeftHanded NOTIFY LeftHandedChanged)
    Q_PROPERTY(bool MiddleButtonEnabled MEMBER m_middleButtonEnabled READ MiddleButtonEnabled WRITE SetMiddleButtonEnabled NOTIFY
                   MiddleButtonEnabledChanged)
    Q_PROPERTY(
        bool WheelEmulation MEMBER m_wheelEmulation READ WheelEmulation WRITE SetWheelEmulation NOTIFY WheelEmulationChanged)
    Q_PROPERTY(bool WheelHorizScroll MEMBER m_wheelHorizScroll READ WheelHorizScroll WRITE SetWheelHorizScroll NOTIFY
                   WheelHorizScrollChanged)
    Q_PROPERTY(double MotionAcceleration MEMBER m_motionAcceleration READ MotionAcceleration WRITE SetMotionAcceleration NOTIFY
                   MotionAccelerationChanged)
    Q_PROPERTY(qint32 MiddleButtonTimeout MEMBER m_middleButtonTimeout READ MiddleButtonTimeout WRITE SetMiddleButtonTimeout
                   NOTIFY MiddleButtonTimeoutChanged)
    Q_PROPERTY(qint32 WheelEmulationButton MEMBER m_wheelEmulationButton READ WheelEmulationButton WRITE SetWheelEmulationButton
                   NOTIFY WheelEmulationButtonChanged)
    Q_PROPERTY(qint32 WheelEmulationTimeout MEMBER m_wheelEmulationTimeout READ WheelEmulationTimeout WRITE
                   SetWheelEmulationTimeout NOTIFY WheelEmulationTimeoutChanged)
    Q_PROPERTY(QString DeviceList MEMBER m_deviceList READ DeviceList NOTIFY DeviceListChanged)
public:
    static inline const bool ExistDefault = true;
    static inline const bool LeftHandedDefault = false;
    static inline const bool MiddleButtonEnabledDefault = true;
    static inline const bool WheelEmulationDefault = true;
    static inline const bool WheelHorizScrollDefault = true;
    static inline const double MotionAccelerationDefault = 1.0;
    static inline const qint32 MiddleButtonTimeoutDefault = 500;
    static inline const qint32 WheelEmulationButtonDefault = 2;
    static inline const qint32 WheelEmulationTimeoutDefault = 500;
    static inline const QString DeviceListDefault = QStringLiteral("[{\"Id\": 10, \"Name\": \"Test TrackPoint\"}]");

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
    void Reset();

public:
    TrackPointAdaptor(QObject *parent = nullptr);
    ~TrackPointAdaptor() override;

private:
    bool m_exist;
    bool m_leftHanded;
    bool m_middleButtonEnabled;
    bool m_wheelEmulation;
    bool m_wheelHorizScroll;
    double m_motionAcceleration;
    qint32 m_middleButtonTimeout;
    qint32 m_wheelEmulationButton;
    qint32 m_wheelEmulationTimeout;
    QString m_deviceList;
};

DDEVICE_END_NAMESPACE
#endif  // TRACKPOINTADAPTOR_H
