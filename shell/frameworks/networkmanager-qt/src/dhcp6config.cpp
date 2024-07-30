/*
    SPDX-FileCopyrightText: 2011 Lamarque V. Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "dhcp6config_p.h"
#include "manager_p.h"
#include "nmdebug.h"

NetworkManager::Dhcp6ConfigPrivate::Dhcp6ConfigPrivate(const QString &path, Dhcp6Config *q)
#ifdef NMQT_STATIC
    : dhcp6Iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    : dhcp6Iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
    , path(path)
    , q_ptr(q)
{
}

NetworkManager::Dhcp6ConfigPrivate::~Dhcp6ConfigPrivate()
{
}

NetworkManager::Dhcp6Config::Dhcp6Config(const QString &path, QObject *owner)
    : d_ptr(new Dhcp6ConfigPrivate(path, this))
{
    Q_D(Dhcp6Config);
    Q_UNUSED(owner);

    QDBusConnection::systemBus().connect(NetworkManagerPrivate::DBUS_SERVICE,
                                         d->path,
                                         NetworkManagerPrivate::FDO_DBUS_PROPERTIES,
                                         QLatin1String("PropertiesChanged"),
                                         d,
                                         SLOT(dbusPropertiesChanged(QString, QVariantMap, QStringList)));

    d->options = d->dhcp6Iface.options();
}

NetworkManager::Dhcp6Config::~Dhcp6Config()
{
    delete d_ptr;
}

QString NetworkManager::Dhcp6Config::path() const
{
    Q_D(const Dhcp6Config);
    return d->path;
}

QVariantMap NetworkManager::Dhcp6Config::options() const
{
    Q_D(const Dhcp6Config);
    return d->options;
}

QString NetworkManager::Dhcp6Config::optionValue(const QString &key) const
{
    Q_D(const Dhcp6Config);
    QString value;
    if (d->options.contains(key)) {
        value = d->options.value(key).toString();
    }
    return value;
}

void NetworkManager::Dhcp6ConfigPrivate::dbusPropertiesChanged(const QString &interfaceName,
                                                               const QVariantMap &properties,
                                                               const QStringList &invalidatedProperties)
{
    Q_UNUSED(invalidatedProperties);
    if (interfaceName == QLatin1String("org.freedesktop.NetworkManager.DHCP6Config")) {
        dhcp6PropertiesChanged(properties);
    }
}

void NetworkManager::Dhcp6ConfigPrivate::dhcp6PropertiesChanged(const QVariantMap &properties)
{
    Q_Q(Dhcp6Config);

    QVariantMap::const_iterator it = properties.constBegin();
    while (it != properties.constEnd()) {
        const QString property = it.key();
        if (property == QLatin1String("Options")) {
            options = it.value().toMap();
            Q_EMIT q->optionsChanged(options);
        } else {
            qCWarning(NMQT) << Q_FUNC_INFO << "Unhandled property" << property;
        }
        ++it;
    }
}

#include "moc_dhcp6config.cpp"
#include "moc_dhcp6config_p.cpp"
