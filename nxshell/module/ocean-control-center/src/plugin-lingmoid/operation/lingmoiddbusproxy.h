// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LINGMOIDDBUSPROXY_H
#define LINGMOIDDBUSPROXY_H

#include <DDBusInterface>
#include <QObject>
#include <QDBusReply>

using Dtk::Core::DDBusInterface;

class LingmoidDBusProxy: public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap UserInfo READ userInfo NOTIFY UserInfoChanged)

public:
    explicit LingmoidDBusProxy(QObject *parent = nullptr);

    void login() const;
    void logout() const;

    QDBusReply<QString> localBindCheck(const QString &uuid);
    QVariantMap userInfo();

signals:
    void UserInfoChanged(const QVariantMap& value) const;

private:
    DDBusInterface *m_lingmoId;
};

#endif // LINGMOIDDBUSPROXY_H
