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

#include "message-action.h"

#include <utility>

MessageAction::MessageAction(QObject *parent) : QObject(parent)
{

}

MessageAction::MessageAction(QString text, QString name, QObject *parent) : QObject(parent), m_text(std::move(text)), m_name(std::move(name))
{

}

void MessageAction::exec()
{
    Q_EMIT actionExecuted();
}

bool MessageAction::isValid()
{
    return m_text.isEmpty() && m_name.isEmpty();
}

const QString &MessageAction::text() const
{
    return m_text;
}

void MessageAction::setText(const QString &text)
{
    m_text = text;
}

const QString &MessageAction::name() const
{
    return m_name;
}

void MessageAction::setName(const QString &name)
{
    m_name = name;
}
