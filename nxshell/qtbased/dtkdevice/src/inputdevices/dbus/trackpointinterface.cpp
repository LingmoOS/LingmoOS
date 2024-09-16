// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "trackpointinterface.h"
#include <QtDBus>

DDEVICE_BEGIN_NAMESPACE

TrackPointInterface::TrackPointInterface(const QString &service, const QString &path, QDBusConnection connection, QObject *parent)
    : QObject(parent)
    , m_interface(new DDBusInterface(service, path, staticInterfaceName(), connection, this))
{
}

bool TrackPointInterface::Exist() const
{
    return qdbus_cast<bool>(m_interface->property("Exist"));
}

bool TrackPointInterface::LeftHanded() const
{
    return qdbus_cast<bool>(m_interface->property("LeftHanded"));
}

bool TrackPointInterface::MiddleButtonEnabled() const
{
    return qdbus_cast<bool>(m_interface->property("MiddleButtonEnabled"));
}

bool TrackPointInterface::WheelEmulation() const
{
    return qdbus_cast<bool>(m_interface->property("WheelEmulation"));
}

bool TrackPointInterface::WheelHorizScroll() const
{
    return qdbus_cast<bool>(m_interface->property("WheelHorizScroll"));
}

double TrackPointInterface::MotionAcceleration() const
{
    return qdbus_cast<double>(m_interface->property("MotionAcceleration"));
}

qint32 TrackPointInterface::MiddleButtonTimeout() const
{
    return qdbus_cast<qint32>(m_interface->property("MiddleButtonTimeout"));
}

qint32 TrackPointInterface::WheelEmulationButton() const
{
    return qdbus_cast<qint32>(m_interface->property("WheelEmulationButton"));
}

qint32 TrackPointInterface::WheelEmulationTimeout() const
{
    return qdbus_cast<qint32>(m_interface->property("WheelEmulationTimeout"));
}

QString TrackPointInterface::DeviceList() const
{
    return qdbus_cast<QString>(m_interface->property("DeviceList"));
}

void TrackPointInterface::SetLeftHanded(bool leftHanded)
{
    m_interface->setProperty("LeftHanded", QVariant::fromValue(leftHanded));
}

void TrackPointInterface::SetMiddleButtonEnabled(bool enabled)
{
    m_interface->setProperty("MiddleButtonEnabled", QVariant::fromValue(enabled));
}

void TrackPointInterface::SetWheelEmulation(bool emulation)
{
    m_interface->setProperty("WheelEmulation", QVariant::fromValue(emulation));
}

void TrackPointInterface::SetWheelHorizScroll(bool horizScroll)
{
    m_interface->setProperty("WheelHorizScroll", QVariant::fromValue(horizScroll));
}

void TrackPointInterface::SetMotionAcceleration(double acceleration)
{
    m_interface->setProperty("MotionAcceleration", QVariant::fromValue(acceleration));
}

void TrackPointInterface::SetMiddleButtonTimeout(qint32 timeout)
{
    m_interface->setProperty("MiddleButtonTimeout", QVariant::fromValue(timeout));
}

void TrackPointInterface::SetWheelEmulationButton(qint32 button)
{
    m_interface->setProperty("WheelEmulationButton", QVariant::fromValue(button));
}

void TrackPointInterface::SetWheelEmulationTimeout(qint32 timeout)
{
    m_interface->setProperty("WheelEmulationTimeout", QVariant::fromValue(timeout));
}

QDBusPendingReply<> TrackPointInterface::Reset()
{
    return m_interface->asyncCall("Reset");
}

DDEVICE_END_NAMESPACE
