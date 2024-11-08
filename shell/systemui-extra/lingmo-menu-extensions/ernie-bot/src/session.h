/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 */

#ifndef LINGMO_MENU_EXTENSIONS_SESSION_H
#define LINGMO_MENU_EXTENSIONS_SESSION_H

#include <QVector>
#include <QDateTime>
#include <QAbstractListModel>

#include "message.h"
class SessionPrivate;

class Session : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int sessionId READ sessionId)
    Q_PROPERTY(bool sessionIsReady READ sessionIsReady NOTIFY sessionIsReadyChanged)
    Q_PROPERTY(bool responseIsReady READ responseIsReady NOTIFY responseIsReadyChanged)
    Q_PROPERTY(QString sessionName READ sessionName NOTIFY sessionNameChanged)
    Q_PROPERTY(QDateTime lastTalkTime READ lastTalkTime NOTIFY lastTalkTimeChanged)

public:
    explicit Session(int sessionId, const QString& key, const QString& secret, QObject *parent = nullptr);
    ~Session() override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int sessionId() const;
    bool sessionIsReady() const;
    bool responseIsReady() const;
    QString sessionName() const;
    const QDateTime &lastTalkTime() const;

    void openChatSession();

Q_SIGNALS:
    void sessionIsReadyChanged();
    void responseIsReadyChanged();
    void lastTalkTimeChanged();
    void sessionNameChanged();

public Q_SLOTS:
    void talk(const QString &message);
    void appendMessage(Message *message);

    void exportSession();
    void clearSession();
    void exitSession();

private Q_SLOTS:
    void onReadyToChat();
    void onResponse(const QString& content);
    void onError(const QString& error);

private:
    void setSessionIsReady(bool isReady);
    void setResponseIsReady(bool isReady);
    void updateTalkTime();

private:
    bool m_sessionIsReady {false};
    bool m_responseIsReady {true};

    int  m_sessionId {-1};

    QString m_key;
    QString m_secret;

    SessionPrivate *d;
    QVector<Message *> m_messages;
    QDateTime m_lastTalkTime;
};


#endif //LINGMO_MENU_EXTENSIONS_SESSION_H
