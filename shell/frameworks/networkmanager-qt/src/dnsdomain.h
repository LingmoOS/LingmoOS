/*
    SPDX-FileCopyrightText: 2018 Aleksander Morgado <aleksander@aleksander.es>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_DNSDOMAIN_H
#define NETWORKMANAGERQT_DNSDOMAIN_H

#include "ipaddress.h"
#include "iproute.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

// To prevent signals in glib2 be defined by QT
#undef signals
#include <libnm/NetworkManager.h>
#include <nm-version.h>
#define signals Q_SIGNALS

#include <QStringList>

namespace NetworkManager
{
/**
 * This class represents the configuration for a DNS domain
 */
class NETWORKMANAGERQT_EXPORT DnsDomain
{
public:
    /**
     * Constructs a DnsDomain object with a list of
     */
    DnsDomain(const QString &name, const QList<QHostAddress> &servers, const QStringList &options);

    /**
     * Constructs a DnsDomain object
     */
    DnsDomain();

    /**
     * Destroys this DnsDomain object.
     */
    ~DnsDomain();

    /**
     * Constructs a DnsDomain object that is a copy of the object @p other.
     */
    DnsDomain(const DnsDomain &other);

    /**
     * Returns the domain name
     */
    QString name() const;

    /**
     * Sets the domain name
     */
    void setName(const QString &name);

    /**
     * Returns the list of servers
     */
    QList<QHostAddress> servers() const;

    /**
     * Sets the list of servers
     */
    void setServers(const QList<QHostAddress> &list);

    /**
     * Returns the list of resolver options
     */
    QStringList options() const;

    /**
     * Sets the list of resolver options
     */
    void setOptions(const QStringList &list);

    /**
     * Makes a copy of the DnsDomain object @p other.
     */
    DnsDomain &operator=(const DnsDomain &other);

private:
    class Private;
    Private *const d;
};

} // namespace NetworkManager

#endif // NETWORKMANAGERQT_DNSDOMAIN_H
