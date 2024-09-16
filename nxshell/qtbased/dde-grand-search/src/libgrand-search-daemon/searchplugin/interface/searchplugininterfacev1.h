// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHPLUGININTERFACEV1_H
#define SEARCHPLUGININTERFACEV1_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

namespace GrandSearch {

class SearchPluginInterfaceV1: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    SearchPluginInterfaceV1(const QString &service, const QString &path, const char *interface,
                            const QDBusConnection &connection, QObject *parent = nullptr);

    ~SearchPluginInterfaceV1();

public slots:
    inline QDBusPendingReply<bool> Action(const QString &json)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(json);
        return asyncCallWithArgumentList(QStringLiteral("Action"), argumentList);
    }

    inline QDBusPendingReply<QString> Search(const QString &json)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(json);
        return asyncCallWithArgumentList(QStringLiteral("Search"), argumentList);
    }

    inline QDBusPendingReply<bool> Stop(const QString &json)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(json);
        return asyncCallWithArgumentList(QStringLiteral("Stop"), argumentList);
    }
};

}

#endif
