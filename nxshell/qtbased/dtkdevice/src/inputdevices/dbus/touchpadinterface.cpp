// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "touchpadinterface.h"
#include <QtDBus>

DDEVICE_BEGIN_NAMESPACE
TouchPadInterface::TouchPadInterface(const QString &service, const QString &path, QDBusConnection connection, QObject *parent)
    : QObject(parent)
    , m_interface(new DDBusInterface(service, path, staticInterfaceName(), connection, this))
{
}

bool TouchPadInterface::DisableIfTyping() const
{
    return qdbus_cast<bool>(m_interface->property("DisableIfTyping"));
}

bool TouchPadInterface::EdgeScroll() const
{
    return qdbus_cast<bool>(m_interface->property("EdgeScroll"));
}

bool TouchPadInterface::Exist() const
{
    return qdbus_cast<bool>(m_interface->property("Exist"));
}

bool TouchPadInterface::LeftHanded() const
{
    return qdbus_cast<bool>(m_interface->property("LeftHanded"));
}

bool TouchPadInterface::TapClick() const
{
    return qdbus_cast<bool>(m_interface->property("TapClick"));
}

bool TouchPadInterface::NaturalScroll() const
{
    return qdbus_cast<bool>(m_interface->property("NaturalScroll"));
}

bool TouchPadInterface::PalmDetect() const
{
    return qdbus_cast<bool>(m_interface->property("PalmDetect"));
}

bool TouchPadInterface::TPadEnable() const
{
    return qdbus_cast<bool>(m_interface->property("TPadEnable"));
}

bool TouchPadInterface::VertScroll() const
{
    return qdbus_cast<bool>(m_interface->property("VertScroll"));
}

double TouchPadInterface::MotionAcceleration() const
{
    return qdbus_cast<double>(m_interface->property("MotionAcceleration"));
}

qint32 TouchPadInterface::DeltaScroll() const
{
    return qdbus_cast<qint32>(m_interface->property("DeltaScroll"));
}

qint32 TouchPadInterface::DoubleClick() const
{
    return qdbus_cast<qint32>(m_interface->property("DoubleClick"));
}

qint32 TouchPadInterface::DragThreshold() const
{
    return qdbus_cast<qint32>(m_interface->property("DragThreshold"));
}

qint32 TouchPadInterface::PalmMinWidth() const
{
    return qdbus_cast<qint32>(m_interface->property("PalmMinWidth"));
}

qint32 TouchPadInterface::PalmMinZ() const
{
    return qdbus_cast<qint32>(m_interface->property("PalmMinZ"));
}

QString TouchPadInterface::DeviceList() const
{
    return qdbus_cast<QString>(m_interface->property("DeviceList"));
}

void TouchPadInterface::SetDisableIfTyping(bool disable)
{
    m_interface->setProperty("DisableIfTyping", QVariant::fromValue(disable));
}

void TouchPadInterface::SetEdgeScroll(bool edgeScroll)
{
    m_interface->setProperty("EdgeScroll", QVariant::fromValue(edgeScroll));
}

void TouchPadInterface::SetLeftHanded(bool leftHanded)
{
    m_interface->setProperty("LeftHanded", QVariant::fromValue(leftHanded));
}

void TouchPadInterface::SetNaturalScroll(bool naturalScroll)
{
    m_interface->setProperty("NaturalScroll", QVariant::fromValue(naturalScroll));
}

void TouchPadInterface::SetPalmDetect(bool palmDetect)
{
    m_interface->setProperty("PalmDetect", QVariant::fromValue(palmDetect));
}

void TouchPadInterface::SetTPadEnable(bool enable)
{
    m_interface->setProperty("TPadEnable", QVariant::fromValue(enable));
}

void TouchPadInterface::SetTapClick(bool tapClick)
{
    m_interface->setProperty("TapClick", QVariant::fromValue(tapClick));
}

void TouchPadInterface::SetVertScroll(bool vertScroll)
{
    m_interface->setProperty("VertScroll", QVariant::fromValue(vertScroll));
}

void TouchPadInterface::SetMotionAcceleration(double acceleration)
{
    m_interface->setProperty("MotionAcceleration", QVariant::fromValue(acceleration));
}

void TouchPadInterface::SetDeltaScroll(qint32 deltaScroll)
{
    m_interface->setProperty("DeltaScroll", QVariant::fromValue(deltaScroll));
}

void TouchPadInterface::SetDoubleClick(qint32 interval)
{
    m_interface->setProperty("DoubleClick", QVariant::fromValue(interval));
}

void TouchPadInterface::SetDragThreshold(qint32 threshold)
{
    m_interface->setProperty("DragThreshold", QVariant::fromValue(threshold));
}

void TouchPadInterface::SetPalmMinWidth(qint32 minWidth)
{
    m_interface->setProperty("PalmMinWidth", QVariant::fromValue(minWidth));
}

void TouchPadInterface::SetPalmMinZ(qint32 minZ)
{
    m_interface->setProperty("PalmMinZ", QVariant::fromValue(minZ));
}

QDBusPendingReply<> TouchPadInterface::Reset()
{
    return m_interface->asyncCall("Reset");
}

DDEVICE_END_NAMESPACE
