// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "inputdevicesinterface.h"
#include <QtDBus>

DDEVICE_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DDBusInterface;
InputDevicesInterface::InputDevicesInterface(const QString &service,
                                             const QString &path,
                                             QDBusConnection connection,
                                             QObject *parent)
    : QObject(parent)
    , m_interface(new DDBusInterface(service, path, staticInterfaceName(), connection, this))
{
}

quint32 InputDevicesInterface::WheelSpeed() const
{
    return qdbus_cast<quint32>(m_interface->property("WheelSpeed"));
}

void InputDevicesInterface::SetWheelSpeed(quint32 speed)
{
    m_interface->setProperty("WheelSpeed", QVariant::fromValue(speed));
}
DDEVICE_END_NAMESPACE
