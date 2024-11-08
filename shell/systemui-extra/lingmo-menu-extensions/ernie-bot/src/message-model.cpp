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

#include "message-model.h"
#include "session.h"

#include <QDebug>
#include <QHash>

MessageModel::MessageModel(QObject *parent) : QSortFilterProxyModel(parent)
{

}

void MessageModel::talk(const QString &message)
{
    if (!m_session) {
        return;
    }

    m_session->talk(message);
}

void MessageModel::setSession(Session *session)
{
    if (session == m_session) {
        return;
    }

    beginResetModel();

    m_session = session;
    setSourceModel(session);

    endResetModel();

    Q_EMIT currentSessionIdChanged();
}

void MessageModel::removeSession(int sessionId)
{
    if (m_session && sessionId == m_session->sessionId()) {
        setSession(nullptr);
    }
}

int MessageModel::currentSessionId() const
{
    if (m_session) {
        return m_session->sessionId();
    }

    return -1;
}
