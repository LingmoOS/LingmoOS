// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "wacominterface.h"
#include <QtDBus>

DDEVICE_BEGIN_NAMESPACE
WacomInterface::WacomInterface(const QString &service, const QString &path, QDBusConnection connection, QObject *parent)
    : QObject(parent)
    , m_interface(new DDBusInterface(service, path, staticInterfaceName(), connection, this))
{
}

bool WacomInterface::CursorMode() const
{
    return qdbus_cast<bool>(m_interface->property("CursorMode"));
}

bool WacomInterface::Exist() const
{
    return qdbus_cast<bool>(m_interface->property("Exist"));
}

bool WacomInterface::ForceProportions() const
{
    return qdbus_cast<bool>(m_interface->property("ForceProportions"));
}

bool WacomInterface::LeftHanded() const
{
    return qdbus_cast<bool>(m_interface->property("LeftHanded"));
}

bool WacomInterface::MouseEnterRemap() const
{
    return qdbus_cast<bool>(m_interface->property("MouseEnterRemap"));
}

QString WacomInterface::DeviceList() const
{
    return qdbus_cast<QString>(m_interface->property("DeviceList"));
}

QString WacomInterface::KeyDownAction() const
{
    return qdbus_cast<QString>(m_interface->property("KeyDownAction"));
}

QString WacomInterface::KeyUpAction() const
{
    return qdbus_cast<QString>(m_interface->property("KeyUpAction"));
}

QString WacomInterface::MapOutput() const
{
    return qdbus_cast<QString>(m_interface->property("MapOutput"));
}

quint32 WacomInterface::EraserPressureSensitive() const
{
    return qdbus_cast<quint32>(m_interface->property("EraserPressureSensitive"));
}

quint32 WacomInterface::EraserRawSample() const
{
    return qdbus_cast<quint32>(m_interface->property("EraserRawSample"));
}

quint32 WacomInterface::EraserThreshold() const
{
    return qdbus_cast<quint32>(m_interface->property("EraserThreshold"));
}

quint32 WacomInterface::StylusPressureSensitive() const
{
    return qdbus_cast<quint32>(m_interface->property("StylusPressureSensitive"));
}

quint32 WacomInterface::StylusRawSample() const
{
    return qdbus_cast<quint32>(m_interface->property("StylusRawSample"));
}

quint32 WacomInterface::StylusThreshold() const
{
    return qdbus_cast<quint32>(m_interface->property("StylusThreshold"));
}

quint32 WacomInterface::Suppress() const
{
    return qdbus_cast<quint32>(m_interface->property("Suppress"));
}

void WacomInterface::SetCursorMode(bool cursorMode)
{
    m_interface->setProperty("CursorMode", QVariant::fromValue(cursorMode));
}

void WacomInterface::SetForceProportions(bool forceProportions)
{
    m_interface->setProperty("ForceProportions", QVariant::fromValue(forceProportions));
}

void WacomInterface::SetLeftHanded(bool leftHanded)
{
    m_interface->setProperty("LeftHanded", QVariant::fromValue(leftHanded));
}

void WacomInterface::SetMouseEnterRemap(bool remap)
{
    m_interface->setProperty("MouseEnterRemap", QVariant::fromValue(remap));
}

void WacomInterface::SetKeyDownAction(const QString &action)
{
    m_interface->setProperty("KeyDownAction", QVariant::fromValue(action));
}

void WacomInterface::SetKeyUpAction(const QString &action)
{
    m_interface->setProperty("KeyUpAction", QVariant::fromValue(action));
}

void WacomInterface::SetEraserPressureSensitive(quint32 sensitivity)
{
    m_interface->setProperty("EraserPressureSensitive", QVariant::fromValue(sensitivity));
}

void WacomInterface::SetEraserRawSample(quint32 sampleSize)
{
    m_interface->setProperty("EraserRawSample", QVariant::fromValue(sampleSize));
}

void WacomInterface::SetEraserThreshold(quint32 threshold)
{
    m_interface->setProperty("EraserThreshold", QVariant::fromValue(threshold));
}

void WacomInterface::SetStylusPressureSensitive(quint32 sensitivity)
{
    m_interface->setProperty("StylusPressureSensitive", QVariant::fromValue(sensitivity));
}

void WacomInterface::SetStylusRawSample(quint32 sampleSize)
{
    m_interface->setProperty("StylusRawSample", QVariant::fromValue(sampleSize));
}

void WacomInterface::SetStylusThreshold(quint32 threshold)
{
    m_interface->setProperty("StylusThreshold", QVariant::fromValue(threshold));
}

void WacomInterface::SetSuppress(quint32 suppress)
{
    m_interface->setProperty("Suppress", QVariant::fromValue(suppress));
}

QDBusPendingReply<> WacomInterface::Reset()
{
    return m_interface->asyncCall("Reset");
}

DDEVICE_END_NAMESPACE
