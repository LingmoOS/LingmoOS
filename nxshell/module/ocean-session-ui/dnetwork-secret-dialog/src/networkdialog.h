// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETWORKDIALOG_H
#define NETWORKDIALOG_H

#include <QObject>

class QLocalSocket;

class NetworkDialog : public QObject
{
    Q_OBJECT

public:
    explicit NetworkDialog(QObject *parent = Q_NULLPTR);
    ~NetworkDialog();

    bool exec(const QJsonDocument &doc);
    void onSecretsResult(QLocalSocket *socket, const QByteArray &data);
    void onPassword(QLocalSocket *socket, const QByteArray &data);

private Q_SLOTS:
    void connectedHandler();
    void disConnectedHandler();
    bool ConnectToServer();
    void exit(int returnCode);
    void readyReadHandler();

private:
    QString m_key;
    QByteArray m_lastData; // 用于数据拼接
    bool m_needIdentity; // 认证信息是否需要用户名
    QString m_identityContent;
    bool m_quitNow;
    QByteArray m_data;
    QLocalSocket *m_clinet;
};

#endif // NETWORKDIALOG_H
