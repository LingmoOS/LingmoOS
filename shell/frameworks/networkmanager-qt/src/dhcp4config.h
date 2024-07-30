/*
    SPDX-FileCopyrightText: 2011-2013 Lamarque V. Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_DHCP4CONFIG_H
#define NETWORKMANAGERQT_DHCP4CONFIG_H

#include "generictypes.h"

#include <networkmanagerqt/networkmanagerqt_export.h>

#include <QSharedPointer>

namespace NetworkManager
{
class Dhcp4ConfigPrivate;

/**
 * This class represents dhcp4 configuration
 */
class NETWORKMANAGERQT_EXPORT Dhcp4Config : public QObject
{
    Q_OBJECT
public:
    typedef QSharedPointer<Dhcp4Config> Ptr;
    typedef QList<Ptr> List;

    explicit Dhcp4Config(const QString &path, QObject *owner = nullptr);
    ~Dhcp4Config() override;

    QString path() const;

    QVariantMap options() const;

    QString optionValue(const QString &key) const;

Q_SIGNALS:
    void optionsChanged(const QVariantMap &);

private:
    Q_DECLARE_PRIVATE(Dhcp4Config)

    Dhcp4ConfigPrivate *const d_ptr;
};
} // namespace NetworkManager

#endif // NETWORKMANAGERQT_DHCP4CONFIG_H
