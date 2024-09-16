// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WACOMSERVICE_H
#define WACOMSERVICE_H
#include "dtkdevice_global.h"
#include <QObject>

DDEVICE_BEGIN_NAMESPACE
class WacomService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.daemon.InputDevice.Wacom")
    Q_PROPERTY(bool CursorMode MEMBER m_cursorMode READ CursorMode WRITE SetCursorMode NOTIFY CursorModeChanged)
    Q_PROPERTY(bool Exist MEMBER m_exist READ Exist NOTIFY ExistChanged)
    Q_PROPERTY(bool ForceProportions MEMBER m_forceProportions READ ForceProportions WRITE SetForceProportions NOTIFY
                   ForceProportionsChanged)
    Q_PROPERTY(bool LeftHanded MEMBER m_leftHanded READ LeftHanded WRITE SetLeftHanded NOTIFY LeftHandedChanged)
    Q_PROPERTY(
        bool MouseEnterRemap MEMBER m_mouseEnterRemap READ MouseEnterRemap WRITE SetMouseEnterRemap NOTIFY MouseEnterRemapChanged)
    Q_PROPERTY(QString DeviceList MEMBER m_deviceList READ DeviceList NOTIFY DeviceListChanged)
    Q_PROPERTY(QString KeyDownAction MEMBER m_keyDownAction READ KeyDownAction WRITE SetKeyDownAction NOTIFY KeyDownActionChanged)
    Q_PROPERTY(QString KeyUpAction MEMBER m_keyUpAction READ KeyUpAction WRITE SetKeyUpAction NOTIFY KeyUpActionChanged)
    Q_PROPERTY(QString MapOutput MEMBER m_mapOutput READ MapOutput NOTIFY MapOutputChanged)
    Q_PROPERTY(quint32 EraserPressureSensitive MEMBER m_eraserPressureSensitive READ EraserPressureSensitive WRITE
                   SetEraserPressureSensitive NOTIFY EraserPressureSensitiveChanged)
    Q_PROPERTY(quint32 EraserRawSample MEMBER m_eraserRawSample READ EraserRawSample WRITE SetEraserRawSample NOTIFY
                   EraserRawSampleChanged)
    Q_PROPERTY(quint32 EraserThreshold MEMBER m_eraserThreshold READ EraserThreshold WRITE SetEraserThreshold NOTIFY
                   EraserThresholdChanged)
    Q_PROPERTY(quint32 StylusPressureSensitive MEMBER m_stylusPressureSensitive READ StylusPressureSensitive WRITE
                   SetStylusPressureSensitive NOTIFY StylusPressureSensitiveChanged)
    Q_PROPERTY(quint32 StylusRawSample MEMBER m_stylusRawSample READ StylusRawSample WRITE SetStylusRawSample NOTIFY
                   StylusRawSampleChanged)
    Q_PROPERTY(quint32 StylusThreshold MEMBER m_stylusThreshold READ StylusThreshold WRITE SetStylusThreshold NOTIFY
                   StylusThresholdChanged)
    Q_PROPERTY(quint32 Suppress MEMBER m_suppress READ Suppress WRITE SetSuppress NOTIFY SuppressChanged)

public:
    static inline const bool CursorModeDefault = false;
    static inline const bool ExistDefault = true;
    static inline const bool ForceProportionsDefault = false;
    static inline const bool LeftHandedDefault = false;
    static inline const bool MouseEnterRemapDefault = false;
    static inline const QString DeviceListDefault = QStringLiteral("[{\"Id\": 10, \"Name\": \"Test Wacom\"}]");
    static inline const QString KeyDownActionDefault = QStringLiteral("LeftClick");
    static inline const QString KeyUpActionDefault = QStringLiteral("RightClick");
    static inline const QString MapOutputDefault = QStringLiteral("DisplayPort-0");
    static inline const quint32 EraserPressureSensitiveDefault = 10;
    static inline const quint32 EraserRawSampleDefault = 3;
    static inline const quint32 EraserThresholdDefault = 10;
    static inline const quint32 StylusPressureSensitiveDefault = 10;
    static inline const quint32 StylusRawSampleDefault = 3;
    static inline const quint32 StylusThresholdDefault = 10;
    static inline const quint32 SuppressDefault = 20;

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
    void Reset();

public:
    WacomService(const QString &service = QStringLiteral("org.deepin.dtk.InputDevices"),
                 const QString &path = QStringLiteral("/com/deepin/daemon/InputDevice/Wacom"),
                 QObject *parent = nullptr);
    ~WacomService() override;

private:
    bool registerService(const QString &service, const QString &path);
    void unregisterService();

private:
    bool m_cursorMode;
    bool m_exist;
    bool m_forceProportions;
    bool m_leftHanded;
    bool m_mouseEnterRemap;
    QString m_deviceList;
    QString m_keyDownAction;
    QString m_keyUpAction;
    QString m_mapOutput;
    quint32 m_eraserPressureSensitive;
    quint32 m_eraserRawSample;
    quint32 m_eraserThreshold;
    quint32 m_stylusPressureSensitive;
    quint32 m_stylusRawSample;
    quint32 m_stylusThreshold;
    quint32 m_suppress;
    QString m_service;
    QString m_path;
};

DDEVICE_END_NAMESPACE
#endif  // WACOMSERVICE_H
