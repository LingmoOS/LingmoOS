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

#ifndef ERNIE_BOT_MESSAGE_MODEL_H
#define ERNIE_BOT_MESSAGE_MODEL_H

#include <QSortFilterProxyModel>
#include <QVector>

class Session;

class MessageModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(int currentSessionId READ currentSessionId NOTIFY currentSessionIdChanged)
public:
    explicit MessageModel(QObject *parent = nullptr);

    void removeSession(int sessionId);
    int currentSessionId() const;

Q_SIGNALS:
    void currentSessionIdChanged();

public Q_SLOTS:
    void setSession(Session *session);
    void talk(const QString &message);

private:
    Session *m_session {nullptr};
};

#endif //ERNIE_BOT_MESSAGE_MODEL_H
