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

#include "session.h"
#include "chat.h"

class SessionPrivate
{
    friend class Session;
public:
    explicit SessionPrivate(Session *parent = nullptr) : chat(new Chat(parent)) {}

private:
    Chat *chat {nullptr};
};


Session::Session(int sessionId, const QString& key, const QString& secret, QObject *parent)
    : QAbstractListModel(parent), d(new SessionPrivate(this)), m_sessionId(sessionId), m_key(key), m_secret(secret)
{
    connect(d->chat, &Chat::readyToChat, this, &Session::onReadyToChat);
    connect(d->chat, &Chat::response, this, &Session::onResponse);
    connect(d->chat, &Chat::error, this, &Session::onError);
    connect(d->chat, &Chat::expired, this, [this] {
        setSessionIsReady(false);
        openChatSession();
    });

    // append tip message
    auto message = new Message(Message::TipMessage, tr("You can try asking me:"), this);

    int hour = QTime::currentTime().hour();
    QString hello;
    if (hour < 0) {
        hello = tr("How are you today!");
    }
    else if (hour < 13) {
        hello = tr("Good morning!");
    }
    else {
        hello = tr("Good afternoon!");
    }

    message->setHeader(tr("Hi, %1").arg(hello));

    auto action1 = new MessageAction(tr("What can AI do?"), tr("Give it a try"), message);
    auto action2 = new MessageAction(tr("Help me write a product requirements document."), tr("Give it a try"), message);
    auto action3 = new MessageAction(tr("What gift to give on Mother's Day is more intentional?"), tr("Give it a try"), message);

    message->setAction({action1, action2, action3});

    connect(action1, &MessageAction::actionExecuted, this, [this, action1] {
        talk(action1->text());
    });
    connect(action2, &MessageAction::actionExecuted, this, [this, action2] {
        talk(action2->text());
    });
    connect(action3, &MessageAction::actionExecuted, this, [this, action3] {
        talk(action3->text());
    });

    appendMessage(message);
}

int Session::rowCount(const QModelIndex &parent) const
{
    return m_messages.size();
}

QVariant Session::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return {};
    }

    int messageIndex = index.row();
    switch (role) {
        case Message::MessageType:
            return m_messages.at(messageIndex)->type();
        case Message::MessageContent:
            return m_messages.at(messageIndex)->content();
        case Message::MessageHeader:
            return m_messages.at(messageIndex)->header();
        case Message::MessageActions: {
            QVariantList list;
            QListIterator<MessageAction *> iterator(m_messages.at(messageIndex)->actions());
            while (iterator.hasNext()) {
                list << QVariant::fromValue(iterator.next());
            }

            return list;
        }
        default:
            break;
    }

    return {};
}

QHash<int, QByteArray> Session::roleNames() const
{
    return Message::roleNames();
}

int Session::sessionId() const
{
    return m_sessionId;
}

bool Session::sessionIsReady() const
{
    return m_sessionIsReady;
}

bool Session::responseIsReady() const
{
    return m_responseIsReady;
}

QString Session::sessionName() const
{
    return tr("Session: %1").arg(m_sessionId);
}

const QDateTime &Session::lastTalkTime() const
{
    return m_lastTalkTime;
}

void Session::openChatSession()
{
    if (d->chat && !m_sessionIsReady) {
        d->chat->newChat(m_key, m_secret);
    }
}

void Session::talk(const QString &message)
{
    if (message.isEmpty() || !m_sessionIsReady || !m_responseIsReady || !d->chat) {
        return;
    }

    // 过滤特殊字符
    QString msg(message);
    msg.replace("<", "&lt;");
    msg.replace(">", "&gt;");

    updateTalkTime();
    setResponseIsReady(false);

    appendMessage(new Message(Message::SentMessage, msg, this));
    appendMessage(new Message(Message::LoadTipMessage, "", this));
    d->chat->talk(msg);
}

void Session::appendMessage(Message *message)
{
    beginInsertRows(QModelIndex(), m_messages.size(), m_messages.size());
    m_messages.append(message);
    endInsertRows();
}

void Session::exportSession()
{
    // TODO: 导出全部消息到文件
}

void Session::clearSession()
{
    if (!m_sessionIsReady || !m_responseIsReady || !d->chat) {
        return;
    }

    int rows = m_messages.size() - 1;
    beginRemoveRows(QModelIndex(), 1, rows);

    m_lastTalkTime = QDateTime();
    for (int i = rows; i > 0; --i) {
        delete m_messages.takeAt(i);
    }

    d->chat->clear();
    endRemoveRows();
}

void Session::exitSession()
{
    setSessionIsReady(false);
    setResponseIsReady(false);
}

void Session::onReadyToChat()
{
    setSessionIsReady(true);
}

void Session::onResponse(const QString &content)
{
    setResponseIsReady(true);

    if (!m_messages.isEmpty() && m_messages.last()->type() == Message::LoadTipMessage) {
        m_messages.last()->setType(Message::ReceivedMessage);
        m_messages.last()->setContent(content);

        QModelIndex index = Session::index(m_messages.size() - 1);
        dataChanged(index, index);

    } else {
        appendMessage(new Message(Message::ReceivedMessage, content, this));
    }
}

void Session::onError(const QString &error)
{
    if (!m_messages.isEmpty() && m_messages.last()->type() == Message::LoadTipMessage) {
        beginRemoveRows(QModelIndex(), m_messages.size(), m_messages.size());
        delete m_messages.takeLast();
        endRemoveRows();
    }

    auto message = new Message(Message::ErrorMessage, "Error: " + error, this);
    if (!m_sessionIsReady) {
        message->setContent(tr("The current network is not good, please check the network.\n\nError: ") + error);
        QList<MessageAction *> actions;
        auto action = new MessageAction("retry", tr("Retry"), message);
        actions << action;

        connect(actions.last(), &MessageAction::actionExecuted, this, [this, action] {
            openChatSession();
        });

        message->setAction(actions);
    }

    appendMessage(message);
    setResponseIsReady(true);
}

void Session::setSessionIsReady(bool isReady)
{
    m_sessionIsReady = isReady;
    Q_EMIT sessionIsReadyChanged();
}

void Session::setResponseIsReady(bool isReady)
{
    m_responseIsReady = isReady;
    Q_EMIT responseIsReadyChanged();
}

void Session::updateTalkTime()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    if (m_lastTalkTime.isValid() && (m_lastTalkTime.secsTo(currentTime) < 5 * 60)) {
        return;
    }

    m_lastTalkTime = currentTime;
    appendMessage(new Message(Message::TimeMessage, m_lastTalkTime.toString("hh:mm"), this));
}

Session::~Session()
{
    exitSession();
    if (d) {
        delete d;
        d = nullptr;
    }
}
