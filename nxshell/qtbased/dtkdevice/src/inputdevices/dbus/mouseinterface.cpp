// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mouseinterface.h"
#include <QtDBus>
DDEVICE_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DDBusInterface;

MouseInterface::MouseInterface(const QString &service, const QString &path, QDBusConnection connection, QObject *parent)
    : QObject(parent)
    , m_interface(new DDBusInterface(service, path, staticInterfaceName(), connection, this))
{
}

bool MouseInterface::AdaptiveAccelProfile() const
{
    return qdbus_cast<bool>(m_interface->property("AdaptiveAccelProfile"));
}

bool MouseInterface::DisableTpad() const
{
    return qdbus_cast<bool>(m_interface->property("DisableTpad"));
}

bool MouseInterface::Exist() const
{
    return qdbus_cast<bool>(m_interface->property("Exist"));
}

bool MouseInterface::LeftHanded() const
{
    return qdbus_cast<bool>(m_interface->property("LeftHanded"));
}

bool MouseInterface::MiddleButtonEmulation() const
{
    return qdbus_cast<bool>(m_interface->property("MiddleButtonEmulation"));
}

bool MouseInterface::NaturalScroll() const
{
    return qdbus_cast<bool>(m_interface->property("NaturalScroll"));
}

double MouseInterface::MotionAcceleration() const
{
    return qdbus_cast<double>(m_interface->property("MotionAcceleration"));
}

qint32 MouseInterface::DoubleClick() const
{
    return qdbus_cast<qint32>(m_interface->property("DoubleClick"));
}

qint32 MouseInterface::DragThreshold() const
{
    return qdbus_cast<qint32>(m_interface->property("DragThreshold"));
}

QString MouseInterface::DeviceList() const
{
    return qdbus_cast<QString>(m_interface->property("DeviceList"));
}

void MouseInterface::SetAdaptiveAccelProfile(bool adaptive)
{
    m_interface->setProperty("AdaptiveAccelProfile", QVariant::fromValue(adaptive));
}

void MouseInterface::SetDisableTpad(bool disable)
{
    m_interface->setProperty("DisableTpad", QVariant::fromValue(disable));
}

void MouseInterface::SetLeftHanded(bool leftHanded)
{
    m_interface->setProperty("LeftHanded", QVariant::fromValue(leftHanded));
}

void MouseInterface::SetMiddleButtonEmulation(bool emulation)
{
    m_interface->setProperty("MiddleButtonEmulation", QVariant::fromValue(emulation));
}

void MouseInterface::SetNaturalScroll(bool naturalScroll)
{
    m_interface->setProperty("NaturalScroll", QVariant::fromValue(naturalScroll));
}

void MouseInterface::SetMotionAcceleration(double acceleration)
{
    m_interface->setProperty("MotionAcceleration", QVariant::fromValue(acceleration));
}

void MouseInterface::SetDoubleClick(qint32 interval)
{
    m_interface->setProperty("DoubleClick", QVariant::fromValue(interval));
}

void MouseInterface::SetDragThreshold(qint32 threshold)
{
    m_interface->setProperty("DragThreshold", QVariant::fromValue(threshold));
}

QDBusPendingReply<> MouseInterface::Reset()
{
    return m_interface->asyncCall("Reset");
}

DDEVICE_END_NAMESPACE
