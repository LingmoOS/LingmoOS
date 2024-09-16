// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AMDBUSLAUNCHERFRAME_H_1458106952
#define AMDBUSLAUNCHERFRAME_H_1458106952

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtDBus>

#define INTERFACE_NAME "org.deepin.dde.Launcher1"
#define SERVICE_PATH "/org/deepin/dde/Launcher1"

/*
 * Proxy class for interface org.deepin.dde.Launcher1
 */
class AMDBusLauncherFrame: public QDBusAbstractInterface
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
        foreach(const QString &prop, changedProps.keys()) {
        const QMetaObject* self = metaObject();
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
    { return INTERFACE_NAME; }

public:
    explicit AMDBusLauncherFrame(QObject *parent = Q_NULLPTR);

    virtual ~AMDBusLauncherFrame();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> Exit()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("Exit"), argumentList);
    }

    inline QDBusPendingReply<> Hide()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("Hide"), argumentList);
    }

    inline QDBusPendingReply<> Show()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("Show"), argumentList);
    }

    inline QDBusPendingReply<> ShowByMode(qlonglong in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("ShowByMode"), argumentList);
    }

    inline QDBusPendingReply<> Toggle()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("Toggle"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void Closed();
    void Shown();
// begin property changed signals
};

#endif
