// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSLOCKFRONTSERVICE_H
#define DBUSLOCKFRONTSERVICE_H

#include "dbuslockagent.h"
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
 * Proxy class for interface org.deepin.dde.LockFront1
 */
class DBusLockFrontService: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.dde.LockFront1")

public:
    explicit DBusLockFrontService(DBusLockAgent *parent);
    virtual ~DBusLockFrontService();

    inline DBusLockAgent *parent() const
    { return static_cast<DBusLockAgent *>(QObject::parent()); }

public:// PROPERTIES
public Q_SLOTS:// METHODS
    void Show();
    void ShowUserList();
    void ShowAuth(bool active);
    void Suspend(bool enable);
    void Hibernate(bool enable);

Q_SIGNALS:
    void ChangKey(QString key);
    void Visible(bool visible);
};

#endif // DBUSLOCKFRONTSERVICE_H
