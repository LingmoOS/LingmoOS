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

#ifndef ERNIE_BOT_MESSAGE_H
#define ERNIE_BOT_MESSAGE_H

#include <QObject>
#include <QList>

#include "message-action.h"

class Message : public QObject
{
    Q_OBJECT
public:
    enum Type {
        TipMessage = 0,
        LoadTipMessage,
        TimeMessage,
        SentMessage,
        ReceivedMessage,
        ErrorMessage
    };
    Q_ENUM(Type)

    enum Key {
        MessageType,
        MessageHeader,
        MessageContent,
        MessageActions
    };
    Q_ENUM(Key)
    static QHash<int, QByteArray> roleNames();

    Message(Type type, QString content, QObject *parent = nullptr);

    const QString &content() const;
    void setContent(const QString &content);

    const QString &header() const;
    void setHeader(const QString &header);

    const Type &type() const;
    void setType(const Type &type);

    const QList<MessageAction *> &actions() const;
    void setAction(const QList<MessageAction *> &actions);

private:
    Type m_type {SentMessage};
    QString m_header;
    QString m_content;
    QList<MessageAction *> m_actions;
};


#endif //ERNIE_BOT_MESSAGE_H
