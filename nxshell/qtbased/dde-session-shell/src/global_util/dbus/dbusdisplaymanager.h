// SPDX-FileCopyrightText: 2015 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSDISPLAYMANAGER_H_1446711552
#define DBUSDISPLAYMANAGER_H_1446711552

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface org.freedesktop.DisplayManager
 */
class DBusDisplayManager: public QDBusAbstractInterface
{
    Q_OBJECT

    Q_SLOT void __propertyChanged__(const QDBusMessage& msg)
    {
        QList<QVariant> arguments = msg.arguments();
        if (3 != arguments.count())
            return;
        QString interfaceName = msg.arguments().at(0).toString();
        if (interfaceName !="org.freedesktop.DisplayManager")
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
    { return "org.freedesktop.DisplayManager"; }

public:
    DBusDisplayManager(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~DBusDisplayManager();

    Q_PROPERTY(QList<QDBusObjectPath> Sessions READ sessions NOTIFY SessionsChanged)
    inline QList<QDBusObjectPath> sessions() const
    { return qvariant_cast< QList<QDBusObjectPath> >(property("Sessions")); }

public Q_SLOTS: // METHODS
Q_SIGNALS: // SIGNALS
// begin property changed signals
void SessionsChanged();
};

namespace org {
  namespace freedesktop {
    typedef ::DBusDisplayManager DisplayManager;
  }
}
#endif
