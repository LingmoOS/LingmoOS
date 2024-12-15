// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GRAPHICSDRIVERPROXY_H
#define GRAPHICSDRIVERPROXY_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface com.lingmo.daemon.GraphicsDriver
 */
class ComLingmoDaemonGraphicsDriverInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "com.lingmo.daemon.GraphicsDriver"; }

public:
    ComLingmoDaemonGraphicsDriverInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

    ~ComLingmoDaemonGraphicsDriverInterface();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> CancelInstall()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("CancelInstall"), argumentList);
    }

    inline QDBusPendingReply<QString> GetCurrDriverName()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetCurrDriverName"), argumentList);
    }

    inline QDBusPendingReply<QString> GetDevice()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetDevice"), argumentList);
    }

    inline QDBusPendingReply<QString> GetNewDriverName()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetNewDriverName"), argumentList);
    }

    inline QDBusPendingReply<QString> GetOldDriverName()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetOldDriverName"), argumentList);
    }

    inline QDBusPendingReply<QString> GetResolutionTitle()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("GetResolutionTitle"), argumentList);
    }

    inline QDBusPendingReply<bool> IsTestSuccess()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("IsTestSuccess"), argumentList);
    }

    inline QDBusPendingReply<> PrepareInstall(const QString &name, const QString &language)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(name) << QVariant::fromValue(language);
        return asyncCallWithArgumentList(QStringLiteral("PrepareInstall"), argumentList);
    }

    inline QDBusPendingReply<> RealInstall()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("RealInstall"), argumentList);
    }

    inline QDBusPendingReply<> TestInstall()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("TestInstall"), argumentList);
    }

    inline QDBusPendingReply<> TestSuccess()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("TestSuccess"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void Cancel();
    void ReportProgress(const QString &ratio);
};

namespace com {
  namespace lingmo {
    namespace daemon {
      typedef ::ComLingmoDaemonGraphicsDriverInterface GraphicsDriver;
    }
  }
}
#endif
