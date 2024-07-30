/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2021 Ivan Podkurkov <podkiva2@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "gattdescriptorremote_p.h"
#include "gattdescriptorremote.h"
#include "device_p.h"
#include "device.h"
#include "adapter.h"
#include "input.h"
#include "input_p.h"
#include "mediaplayer.h"
#include "mediaplayer_p.h"
#include "utils.h"
#include "macros.h"

namespace BluezQt
{

GattDescriptorRemotePrivate::GattDescriptorRemotePrivate(const QString &path, const QVariantMap &properties, const GattCharacteristicRemotePtr &characteristic)
    : QObject()
    , m_dbusProperties(nullptr)
    , m_handle()
    , m_characteristic(characteristic)
{
    m_bluezGattDescriptor = new BluezGattDescriptor(Strings::orgBluez(), path, DBusConnection::orgBluez(), this);

    init(properties);
}

void GattDescriptorRemotePrivate::init(const QVariantMap &properties)
{
    m_dbusProperties = new DBusProperties(Strings::orgBluez(), m_bluezGattDescriptor->path(),
                                          DBusConnection::orgBluez(), this);

    // Init properties
    m_uuid = properties.value(QStringLiteral("UUID")).toString();
    m_value = properties.value(QStringLiteral("Value")).toByteArray();
    m_flags = properties.value(QStringLiteral("Flags")).toStringList();
    m_handle = properties.value(QStringLiteral("Handle")).value<quint16>();
}

QDBusPendingReply<> GattDescriptorRemotePrivate::setDBusProperty(const QString &name, const QVariant &value)
{
    return m_dbusProperties->Set(Strings::orgBluezGattDescriptor1(), name, QDBusVariant(value));
}

void GattDescriptorRemotePrivate::propertiesChanged(const QString &path, const QString &interface, const QVariantMap &changed, const QStringList &invalidated)
{
    Q_UNUSED(path)

    if (interface != Strings::orgBluezGattDescriptor1()) {
        return;
    }

    QVariantMap::const_iterator i;
    for (i = changed.constBegin(); i != changed.constEnd(); ++i) {
        const QVariant &value = i.value();
        const QString &property = i.key();

        if (property == QLatin1String("UUID")) {
            PROPERTY_CHANGED(m_uuid, toString, uuidChanged);
        } else if (property == QLatin1String("Value")) {
            PROPERTY_CHANGED(m_value, toByteArray, valueChanged);
        } else if (property == QLatin1String("Flags")) {
            PROPERTY_CHANGED2(m_flags, value.toStringList(), flagsChanged);
        } else if (property == QLatin1String("Handle")) {
            PROPERTY_CHANGED2(m_handle, value.value<quint16>(), handleChanged);
        }
    }

    for (auto& property : invalidated) {
        if (property == QLatin1String("UUID")) {
            PROPERTY_INVALIDATED(m_uuid, QString(), uuidChanged);
        } else if (property == QLatin1String("Value")) {
            PROPERTY_INVALIDATED(m_value, QByteArray(), valueChanged);
        } else if (property == QLatin1String("Flags")) {
            PROPERTY_INVALIDATED(m_flags, QStringList(), flagsChanged);
        } else if (property == QLatin1String("Handle")) {
            PROPERTY_INVALIDATED(m_handle, quint16(), handleChanged);
        }
    }

    Q_EMIT q.lock().data()->descriptorChanged(q.toStrongRef());
}

} // namespace BluezQt

#include "moc_gattdescriptorremote_p.cpp"
