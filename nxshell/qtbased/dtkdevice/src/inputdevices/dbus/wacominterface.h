// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WACOMINTERFACE_H
#define WACOMINTERFACE_H
#include "dtkdevice_global.h"
#include <DDBusInterface>
#include <QDBusPendingReply>
#include <QObject>

DDEVICE_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DDBusInterface;
class LIBDTKDEVICESHARED_EXPORT WacomInterface : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool CursorMode READ CursorMode WRITE SetCursorMode NOTIFY CursorModeChanged)
    Q_PROPERTY(bool Exist READ Exist NOTIFY ExistChanged)
    Q_PROPERTY(bool ForceProportions READ ForceProportions WRITE SetForceProportions NOTIFY ForceProportionsChanged)
    Q_PROPERTY(bool LeftHanded READ LeftHanded WRITE SetLeftHanded NOTIFY LeftHandedChanged)
    Q_PROPERTY(bool MouseEnterRemap READ MouseEnterRemap WRITE SetMouseEnterRemap NOTIFY MouseEnterRemapChanged)
    Q_PROPERTY(QString DeviceList READ DeviceList NOTIFY DeviceListChanged)
    Q_PROPERTY(QString KeyDownAction READ KeyDownAction WRITE SetKeyDownAction NOTIFY KeyDownActionChanged)
    Q_PROPERTY(QString KeyUpAction READ KeyUpAction WRITE SetKeyUpAction NOTIFY KeyUpActionChanged)
    Q_PROPERTY(QString MapOutput READ MapOutput NOTIFY MapOutputChanged)
    Q_PROPERTY(quint32 EraserPressureSensitive READ EraserPressureSensitive WRITE SetEraserPressureSensitive NOTIFY
                   EraserPressureSensitiveChanged)
    Q_PROPERTY(quint32 EraserRawSample READ EraserRawSample WRITE SetEraserRawSample NOTIFY EraserRawSampleChanged)
    Q_PROPERTY(quint32 EraserThreshold READ EraserThreshold WRITE SetEraserThreshold NOTIFY EraserThresholdChanged)
    Q_PROPERTY(quint32 StylusPressureSensitive READ StylusPressureSensitive WRITE SetStylusPressureSensitive NOTIFY
                   StylusPressureSensitiveChanged)
    Q_PROPERTY(quint32 StylusRawSample READ StylusRawSample WRITE SetStylusRawSample NOTIFY StylusRawSampleChanged)
    Q_PROPERTY(quint32 StylusThreshold READ StylusThreshold WRITE SetStylusThreshold NOTIFY StylusThresholdChanged)
    Q_PROPERTY(quint32 Suppress READ Suppress WRITE SetSuppress NOTIFY SuppressChanged)

public:
    static inline const char *staticInterfaceName() { return "com.deepin.daemon.InputDevice.Wacom"; }
    WacomInterface(const QString &service = QStringLiteral("com.deepin.daemon.InputDevices"),
                   const QString &path = QStringLiteral("/com/deepin/daemon/InputDevice/Wacom"),
                   QDBusConnection connection = QDBusConnection::sessionBus(),
                   QObject *parent = nullptr);

    bool CursorMode() const;
    bool Exist() const;
    bool ForceProportions() const;
    bool LeftHanded() const;
    bool MouseEnterRemap() const;
    QString DeviceList() const;
    QString KeyDownAction() const;
    QString KeyUpAction() const;
    QString MapOutput() const;
    quint32 EraserPressureSensitive() const;
    quint32 EraserRawSample() const;
    quint32 EraserThreshold() const;
    quint32 StylusPressureSensitive() const;
    quint32 StylusRawSample() const;
    quint32 StylusThreshold() const;
    quint32 Suppress() const;

    void SetCursorMode(bool cursorMode);
    void SetForceProportions(bool forceProportions);
    void SetLeftHanded(bool leftHanded);
    void SetMouseEnterRemap(bool remap);
    void SetKeyDownAction(const QString &action);
    void SetKeyUpAction(const QString &action);
    void SetEraserPressureSensitive(quint32 sensitivity);
    void SetEraserRawSample(quint32 sampleSize);
    void SetEraserThreshold(quint32 threshold);
    void SetStylusPressureSensitive(quint32 sensitivity);
    void SetStylusRawSample(quint32 sampleSize);
    void SetStylusThreshold(quint32 threshold);
    void SetSuppress(quint32 suppress);
Q_SIGNALS:
    void CursorModeChanged(bool cursorMode);
    void ExistChanged(bool exist);
    void ForceProportionsChanged(bool forceProportions);
    void LeftHandedChanged(bool leftHanded);
    void MouseEnterRemapChanged(bool remap);
    void DeviceListChanged(const QString &deviceList);
    void KeyDownActionChanged(const QString &action);
    void KeyUpActionChanged(const QString &action);
    void MapOutputChanged(const QString &mapOutput);
    void EraserPressureSensitiveChanged(quint32 sensitivity);
    void EraserRawSampleChanged(quint32 sampleSize);
    void EraserThresholdChanged(quint32 threshold);
    void StylusPressureSensitiveChanged(quint32 sensitivity);
    void StylusRawSampleChanged(quint32 sampleSize);
    void StylusThresholdChanged(quint32 threshold);
    void SuppressChanged(quint32 suppress);

public Q_SLOTS:
    QDBusPendingReply<> Reset();

private:
    DDBusInterface *m_interface;
};

DDEVICE_END_NAMESPACE
#endif  // WACOMINTERFACE_H
