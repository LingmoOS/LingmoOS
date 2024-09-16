// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "wacomservice.h"
#include <QtDBus>

DDEVICE_BEGIN_NAMESPACE
WacomService::WacomService(const QString &service, const QString &path, QObject *parent)
    : QObject(parent)
    , m_cursorMode(CursorModeDefault)
    , m_exist(ExistDefault)
    , m_forceProportions(ForceProportionsDefault)
    , m_leftHanded(LeftHandedDefault)
    , m_mouseEnterRemap(MouseEnterRemapDefault)
    , m_deviceList(DeviceListDefault)
    , m_keyDownAction(KeyDownActionDefault)
    , m_keyUpAction(KeyUpActionDefault)
    , m_mapOutput(MapOutputDefault)
    , m_eraserPressureSensitive(EraserPressureSensitiveDefault)
    , m_eraserRawSample(EraserRawSampleDefault)
    , m_eraserThreshold(EraserThresholdDefault)
    , m_stylusPressureSensitive(StylusPressureSensitiveDefault)
    , m_stylusRawSample(StylusRawSampleDefault)
    , m_stylusThreshold(StylusThresholdDefault)
    , m_suppress(SuppressDefault)
    , m_service(service)
    , m_path(path)
{
    registerService(m_service, m_path);
}

WacomService::~WacomService()
{
    unregisterService();
}

bool WacomService::registerService(const QString &service, const QString &path)
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService(service)) {
        QString errorMsg = connection.lastError().message();
        if (errorMsg.isEmpty())
            errorMsg = "maybe it's running";

        qWarning() << QString("Can't register the %1 service, %2.").arg(service).arg(errorMsg);
        return false;
    }
    if (!connection.registerObject(path, this, QDBusConnection::ExportAllContents)) {
        qWarning() << QString("Can't register %1 the D-Bus object.").arg(path);
        return false;
    }
    return true;
}

void WacomService::unregisterService()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.unregisterObject(m_path);
    connection.unregisterService(m_service);
}

bool WacomService::CursorMode() const
{
    return m_cursorMode;
}

bool WacomService::Exist() const
{
    return m_exist;
}

bool WacomService::ForceProportions() const
{
    return m_forceProportions;
}

bool WacomService::LeftHanded() const
{
    return m_leftHanded;
}

bool WacomService::MouseEnterRemap() const
{
    return m_mouseEnterRemap;
}

QString WacomService::DeviceList() const
{
    return m_deviceList;
}

QString WacomService::KeyDownAction() const
{
    return m_keyDownAction;
}

QString WacomService::KeyUpAction() const
{
    return m_keyUpAction;
}

QString WacomService::MapOutput() const
{
    return m_mapOutput;
}

quint32 WacomService::EraserPressureSensitive() const
{
    return m_eraserPressureSensitive;
}

quint32 WacomService::EraserRawSample() const
{
    return m_eraserRawSample;
}

quint32 WacomService::EraserThreshold() const
{
    return m_eraserThreshold;
}

quint32 WacomService::StylusPressureSensitive() const
{
    return m_stylusPressureSensitive;
}

quint32 WacomService::StylusRawSample() const
{
    return m_stylusRawSample;
}

quint32 WacomService::StylusThreshold() const
{
    return m_stylusThreshold;
}

quint32 WacomService::Suppress() const
{
    return m_suppress;
}

void WacomService::SetCursorMode(bool cursorMode)
{
    m_cursorMode = cursorMode;
    Q_EMIT this->CursorModeChanged(cursorMode);
}

void WacomService::SetForceProportions(bool forceProportions)
{
    m_forceProportions = forceProportions;
    Q_EMIT this->ForceProportionsChanged(forceProportions);
}

void WacomService::SetLeftHanded(bool leftHanded)
{
    m_leftHanded = leftHanded;
    Q_EMIT this->LeftHandedChanged(leftHanded);
}

void WacomService::SetMouseEnterRemap(bool remap)
{
    m_mouseEnterRemap = remap;
    Q_EMIT this->MouseEnterRemapChanged(remap);
}

void WacomService::SetKeyDownAction(const QString &action)
{
    m_keyDownAction = action;
    Q_EMIT this->KeyDownActionChanged(action);
}

void WacomService::SetKeyUpAction(const QString &action)
{
    m_keyUpAction = action;
    Q_EMIT this->KeyUpActionChanged(action);
}

void WacomService::SetEraserPressureSensitive(quint32 sensitivity)
{
    m_eraserPressureSensitive = sensitivity;
    Q_EMIT this->EraserPressureSensitiveChanged(sensitivity);
}

void WacomService::SetEraserRawSample(quint32 sampleSize)
{
    m_eraserRawSample = sampleSize;
    Q_EMIT this->EraserRawSampleChanged(sampleSize);
}

void WacomService::SetEraserThreshold(quint32 threshold)
{
    m_eraserThreshold = threshold;
    Q_EMIT this->EraserThresholdChanged(threshold);
}

void WacomService::SetStylusPressureSensitive(quint32 sensitivity)
{
    m_stylusPressureSensitive = sensitivity;
    Q_EMIT this->StylusPressureSensitiveChanged(sensitivity);
}

void WacomService::SetStylusRawSample(quint32 sampleSize)
{
    m_stylusRawSample = sampleSize;
    Q_EMIT this->StylusRawSampleChanged(sampleSize);
}

void WacomService::SetStylusThreshold(quint32 threshold)
{
    m_stylusThreshold = threshold;
    Q_EMIT this->StylusThresholdChanged(threshold);
}

void WacomService::SetSuppress(quint32 suppress)
{
    m_suppress = suppress;
    Q_EMIT this->SuppressChanged(suppress);
}

void WacomService::Reset()
{
    m_cursorMode = CursorModeDefault;
    m_exist = ExistDefault;
    m_forceProportions = ForceProportionsDefault;
    m_leftHanded = LeftHandedDefault;
    m_mouseEnterRemap = MouseEnterRemapDefault;
    m_deviceList = DeviceListDefault;
    m_keyDownAction = KeyDownActionDefault;
    m_keyUpAction = KeyUpActionDefault;
    m_mapOutput = MapOutputDefault;
    m_eraserPressureSensitive = EraserPressureSensitiveDefault;
    m_eraserRawSample = EraserRawSampleDefault;
    m_eraserThreshold = EraserThresholdDefault;
    m_stylusPressureSensitive = StylusPressureSensitiveDefault;
    m_stylusRawSample = StylusRawSampleDefault;
    m_stylusThreshold = StylusThresholdDefault;
    m_suppress = SuppressDefault;
}

DDEVICE_END_NAMESPACE
