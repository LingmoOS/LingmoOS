/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "input.h"
#include "input_p.h"
#include "macros.h"
#include "utils.h"

#include <QVariantMap>

namespace BluezQt
{
static Input::ReconnectMode stringToReconnectMode(const QString &mode)
{
    if (mode == QLatin1String("none")) {
        return Input::NoReconnect;
    } else if (mode == QLatin1String("host")) {
        return Input::HostReconnect;
    } else if (mode == QLatin1String("device")) {
        return Input::DeviceReconnect;
    }
    return Input::AnyReconnect;
}

InputPrivate::InputPrivate(const QString &path, const QVariantMap &properties)
    : QObject()
    , m_path(path)
{
    // Init properties
    m_reconnectMode = stringToReconnectMode(properties.value(QStringLiteral("ReconnectMode")).toString());
}

void InputPrivate::propertiesChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated)
{
    Q_UNUSED(invalidated)

    if (interface != Strings::orgBluezInput1()) {
        return;
    }

    QVariantMap::const_iterator i;
    for (i = changed.constBegin(); i != changed.constEnd(); ++i) {
        const QVariant &value = i.value();
        const QString &property = i.key();

        if (property == QLatin1String("ReconnectMode")) {
            PROPERTY_CHANGED2(m_reconnectMode, stringToReconnectMode(value.toString()), reconnectModeChanged);
        }
    }
}

Input::Input(const QString &path, const QVariantMap &properties)
    : d(new InputPrivate(path, properties))
{
}

Input::~Input() = default;

InputPtr Input::toSharedPtr() const
{
    return d->q.toStrongRef();
}

Input::ReconnectMode Input::reconnectMode() const
{
    return d->m_reconnectMode;
}

} // namespace BluezQt

#include "moc_input.cpp"
#include "moc_input_p.cpp"
