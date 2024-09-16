// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once
#include "dlogintypes_p.h"

#include <ddbusinterface.h>
#include <qdbuspendingreply.h>

using DTK_CORE_NAMESPACE::DDBusInterface;
DLOGIN_BEGIN_NAMESPACE

class Login1UserInterface : public QObject
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName() { return "org.freedesktop.login1.User"; }

    Login1UserInterface(const QString &service,
                        const QString &path,
                        const QDBusConnection &connection,
                        QObject *parent = nullptr);
    ~Login1UserInterface() override;

    Q_PROPERTY(QList<DBusSessionPath> sessions READ sessions);
    Q_PROPERTY(bool idleHint READ idleHint);
    Q_PROPERTY(bool linger READ linger);
    Q_PROPERTY(QString name READ name);
    Q_PROPERTY(QString runtimePath READ runtimePath);
    Q_PROPERTY(QString service READ service);
    Q_PROPERTY(QString slice READ slice);
    Q_PROPERTY(QString state READ state);
    Q_PROPERTY(DBusSessionPath display READ display);
    Q_PROPERTY(quint32 GID READ GID);
    Q_PROPERTY(quint32 UID READ UID);
    Q_PROPERTY(quint64 idleSinceHint READ idleSinceHint);
    Q_PROPERTY(quint64 idleSinceHintMonotonic READ idleSinceHintMonotonic);
    Q_PROPERTY(quint64 timestamp READ timestamp);
    Q_PROPERTY(quint64 timestampMonotonic READ timestampMonotonic);

    QList<DBusSessionPath> sessions() const;
    bool idleHint() const;
    bool linger() const;
    QString name() const;
    QString runtimePath() const;
    QString service() const;
    QString slice() const;
    QString state() const;
    DBusSessionPath display() const;
    quint32 GID() const;
    quint32 UID() const;
    quint64 idleSinceHint() const;
    quint64 idleSinceHintMonotonic() const;
    quint64 timestamp() const;
    quint64 timestampMonotonic() const;
public slots:
    QDBusPendingReply<> kill(int signalNumber);
    QDBusPendingReply<> terminate();

private:
    DDBusInterface *m_interface;
    QString m_path;
};

DLOGIN_END_NAMESPACE
