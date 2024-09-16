// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MOUSEADAPTOR_H
#define MOUSEADAPTOR_H
#include "dtkdevice_global.h"
#include <QDBusAbstractAdaptor>
DDEVICE_BEGIN_NAMESPACE
class MouseAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.daemon.InputDevice.Mouse")
    Q_PROPERTY(bool AdaptiveAccelProfile MEMBER m_adaptiveAccelProfile READ AdaptiveAccelProfile WRITE SetAdaptiveAccelProfile
                   NOTIFY AdaptiveAccelProfileChanged)
    Q_PROPERTY(bool DisableTpad MEMBER m_disableTpad READ DisableTpad WRITE SetDisableTpad NOTIFY DisableTpadChanged)
    Q_PROPERTY(bool Exist MEMBER m_exist READ Exist NOTIFY ExistChanged)
    Q_PROPERTY(bool LeftHanded MEMBER m_leftHanded READ LeftHanded WRITE SetLeftHanded NOTIFY LeftHandedChanged)
    Q_PROPERTY(bool MiddleButtonEmulation MEMBER m_middleButtonEmulation READ MiddleButtonEmulation WRITE SetMiddleButtonEmulation
                   NOTIFY MiddleButtonEmulationChanged)
    Q_PROPERTY(bool NaturalScroll MEMBER m_naturalScroll READ NaturalScroll WRITE SetNaturalScroll NOTIFY NaturalScrollChanged)
    Q_PROPERTY(double MotionAcceleration MEMBER m_motionAcceleration READ MotionAcceleration WRITE SetMotionAcceleration NOTIFY
                   MotionAccelerationChanged)
    Q_PROPERTY(qint32 DoubleClick MEMBER m_doubleClick READ DoubleClick WRITE SetDoubleClick NOTIFY DoubleClickChanged)
    Q_PROPERTY(qint32 DragThreshold MEMBER m_dragThreshold READ DragThreshold WRITE SetDragThreshold NOTIFY DragThresholdChanged)
    Q_PROPERTY(QString DeviceList MEMBER m_deviceList READ DeviceList NOTIFY DeviceListChanged)

public:
    static inline const bool AdaptiveAccelProfileDefault = false;
    static inline const bool DisableTpadDefault = false;
    static inline const bool ExistDefault = true;
    static inline const bool LeftHandedDefault = false;
    static inline const bool MiddleButtonEmulationDefault = true;
    static inline const bool NaturalScrollDefault = false;
    static inline const double MotionAccelerationDefault = 1.0;
    static inline const qint32 DoubleClickDefault = 500;
    static inline const qint32 DragThresholdDefault = 20;
    static inline const QString DeviceListDefault = QStringLiteral("[{\"Id\": 10, \"Name\": \"Test Mouse\"}]");

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
    void Reset();

public:
    MouseAdaptor(QObject *parent = nullptr);
    ~MouseAdaptor() override;

private:
    bool m_adaptiveAccelProfile;
    bool m_disableTpad;
    bool m_exist;
    bool m_leftHanded;
    bool m_middleButtonEmulation;
    bool m_naturalScroll;
    double m_motionAcceleration;
    qint32 m_doubleClick;
    qint32 m_dragThreshold;
    QString m_deviceList;
};
DDEVICE_END_NAMESPACE
#endif  // MOUSEADAPTOR_H
