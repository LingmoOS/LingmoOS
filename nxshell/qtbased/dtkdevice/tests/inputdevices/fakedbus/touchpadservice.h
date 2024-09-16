// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TOUCHPADSERVICE_H
#define TOUCHPADSERVICE_H
#include "dtkdevice_global.h"
#include <QObject>
DDEVICE_BEGIN_NAMESPACE

class TouchPadService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.daemon.InputDevice.TouchPad")
    Q_PROPERTY(
        bool DisableIfTyping MEMBER m_disableIfTyping READ DisableIfTyping WRITE SetDisableIfTyping NOTIFY DisableIfTypingChanged)
    Q_PROPERTY(bool EdgeScroll MEMBER m_edgeScroll READ EdgeScroll WRITE SetEdgeScroll NOTIFY EdgeScrollChanged)
    Q_PROPERTY(bool Exist MEMBER m_exist READ Exist NOTIFY ExistChanged)
    Q_PROPERTY(bool LeftHanded MEMBER m_leftHanded READ LeftHanded WRITE SetLeftHanded NOTIFY LeftHandedChanged)
    Q_PROPERTY(bool NaturalScroll MEMBER m_naturalScroll READ NaturalScroll WRITE SetNaturalScroll NOTIFY NaturalScrollChanged)
    Q_PROPERTY(bool TPadEnable MEMBER m_tPadEnable READ TPadEnable WRITE SetTPadEnable NOTIFY TPadEnableChanged)
    Q_PROPERTY(bool TapClick MEMBER m_tapClick READ TapClick WRITE SetTapClick NOTIFY TapClickChanged)
    Q_PROPERTY(bool VertScroll MEMBER m_vertScroll READ VertScroll WRITE SetVertScroll NOTIFY VertScrollChanged)
    Q_PROPERTY(double MotionAcceleration MEMBER m_motionAcceleration READ MotionAcceleration WRITE SetMotionAcceleration NOTIFY
                   MotionAccelerationChanged)
    Q_PROPERTY(qint32 DeltaScroll MEMBER m_deltaScroll READ DeltaScroll WRITE SetDeltaScroll NOTIFY DeltaScrollChanged)
    Q_PROPERTY(qint32 DoubleClick MEMBER m_doubleClick READ DoubleClick WRITE SetDoubleClick NOTIFY DoubleClickChanged)
    Q_PROPERTY(qint32 DragThreshold MEMBER m_dragThreshold READ DragThreshold WRITE SetDragThreshold NOTIFY DragThresholdChanged)
    Q_PROPERTY(QString DeviceList MEMBER m_deviceList READ DeviceList NOTIFY DeviceListChanged)

public:
    static inline const bool DisableIfTypingDefault = false;
    static inline const bool EdgeScrollDefault = false;
    static inline const bool ExistDefault = true;
    static inline const bool LeftHandedDefault = false;
    static inline const bool NaturalScrollDefault = false;
    static inline const bool TPadEnableDefault = true;
    static inline const bool TapClickDefault = true;
    static inline const bool VertScrollDefault = true;
    static inline const double MotionAccelerationDefault = 1.0;
    static inline const qint32 DeltaScrollDefault = 20;
    static inline const qint32 DoubleClickDefault = 500;
    static inline const qint32 DragThresholdDefault = 30;
    static inline const QString DeviceListDefault = QStringLiteral("[{\"Id\": 10, \"Name\": \"Test TouchPad\"}]");

    bool DisableIfTyping() const;
    bool EdgeScroll() const;
    bool Exist() const;
    bool LeftHanded() const;
    bool NaturalScroll() const;
    bool TPadEnable() const;
    bool TapClick() const;
    bool VertScroll() const;
    double MotionAcceleration() const;
    qint32 DeltaScroll() const;
    qint32 DoubleClick() const;
    qint32 DragThreshold() const;
    QString DeviceList() const;

    void SetDisableIfTyping(bool disable);
    void SetEdgeScroll(bool edgeScroll);
    void SetLeftHanded(bool leftHanded);
    void SetNaturalScroll(bool naturalScroll);
    void SetTPadEnable(bool enable);
    void SetTapClick(bool tapClick);
    void SetVertScroll(bool vertScroll);
    void SetMotionAcceleration(double acceleration);
    void SetDeltaScroll(qint32 deltaScroll);
    void SetDoubleClick(qint32 interval);
    void SetDragThreshold(qint32 threshold);

Q_SIGNALS:
    void DisableIfTypingChanged(bool disable);
    void EdgeScrollChanged(bool edgeScroll);
    void ExistChanged(bool exist);
    void LeftHandedChanged(bool leftHanded);
    void NaturalScrollChanged(bool naturalScroll);
    void TPadEnableChanged(bool enable);
    void TapClickChanged(bool tapClick);
    void VertScrollChanged(bool vertScroll);
    void MotionAccelerationChanged(double acceleration);
    void DeltaScrollChanged(qint32 deltaScroll);
    void DoubleClickChanged(qint32 interval);
    void DragThresholdChanged(qint32 threshold);
    void DeviceListChanged(const QString &deviceList);

public Q_SLOTS:
    void Reset();

public:
    TouchPadService(const QString &service = QStringLiteral("org.deepin.dtk.InputDevices"),
                    const QString &path = QStringLiteral("/com/deepin/daemon/InputDevice/TouchPad"),
                    QObject *parent = nullptr);
    ~TouchPadService() override;

private:
    bool registerService(const QString &service, const QString &path);
    void unregisterService();

private:
    bool m_disableIfTyping;
    bool m_edgeScroll;
    bool m_exist;
    bool m_leftHanded;
    bool m_naturalScroll;
    bool m_tPadEnable;
    bool m_tapClick;
    bool m_vertScroll;
    double m_motionAcceleration;
    qint32 m_deltaScroll;
    qint32 m_doubleClick;
    qint32 m_dragThreshold;
    QString m_deviceList;
    QString m_service;
    QString m_path;
};

DDEVICE_END_NAMESPACE
#endif  // TOUCHPADSERVICE_H
