// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETSECRETAGENTFORUI_H
#define NETSECRETAGENTFORUI_H

#include "netsecretagentinterface.h"

#include <QObject>

class QWidget;
class QLocalServer;
class QLocalSocket;

namespace dde {
namespace network {

class NetSecretAgentForUI : public QObject, public NetSecretAgentInterface
{
    Q_OBJECT
public:
    explicit NetSecretAgentForUI(PasswordCallbackFunc fun, const QString &serverKey, QObject *parent = Q_NULLPTR);
    ~NetSecretAgentForUI() override;

    bool hasTask() override;
    void inputPassword(const QString &key, const QVariantMap &password, bool input) override;

Q_SIGNALS:
    void requestShow();

public:
    void setServerName(const QString &name);

private Q_SLOTS:
    void newConnectionHandler();
    void readyReadHandler();
    void disconnectedHandler();
    void sendSecretsResult(const QString &key, const QVariantMap &password, bool input);

public:
    void requestSecrets(QLocalSocket *socket, const QByteArray &data);

private:
    QString m_connectDev;
    QString m_connectSsid;
    QStringList m_secrets;
    QByteArray m_lastData;

    QLocalServer *m_server;
    QList<QLocalSocket *> m_clients;
    QString m_serverName;
};

} // namespace network
} // namespace dde
#endif // NETSECRETAGENTFORUI_H
