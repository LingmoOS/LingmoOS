// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WEBSOCKETTRANSPORT_H
#define WEBSOCKETTRANSPORT_H

#include <QWebChannelAbstractTransport>

QT_BEGIN_NAMESPACE
class QWebSocket;
QT_END_NAMESPACE

class WebSocketTransport : public QWebChannelAbstractTransport
{
    Q_OBJECT
public:
    explicit WebSocketTransport(QWebSocket *socket);
    virtual ~WebSocketTransport();

    void sendMessage(const QJsonObject &message) override;

private slots:
    void textMessageReceived(const QString &message);

private:
    QWebSocket *m_socket;
};

#endif // WEBSOCKETTRANSPORT_H
