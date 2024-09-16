// SPDX-FileCopyrightText: 2015 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSHOTZONE_H
#define DBUSHOTZONE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface org.deepin.dde.Zone1
 */
class DBusHotzone: public QDBusAbstractInterface
{
    Q_OBJECT
    Q_SLOT void __propertyChanged__(const QDBusMessage& msg)
    {
        QList<QVariant> arguments = msg.arguments();
        if (3 != arguments.count())
            return;
        QString interfaceName = msg.arguments().at(0).toString();
        if (interfaceName != staticInterfaceName())
            return;
        QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
        QStringList keys = changedProps.keys();
        foreach(const QString &prop, keys) {
        const QMetaObject* self = metaObject();
            for (int i=self->propertyOffset(); i < self->propertyCount(); ++i) {
                QMetaProperty p = self->property(i);
                if (p.name() == prop) {
                Q_EMIT p.notifySignal().invoke(this);
                }
            }
        }
   }
public:
    static inline const char *staticInterfaceName()
    { return "org.deepin.dde.Zone1"; }

public:
    DBusHotzone(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~DBusHotzone();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<QString> BottomLeftAction()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("BottomLeftAction"), argumentList);
    }

    inline QDBusPendingReply<QString> BottomRightAction()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("BottomRightAction"), argumentList);
    }

    inline QDBusPendingReply<> EnableZoneDetected(bool in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("EnableZoneDetected"), argumentList);
    }

    inline QDBusPendingReply<> SetBottomLeft(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("SetBottomLeft"), argumentList);
    }

    inline QDBusPendingReply<> SetBottomRight(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("SetBottomRight"), argumentList);
    }

    inline QDBusPendingReply<> SetTopLeft(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("SetTopLeft"), argumentList);
    }

    inline QDBusPendingReply<> SetTopRight(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("SetTopRight"), argumentList);
    }

    inline QDBusPendingReply<QString> TopLeftAction()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("TopLeftAction"), argumentList);
    }

    inline QDBusPendingReply<QString> TopRightAction()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("TopRightAction"), argumentList);
    }

Q_SIGNALS: // SIGNALS
};

namespace com {
  namespace deepin {
    namespace dde {
      typedef ::DBusHotzone Zone1;
    }
  }
}
#endif
