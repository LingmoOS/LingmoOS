// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface com.deepin.StartManager
 */
class DBusStartManager: public QDBusAbstractInterface
{
    Q_OBJECT

    Q_SLOT void __propertyChanged__(const QDBusMessage &msg)
    {
        QList<QVariant> arguments = msg.arguments();
        if (3 != arguments.count())
            return;
        QString interfaceName = msg.arguments().at(0).toString();
        if (interfaceName != "com.deepin.StartManager")
            return;
        QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
        auto keys = changedProps.keys();
        foreach (const QString &prop, keys) {
            const QMetaObject *self = metaObject();
            for (int i = self->propertyOffset(); i < self->propertyCount(); ++i) {
                QMetaProperty p = self->property(i);
                if (p.name() == prop) {
                    Q_EMIT p.notifySignal().invoke(this);
                }
            }
        }
    }
public:
    static inline const char *staticInterfaceName()
    { return "com.deepin.StartManager"; }

public:
    explicit DBusStartManager(QObject *parent = 0);

    ~DBusStartManager();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<bool> AddAutostart(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("AddAutostart"), argumentList);
    }

    inline QDBusPendingReply<QStringList> AutostartList()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("AutostartList"), argumentList);
    }

    inline QDBusPendingReply<bool> IsAutostart(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("IsAutostart"), argumentList);
    }

    inline QDBusPendingReply<bool> Launch(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("Launch"), argumentList);
    }

    inline QDBusPendingReply<bool> LaunchWithTimestamp(const QString &in0, uint in1)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0) << QVariant::fromValue(in1);
        return asyncCallWithArgumentList(QStringLiteral("LaunchWithTimestamp"), argumentList);
    }

    inline QDBusPendingReply<bool> RemoveAutostart(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("RemoveAutostart"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void AutostartChanged(const QString &in0, const QString &in1);
// begin property changed signals
};

namespace com {
namespace deepin {
typedef ::DBusStartManager StartManager;
}
}
