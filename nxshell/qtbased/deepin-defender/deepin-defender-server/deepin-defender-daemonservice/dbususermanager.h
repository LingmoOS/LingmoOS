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

class DbususerManager:public QDBusAbstractInterface
{
    Q_OBJECT
    Q_SLOT void __propertyChanged__(const QDBusMessage& msg)
        {
            QList<QVariant> arguments = msg.arguments();
            if (3 != arguments.count())
                return;
            QString interfaceName = msg.arguments().at(0).toString();
            if (interfaceName !="com.deepin.daemon.Accounts.User")
                return;
            QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
            foreach(const QString &prop, changedProps.keys()) {
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
    { return "com.deepin.daemon.Accounts.User"; }

public:
    DbususerManager(QString path,QObject *parent = nullptr);
    ~DbususerManager();

    Q_PROPERTY(int PasswordLastChange READ passwordLastChange NOTIFY PasswordLastChangeChanged)
    inline int passwordLastChange() const
    { return qvariant_cast< int >(property("PasswordLastChange")); }

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<bool> IsPasswordExpired ()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("IsPasswordExpired"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void PasswordLastChangeChanged();
};

namespace com {
namespace deepin {
typedef ::DbususerManager UserManager;
}
}
