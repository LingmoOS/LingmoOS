// SPDX-FileCopyrightText: 2011-2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUS_INTERFACE_H
#define DBUS_INTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

class SaveFileInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName() {
        return "com.deepin.editor.daemon";
    }

public:
    SaveFileInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);
    ~SaveFileInterface();

public Q_SLOTS: // METHODS
//    inline QDBusPendingReply<bool> saveFile(const QByteArray &filepath, const QByteArray &text, const QByteArray &encoding) {
//        QList<QVariant> argumentList;
//        argumentList << QVariant::fromValue(filepath) << QVariant::fromValue(text) << QVariant::fromValue(encoding);
//        return asyncCallWithArgumentList(QStringLiteral("saveFile"), argumentList);
//    }

Q_SIGNALS: // SIGNALS
};

namespace DBusDaemon {
    typedef ::SaveFileInterface dbus;
}
#endif
