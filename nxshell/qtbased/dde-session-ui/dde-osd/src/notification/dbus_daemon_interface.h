// SPDX-FileCopyrightText: 2014 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUS_DAEMON_INTERFACE_H_1413787035
#define DBUS_DAEMON_INTERFACE_H_1413787035

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface org.freedesktop.DBus
 */
class DBusDaemonInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.freedesktop.DBus"; }

public:
    DBusDaemonInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~DBusDaemonInterface();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> AddMatch(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QLatin1String("AddMatch"), argumentList);
    }

    inline QDBusPendingReply<QByteArray> GetAdtAuditSessionData(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QLatin1String("GetAdtAuditSessionData"), argumentList);
    }

    inline QDBusPendingReply<QString> GetConnectionAppArmorSecurityContext(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QLatin1String("GetConnectionAppArmorSecurityContext"), argumentList);
    }

    inline QDBusPendingReply<QByteArray> GetConnectionSELinuxSecurityContext(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QLatin1String("GetConnectionSELinuxSecurityContext"), argumentList);
    }

    inline QDBusPendingReply<uint> GetConnectionUnixProcessID(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QLatin1String("GetConnectionUnixProcessID"), argumentList);
    }

    inline QDBusPendingReply<uint> GetConnectionUnixUser(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QLatin1String("GetConnectionUnixUser"), argumentList);
    }

    inline QDBusPendingReply<QString> GetId()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("GetId"), argumentList);
    }

    inline QDBusPendingReply<QString> GetNameOwner(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QLatin1String("GetNameOwner"), argumentList);
    }

    inline QDBusPendingReply<QString> Hello()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("Hello"), argumentList);
    }

    inline QDBusPendingReply<QStringList> ListActivatableNames()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("ListActivatableNames"), argumentList);
    }

    inline QDBusPendingReply<QStringList> ListNames()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("ListNames"), argumentList);
    }

    inline QDBusPendingReply<QStringList> ListQueuedOwners(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QLatin1String("ListQueuedOwners"), argumentList);
    }

    inline QDBusPendingReply<bool> NameHasOwner(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QLatin1String("NameHasOwner"), argumentList);
    }

    inline QDBusPendingReply<uint> ReleaseName(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QLatin1String("ReleaseName"), argumentList);
    }

    inline QDBusPendingReply<> ReloadConfig()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("ReloadConfig"), argumentList);
    }

    inline QDBusPendingReply<> RemoveMatch(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QLatin1String("RemoveMatch"), argumentList);
    }

    inline QDBusPendingReply<uint> RequestName(const QString &in0, uint in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QLatin1String("RequestName"), argumentList);
    }

    inline QDBusPendingReply<uint> StartServiceByName(const QString &in0, uint in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QLatin1String("StartServiceByName"), argumentList);
    }

    inline QDBusPendingReply<> UpdateActivationEnvironment(const QMap<QString, QString> &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QLatin1String("UpdateActivationEnvironment"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void NameAcquired(const QString &in0);
    void NameLost(const QString &in0);
    void NameOwnerChanged(const QString &in0, const QString &in1, const QString &in2);
};
#endif
