// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TOUCHPADINTERFACE_H
#define TOUCHPADINTERFACE_H
#include "dtkdevice_global.h"
#include <DDBusInterface>
#include <QDBusConnection>
#include <QDBusPendingReply>
#include <QObject>
DDEVICE_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DDBusInterface;
class LIBDTKDEVICESHARED_EXPORT TouchPadInterface : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool DisableIfTyping READ DisableIfTyping WRITE SetDisableIfTyping NOTIFY DisableIfTypingChanged)
    Q_PROPERTY(bool EdgeScroll READ EdgeScroll WRITE SetEdgeScroll NOTIFY EdgeScrollChanged)
    Q_PROPERTY(bool Exist READ Exist NOTIFY ExistChanged)
    Q_PROPERTY(bool LeftHanded READ LeftHanded WRITE SetLeftHanded NOTIFY LeftHandedChanged)
    Q_PROPERTY(bool NaturalScroll READ NaturalScroll WRITE SetNaturalScroll NOTIFY NaturalScrollChanged)
    Q_PROPERTY(bool PalmDetect READ PalmDetect WRITE SetPalmDetect NOTIFY PalmDetectChanged)
    Q_PROPERTY(bool TPadEnable READ TPadEnable WRITE SetTPadEnable NOTIFY TPadEnableChanged)
    Q_PROPERTY(bool TapClick READ TapClick WRITE SetTapClick NOTIFY TapClickChanged)
    Q_PROPERTY(bool VertScroll READ VertScroll WRITE SetVertScroll NOTIFY VertScrollChanged)
    Q_PROPERTY(double MotionAcceleration READ MotionAcceleration WRITE SetMotionAcceleration NOTIFY MotionAccelerationChanged)
    Q_PROPERTY(qint32 DeltaScroll READ DeltaScroll WRITE SetDeltaScroll NOTIFY DeltaScrollChanged)
    Q_PROPERTY(qint32 DoubleClick READ DoubleClick WRITE SetDoubleClick NOTIFY DoubleClickChanged)
    Q_PROPERTY(qint32 DragThreshold READ DragThreshold WRITE SetDragThreshold NOTIFY DragThresholdChanged)
    Q_PROPERTY(qint32 PalmMinWidth READ PalmMinWidth WRITE SetPalmMinWidth NOTIFY PalmMinWidthChanged)
    Q_PROPERTY(qint32 PalmMinZ READ PalmMinZ WRITE SetPalmMinZ NOTIFY PalmMinZChanged)
    Q_PROPERTY(QString DeviceList READ DeviceList NOTIFY DeviceListChanged)

public:
    static inline const char *staticInterfaceName() { return "com.deepin.daemon.InputDevice.TouchPad"; }
    explicit TouchPadInterface(const QString &service = QStringLiteral("com.deepin.daemon.InputDevices"),
                               const QString &path = QStringLiteral("/com/deepin/daemon/InputDevice/TouchPad"),
                               QDBusConnection connection = QDBusConnection::sessionBus(),
                               QObject *parent = nullptr);
    bool DisableIfTyping() const;
    bool EdgeScroll() const;
    bool Exist() const;
    bool LeftHanded() const;
    bool NaturalScroll() const;
    bool PalmDetect() const;
    bool TPadEnable() const;
    bool TapClick() const;
    bool VertScroll() const;
    double MotionAcceleration() const;
    qint32 DeltaScroll() const;
    qint32 DoubleClick() const;
    qint32 DragThreshold() const;
    qint32 PalmMinWidth() const;
    qint32 PalmMinZ() const;
    QString DeviceList() const;

    void SetDisableIfTyping(bool disable);
    void SetEdgeScroll(bool edgeScroll);
    void SetLeftHanded(bool leftHanded);
    void SetNaturalScroll(bool naturalScroll);
    void SetPalmDetect(bool palmDetect);
    void SetTPadEnable(bool enable);
    void SetTapClick(bool tapClick);
    void SetVertScroll(bool vertScroll);
    void SetMotionAcceleration(double acceleration);
    void SetDeltaScroll(qint32 deltaScroll);
    void SetDoubleClick(qint32 interval);
    void SetDragThreshold(qint32 threshold);
    void SetPalmMinWidth(qint32 minWidth);
    void SetPalmMinZ(qint32 minZ);

Q_SIGNALS:
    void DisableIfTypingChanged(bool disable);
    void EdgeScrollChanged(bool edgeScroll);
    void ExistChanged(bool exist);
    void LeftHandedChanged(bool leftHanded);
    void NaturalScrollChanged(bool naturalScroll);
    void PalmDetectChanged(bool palmDetect);
    void TPadEnableChanged(bool enable);
    void TapClickChanged(bool tapClick);
    void VertScrollChanged(bool vertScroll);
    void MotionAccelerationChanged(double acceleration);
    void DeltaScrollChanged(qint32 deltaScroll);
    void DoubleClickChanged(qint32 interval);
    void DragThresholdChanged(qint32 threshold);
    void PalmMinWidthChanged(qint32 minWidth);
    void PalmMinZChanged(qint32 minZ);
    void DeviceListChanged(const QString &deviceList);
public Q_SLOTS:
    QDBusPendingReply<> Reset();

private:
    DDBusInterface *m_interface;
};

DDEVICE_END_NAMESPACE
#endif  // TOUCHPADINTERFACE_H
