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

#ifndef ERNIE_BOT_SESSION_MANAGER_H
#define ERNIE_BOT_SESSION_MANAGER_H

#include <QObject>
#include <QMap>
#include <QVariantList>
#include <QPointer>

// TODO: 更优雅的实现？
#include "chat.h"

class QSettings;
class Session;

class SessionManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool apiReady READ apiReady NOTIFY apiReadyChanged)
public:
    explicit SessionManager(QObject *parent = nullptr);
    bool apiReady() const;

public Q_SLOTS:
    int newSession();
    QVariantList sessionIds();
    Session *getSession(int sessionId) const;

    void checkApiReady();
    void exitSessionApi();
    void setSessionKeys(const QString& key, const QString& secret);

Q_SIGNALS:
    void sessionAdded(int sessionId);
    void sessionRemoved(int sessionId);
    void apiReadyChanged();
    void apiError(QString error);

private:
    bool m_apiReady = false;
    QPointer<Chat> m_chat;
    QString m_key;
    QString m_secret;
    QSettings *m_settings {nullptr};
    QMap<int, Session *> m_sessions;
};


#endif //ERNIE_BOT_SESSION_MANAGER_H
