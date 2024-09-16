// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSSCREENSHOT_H
#define DBUSSCREENSHOT_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface com.deepin.DeepinScreenshot
 */
class DBusScreenshot: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "com.deepin.DeepinScreenshot"; }

public:
    DBusScreenshot(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = nullptr);

    ~DBusScreenshot();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> DelayScreenshot(qlonglong in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("DelayScreenshot"), argumentList);
    }

    inline QDBusPendingReply<> FullscreenScreenshot()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("FullscreenScreenshot"), argumentList);
    }

    inline QDBusPendingReply<> NoNotifyScreenshot()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("NoNotifyScreenshot"), argumentList);
    }

    inline QDBusPendingReply<> SavePathScreenshot(const QString &in0)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(in0);
        return asyncCallWithArgumentList(QStringLiteral("SavePathScreenshot"), argumentList);
    }

    inline QDBusPendingReply<> StartScreenshot()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("StartScreenshot"), argumentList);
    }

    inline QDBusPendingReply<> TopWindowScreenshot()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("TopWindowScreenshot"), argumentList);
    }

Q_SIGNALS: // SIGNALS
};

namespace com {
  namespace deepin {
    typedef ::DBusScreenshot DeepinScreenshot;
  }
}
#endif
