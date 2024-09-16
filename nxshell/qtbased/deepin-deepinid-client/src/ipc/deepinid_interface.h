// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DEEPINID_INTERFACE_H
#define DEEPINID_INTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

typedef QVariantMap UserInfo;

Q_DECLARE_METATYPE(UserInfo)

/*
 * Proxy class for interface com.deepin.deepinid
 */
class DeepinIDInterface: public QDBusAbstractInterface
{
Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "com.deepin.deepinid"; }

public:
    DeepinIDInterface(const QString &service,
                      const QString &path,
                      const QDBusConnection &connection,
                      QObject *parent = 0);

    ~DeepinIDInterface();

    Q_PROPERTY(QString HardwareID
                   READ
                   hardwareID)
    inline QString hardwareID() const
    { return qvariant_cast<QString>(property("HardwareID")); }

    Q_PROPERTY(QString MachineName
                   READ
                   machineName)
    inline QString machineName() const
    { return qvariant_cast<QString>(property("MachineName")); }

    Q_PROPERTY(UserInfo UserInfo
                   READ
                   userInfo)
    inline UserInfo userInfo() const
    { return qvariant_cast<UserInfo>(property("UserInfo")); }

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> ConfirmPrivacy(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("ConfirmPrivacy"), argumentList);
    }

    inline QDBusPendingReply<bool> HasConfirmPrivacy(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("HasConfirmPrivacy"), argumentList);
    }

    inline QDBusPendingReply<> Login()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("Login"), argumentList);
    }

    inline QDBusPendingReply<> Logout()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("Logout"), argumentList);
    }

    inline QDBusPendingReply<QByteArray> Get()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("Get"), argumentList);
    }

    inline QDBusPendingReply<> Set(const QVariantMap &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("Set"), argumentList);
    }

Q_SIGNALS: // SIGNALS
};

#endif
