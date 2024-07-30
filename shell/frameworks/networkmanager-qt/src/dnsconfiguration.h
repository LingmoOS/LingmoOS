/*
    SPDX-FileCopyrightText: 2018 Aleksander Morgado <aleksander@aleksander.es>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_DNSCONFIGURATION_H
#define NETWORKMANAGERQT_DNSCONFIGURATION_H

#include "dnsdomain.h"
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
 * This class represents IP configuration
 */
class NETWORKMANAGERQT_EXPORT DnsConfiguration
{
public:
    /**
     * Constructs an initialized DnsConfiguration object
     */
    DnsConfiguration(const QStringList &searches, const QStringList &options, const QList<DnsDomain> domains);

    /**
     * Constructs an empty DnsConfiguration object
     */
    DnsConfiguration();

    /**
     * Destroys this DnsConfiguration object.
     */
    ~DnsConfiguration();

    /**
     * Constructs a DnsConfiguration object that is a copy of the object @p other.
     */
    DnsConfiguration(const DnsConfiguration &other);

    /**
     * Returns the list of search domains
     */
    QStringList searches() const;

    /**
     * Sets the list of search domains
     */
    void setSearches(const QStringList &list);

    /**
     * Returns the list of resolver options
     */
    QStringList options() const;

    /**
     * Sets the list of resolver options
     */
    void setOptions(const QStringList &list);

    /**
     * Returns the list of domains
     */
    QList<DnsDomain> domains() const;

    /**
     * Sets the list of domains
     */
    void setDomains(const QList<DnsDomain> &domains);

    /**
     * Marshall into a map
     */
    QVariantMap toMap() const;

    /**
     * De-marshall from a map
     */
    void fromMap(const QVariantMap &map);

    /**
     * Makes a copy of the DnsConfiguration object @p other.
     */
    DnsConfiguration &operator=(const DnsConfiguration &other);

private:
    class Private;
    Private *const d;
};

} // namespace NetworkManager

#endif // NETWORKMANAGERQT_DNSCONFIGURATION_H
