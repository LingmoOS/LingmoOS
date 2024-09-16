// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <DDBusInterface>

#include <QDBusMetaType>
#include <QObject>
#include <QtCore/QString>
#include <QtDBus/QtDBus>

struct IntString
{
    qint32 state;
    QString description;

    bool operator!=(const IntString &intString);
};

Q_DECLARE_METATYPE(IntString)

QDBusArgument &operator<<(QDBusArgument &argument, const IntString &intString);
const QDBusArgument &operator>>(const QDBusArgument &argument, IntString &intString);

void registerIntStringMetaType();

using Dtk::Core::DDBusInterface;

class SyncDaemon : public QObject
{
    Q_OBJECT
public:
    explicit SyncDaemon(QObject *parent = nullptr);

    void SwitcherSet(const QString &arg_0, bool state);

    bool SwitcherGet(const QString &arg_0);

    QDBusPendingCall SwitcherDump();

    void setDBusBlockSignals(bool status);

    Q_PROPERTY(qlonglong LastSyncTime READ lastSyncTime NOTIFY LastSyncTimeChanged)
    qlonglong lastSyncTime();

    Q_PROPERTY(IntString State READ state NOTIFY StateChanged)
    IntString state();

signals:
    void SwitcherChange(QString, bool);
    void LastSyncTimeChanged(qlonglong);
    void StateChanged(IntString);
private:
    DDBusInterface *m_syncInner;
};
