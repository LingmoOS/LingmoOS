/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "object.h"

#include <QDBusConnection>
#include <QDBusMessage>

Object::Object()
    : m_parent(nullptr)
{
}

QObject *Object::objectParent() const
{
    return m_parent;
}

void Object::setObjectParent(QObject *parent)
{
    m_parent = parent;
}

QDBusObjectPath Object::path() const
{
    return m_path;
}

void Object::setPath(const QDBusObjectPath &path)
{
    m_path = path;
}

QString Object::name() const
{
    return m_name;
}

void Object::setName(const QString &name)
{
    m_name = name;
}

bool Object::haveProperties() const
{
    return !m_properties.isEmpty();
}

QVariantMap Object::properties() const
{
    return m_properties;
}

void Object::setProperties(const QVariantMap &properties)
{
    m_properties = properties;
}

QVariant Object::property(const QString &name) const
{
    return m_properties.value(name);
}

void Object::changeProperty(const QString &name, const QVariant &value)
{
    if (m_properties.value(name) == value) {
        return;
    }

    QVariantMap updatedProperties;
    updatedProperties[name] = value;

    m_properties[name] = value;

    QDBusMessage signal = QDBusMessage::createSignal(m_path.path(), QStringLiteral("org.freedesktop.DBus.Properties"), QStringLiteral("PropertiesChanged"));
    signal << m_name;
    signal << updatedProperties;
    signal << QStringList();
    QDBusConnection::sessionBus().send(signal);
}
