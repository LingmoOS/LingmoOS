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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */

#ifndef LINGMO_MENU_CHAT_H
#define LINGMO_MENU_CHAT_H
#include <QObject>

class ChatPrivate;
class Chat : public QObject
{
    Q_OBJECT
public:
    explicit Chat(QObject *parent = nullptr);
    Q_DISABLE_COPY(Chat)
    /**
     * 开启一个新会话
     * @param clientId
     * @param clientSecret
     * @return
     */
    void newChat(const QString &clientId = {}, const QString &clientSecret = {});
    /**
     * 清除对话历史
     * @return
     */
    void clear();
    bool isValid() const;

public Q_SLOTS:
    void talk(const QString& content);

Q_SIGNALS:
    void readyToChat();
    void response(const QString& content);
    void expired();
    void error(const QString &errorMsg);

private:
    ChatPrivate *d = nullptr;
};


#endif //LINGMO_MENU_CHAT_H
