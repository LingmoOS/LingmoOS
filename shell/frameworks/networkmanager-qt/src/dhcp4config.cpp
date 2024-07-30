/*
    SPDX-FileCopyrightText: 2011 Lamarque V. Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "dhcp4config.h"
#include "dhcp4config_p.h"
#include "manager_p.h"
#include "nmdebug.h"

NetworkManager::Dhcp4ConfigPrivate::Dhcp4ConfigPrivate(const QString &path, Dhcp4Config *q)
#ifdef NMQT_STATIC
    : dhcp4Iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    : dhcp4Iface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
    , myPath(path)
    , q_ptr(q)
{
}

NetworkManager::Dhcp4ConfigPrivate::~Dhcp4ConfigPrivate()
{
}

NetworkManager::Dhcp4Config::Dhcp4Config(const QString &path, QObject *owner)
    : d_ptr(new Dhcp4ConfigPrivate(path, this))
{
    Q_D(Dhcp4Config);
    Q_UNUSED(owner);

    QDBusConnection::systemBus().connect(NetworkManagerPrivate::DBUS_SERVICE,
                                         d->myPath,
                                         NetworkManagerPrivate::FDO_DBUS_PROPERTIES,
                                         QLatin1String("PropertiesChanged"),
                                         d,
                                         SLOT(dbusPropertiesChanged(QString, QVariantMap, QStringList)));
    d->options = d->dhcp4Iface.options();
}

NetworkManager::Dhcp4Config::~Dhcp4Config()
{
    delete d_ptr;
}

QString NetworkManager::Dhcp4Config::path() const
{
    Q_D(const Dhcp4Config);
    return d->myPath;
}

QVariantMap NetworkManager::Dhcp4Config::options() const
{
    Q_D(const Dhcp4Config);
    return d->options;
}

QString NetworkManager::Dhcp4Config::optionValue(const QString &key) const
{
    Q_D(const Dhcp4Config);
    QString value;
    if (d->options.contains(key)) {
        value = d->options.value(key).toString();
    }
    return value;
}

void NetworkManager::Dhcp4ConfigPrivate::dbusPropertiesChanged(const QString &interfaceName,
                                                               const QVariantMap &properties,
                                                               const QStringList &invalidatedProperties)
{
    Q_UNUSED(invalidatedProperties);
    if (interfaceName == QLatin1String("org.freedesktop.NetworkManager.DHCP4Config")) {
        dhcp4PropertiesChanged(properties);
    }
}

void NetworkManager::Dhcp4ConfigPrivate::dhcp4PropertiesChanged(const QVariantMap &properties)
{
    Q_Q(Dhcp4Config);

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

#include "moc_dhcp4config.cpp"
#include "moc_dhcp4config_p.cpp"
