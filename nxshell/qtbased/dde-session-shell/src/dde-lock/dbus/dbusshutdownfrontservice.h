// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSSHUTDOWNFRONTSERVICE_H
#define DBUSSHUTDOWNFRONTSERVICE_H

#include "dbusshutdownagent.h"
#include <QDBusAbstractAdaptor>
#include <QtCore/QObject>
#include <QtDBus/QtDBus>

QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Proxy class for interface org.deepin.dde.ShutdownFront1
 */

class DBusShutdownFrontService : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.dde.ShutdownFront1")

public:
    explicit DBusShutdownFrontService(DBusShutdownAgent *parent);
    virtual ~DBusShutdownFrontService();

    inline DBusShutdownAgent *parent() const
    { return static_cast<DBusShutdownAgent *>(QObject::parent()); }

public Q_SLOTS:
    void Show();
    void Shutdown();
    void Restart();
    void Logout();
    void Suspend();
    void Hibernate();
    void SwitchUser();
    void Lock();

Q_SIGNALS:
    void ChangKey(QString key);
};

#endif // DBUSSHUTDOWNFRONTSERVICE_H
