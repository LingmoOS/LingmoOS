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
 *
 */

#include "session-manager.h"
#include "session.h"

#include <QDebug>
#include <QMutableMapIterator>
#include <QSettings>
#include <QDir>
#include <qqml.h>

SessionManager::SessionManager(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<Session*>("Session*");
    qRegisterMetaType<MessageAction *>("MessageAction*");
    qmlRegisterUncreatableType<Message>("org.lingmo.menu.aiAssistant", 1, 0, "Message", "Enum");

    QString path(QDir::homePath() + "/.config/lingmo-menu/ai-assistant/");
    if (!QDir(path).exists()) {
        QDir().mkpath(path);
    }

    path += "ai-assistant.conf";
    if (!QFile::exists(path)) {
        QFile file(path);
        file.open(QFile::WriteOnly);
        file.close();
    }

    QSettings *settings = new QSettings(path, QSettings::IniFormat);
    settings->sync();

    settings->beginGroup("Api Setting");
    QStringList keys = settings->allKeys();

    if (settings->contains("key")) {
        m_key = settings->value("key").toString();
    }

    if (settings->contains("secret")) {
        m_secret = settings->value("secret").toString();
    }

    settings->endGroup();
    m_settings = settings;
}

Session *SessionManager::getSession(int sessionId) const
{
    return m_sessions.value(sessionId, nullptr);
}

int SessionManager::newSession()
{
    if (!m_apiReady) {
        return -1;
    }

    int sessionId = m_sessions.isEmpty() ? 0 : m_sessions.lastKey() + 1;

    auto session = new Session(sessionId, m_key, m_secret, this);
    m_sessions.insert(sessionId, session);

    session->openChatSession();

    Q_EMIT sessionAdded(sessionId);
    return sessionId;
}

QVariantList SessionManager::sessionIds()
{
    QVariantList list;

    QMutableMapIterator<int, Session *> iterator(m_sessions);
    while (iterator.hasNext()) {
        iterator.next();
        list << iterator.key();
    }

    return list;
}

void SessionManager::setSessionKeys(const QString& key, const QString& secret)
{
    if (m_apiReady || key.isEmpty() || secret.isEmpty()) {
        return;
    }

    m_key = key;
    m_secret = secret;

    m_settings->beginGroup("Api Setting");
    m_settings->setValue("key", m_key);
    m_settings->setValue("secret", m_secret);
    m_settings->endGroup();

    checkApiReady();
}

bool SessionManager::apiReady() const
{
    return m_apiReady;
}

void SessionManager::checkApiReady()
{
    if (m_key.isEmpty() || m_secret.isEmpty() || m_chat || m_apiReady) {
        return;
    }

    Chat *chat = new Chat(this);
    connect(chat, &Chat::readyToChat, this, [this, chat] {
        chat->deleteLater();
        m_apiReady = true;
        Q_EMIT apiReadyChanged();
    });

    connect(chat, &Chat::error, this, [this, chat] (const QString &errorMsg) {
        chat->deleteLater();
        Q_EMIT apiError(errorMsg);
    });

    m_chat = chat;
    chat->newChat(m_key, m_secret);
}

void SessionManager::exitSessionApi()
{
    QList<int> keys = m_sessions.keys();
    for (const auto &id : keys) {
        Session *session = m_sessions.take(id);
        if (session) {
            Q_EMIT sessionRemoved(id);
            delete session;
        }
    }

    m_key = "";
    m_secret = "";
    m_settings->beginGroup("Api Setting");
    m_settings->setValue("key", "");
    m_settings->setValue("secret", "");
    m_settings->endGroup();

    m_apiReady = false;
    Q_EMIT apiReadyChanged();
}
