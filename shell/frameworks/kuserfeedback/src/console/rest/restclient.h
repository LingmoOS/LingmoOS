/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_RESTCLIENT_H
#define KUSERFEEDBACK_CONSOLE_RESTCLIENT_H

#include "serverinfo.h"

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;

namespace KUserFeedback {
namespace Console {

/** REST communication handling. */
class RESTClient : public QObject
{
    Q_OBJECT
public:
    explicit RESTClient(QObject *parent = nullptr);
    ~RESTClient() override;

    ServerInfo serverInfo() const;
    void setServerInfo(const ServerInfo &info);
    void setConnected(bool connected);
    bool isConnected() const;

    QNetworkReply* get(const QString &command);
    QNetworkReply* post(const QString &command, const QByteArray &data);
    QNetworkReply* put(const QString &command, const QByteArray &data);
    QNetworkReply* deleteResource(const QString &command);

    QNetworkAccessManager* networkAccessManager() const;
Q_SIGNALS:
    void clientConnected();
    void errorMessage(const QString &msg);

private:
    QNetworkRequest makeRequest(const QString &command);
    void setupMessageHandler(QNetworkReply *reply);

    ServerInfo m_serverInfo;
    QNetworkAccessManager *m_networkAccessManager;
    bool m_isConnected = false;
};

}
}

#endif // KUSERFEEDBACK_CONSOLE_RESTCLIENT_H
