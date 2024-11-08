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

#include "message.h"

#include <QHash>
#include <utility>

Message::Message(Message::Type type, QString content, QObject *parent) : QObject(parent), m_type(type), m_content(std::move(content))
{

}

const QString &Message::content() const
{
    return m_content;
}

void Message::setContent(const QString &content)
{
    m_content = content;
}

const Message::Type &Message::type() const
{
    return m_type;
}

void Message::setType(const Message::Type &type)
{
    m_type = type;
}

const QList<MessageAction *> &Message::actions() const
{
    return m_actions;
}

void Message::setAction(const QList<MessageAction *> &actions)
{
    m_actions = actions;
}

const QString &Message::header() const
{
    return m_header;
}

void Message::setHeader(const QString &header)
{
    m_header = header;
}

QHash<int, QByteArray> Message::roleNames()
{
    QHash<int, QByteArray> hash;
    hash.insert(MessageType, "type");
    hash.insert(MessageHeader, "header");
    hash.insert(MessageContent, "content");
    hash.insert(MessageActions, "actions");
    return hash;
}
