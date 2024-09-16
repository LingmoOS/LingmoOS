// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "keyboardinterface.h"

DDEVICE_BEGIN_NAMESPACE
KeyboardInterface::KeyboardInterface(const QString &service, const QString &path, QDBusConnection connection, QObject *parent)
    : QObject(parent)
    , m_interface(new DDBusInterface(service, path, staticInterfaceName(), connection, this))
{
}

bool KeyboardInterface::RepeatEnabled() const
{
    return qdbus_cast<bool>(m_interface->property("RepeatEnabled"));
}

qint32 KeyboardInterface::CursorBlink() const
{
    return qdbus_cast<qint32>(m_interface->property("CursorBlink"));
}

quint32 KeyboardInterface::RepeatDelay() const
{
    return qdbus_cast<quint32>(m_interface->property("RepeatDelay"));
}

quint32 KeyboardInterface::RepeatInterval() const
{
    return qdbus_cast<quint32>(m_interface->property("RepeatInterval"));
}

void KeyboardInterface::SetRepeatEnabled(bool enabled)
{
    m_interface->setProperty("RepeatEnabled", QVariant::fromValue(enabled));
}

void KeyboardInterface::SetCursorBlink(qint32 cursorBlink)
{
    m_interface->setProperty("CursorBlink", QVariant::fromValue(cursorBlink));
}

void KeyboardInterface::SetRepeatDelay(quint32 delay)
{
    m_interface->setProperty("RepeatDelay", QVariant::fromValue(delay));
}

void KeyboardInterface::SetRepeatInterval(quint32 interval)
{
    m_interface->setProperty("RepeatInterval", QVariant::fromValue(interval));
}

QDBusPendingReply<> KeyboardInterface::Reset()
{
    return m_interface->asyncCall("Reset");
}
DDEVICE_END_NAMESPACE
