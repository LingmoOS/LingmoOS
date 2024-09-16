// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WEBSOCKETCLIENTWRAPPER_H
#define WEBSOCKETCLIENTWRAPPER_H

#include <QObject>

class WebSocketTransport;

QT_BEGIN_NAMESPACE
class QWebSocketServer;
QT_END_NAMESPACE

class WebSocketClientWrapper : public QObject
{
    Q_OBJECT

public:
    WebSocketClientWrapper(QWebSocketServer *server, QObject *parent = nullptr);

signals:
    void clientConnected(WebSocketTransport *client);

private slots:
    void handleNewConnection();

private:
    QWebSocketServer *m_server;
};

#endif // WEBSOCKETCLIENTWRAPPER_H
