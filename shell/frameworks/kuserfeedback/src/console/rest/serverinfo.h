/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_SERVERINFO_H
#define KUSERFEEDBACK_CONSOLE_SERVERINFO_H

#include <QSharedDataPointer>
#include <QStringList>

class QUrl;

namespace KUserFeedback {
namespace Console {

class ServerInfoData;

/** Connection information for an analytics server. */
class ServerInfo
{
public:
    ServerInfo();
    ServerInfo(const ServerInfo &other);
    ~ServerInfo();
    ServerInfo& operator=(const ServerInfo &other);

    bool isValid() const;

    QString name() const;
    void setName(const QString &name);

    QUrl url() const;
    void setUrl(const QUrl& url);

    QString userName() const;
    void setUserName(const QString& userName);

    QString password() const;
    void setPassword(const QString& password);

    /** Save to settings. */
    void save() const;
    /** Load from settings. */
    static ServerInfo load(const QString &name);
    /** Delete stored information for the server with name @p name. */
    static void remove(const QString &name);

    /** Returns the names of all configured servers. */
    static QStringList allServerInfoNames();

private:
    QSharedDataPointer<ServerInfoData> d;
};

}
}

#endif // KUSERFEEDBACK_CONSOLE_SERVERINFO_H
