/*
 *   SPDX-FileCopyrightText: 2017, 2018, 2019 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef PLASMAVAULT_COMMON_VAULT_INFO_H
#define PLASMAVAULT_COMMON_VAULT_INFO_H

#include <QDBusArgument>
#include <QDebug>
#include <QString>

namespace PlasmaVault
{
/**
 * Class used to serialize the vault data for DBus communication
 */
class VaultInfo
{
public:
    enum Status {
        NotInitialized = 0,
        Opened = 1,
        Closed = 2,

        Creating = 3,
        Opening = 4,
        Closing = 5,
        Dismantling = 6,
        Dismantled = 7,

        DeviceMissing = 254,

        Error = 255,
    };
    // Q_ENUM(Status); // VaultInfo is not a QObject or anything like that

    QString name;
    QString device;
    QString mountPoint;

    Status status;
    QString message;

    QStringList activities;
    bool isOfflineOnly;

    inline bool isInitialized() const
    {
        return status == Closed || status == Opened || status == Closing || status == Opening;
    }

    inline bool isOpened() const
    {
        return status == Opened;
    }

    inline bool isBusy() const
    {
        return status == Creating || status == Opening || status == Closing || status == Dismantling;
    }
};

typedef QList<VaultInfo> VaultInfoList;

inline QDebug &operator<<(QDebug &debug, const VaultInfo &vaultInfo)
{
    debug << vaultInfo.name << vaultInfo.device << vaultInfo.mountPoint

          << (quint16)vaultInfo.status << vaultInfo.message

          << vaultInfo.activities << vaultInfo.isOfflineOnly;
    return debug;
}

inline QDBusArgument &operator<<(QDBusArgument &argument, const VaultInfo &vaultInfo)
{
    argument.beginStructure();
    argument << vaultInfo.name << vaultInfo.device << vaultInfo.mountPoint

             << (quint16)vaultInfo.status << vaultInfo.message

             << vaultInfo.activities << vaultInfo.isOfflineOnly;
    argument.endStructure();
    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, VaultInfo &vaultInfo)
{
    quint16 status;
    argument.beginStructure();
    argument >> vaultInfo.name >> vaultInfo.device >> vaultInfo.mountPoint

        >> status >> vaultInfo.message

        >> vaultInfo.activities >> vaultInfo.isOfflineOnly;
    vaultInfo.status = (VaultInfo::Status)status;
    argument.endStructure();
    return argument;
}

} // namespace PlasmaVault

Q_DECLARE_METATYPE(PlasmaVault::VaultInfo)
Q_DECLARE_METATYPE(PlasmaVault::VaultInfoList)

#endif // include guard
