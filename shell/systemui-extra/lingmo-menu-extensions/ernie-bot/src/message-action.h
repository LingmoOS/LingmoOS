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

#ifndef ERNIE_BOT_MESSAGE_ACTION_H
#define ERNIE_BOT_MESSAGE_ACTION_H

#include <QObject>

class MessageAction : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString text READ text NOTIFY textChanged)
public:
    MessageAction() = delete;
    explicit MessageAction(QObject *parent = nullptr);
    MessageAction(QString text, QString name, QObject *parent = nullptr);

    const QString &text() const;
    void setText(const QString &text);

    const QString &name() const;
    void setName(const QString &name);

public Q_SLOTS:
    void exec();
    bool isValid();

Q_SIGNALS:
    void nameChanged();
    void textChanged();
    void actionExecuted();

private:
    QString m_text;
    QString m_name;
};


#endif //ERNIE_BOT_MESSAGE_ACTION_H
