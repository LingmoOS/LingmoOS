/*
 *
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
 *
 */
#ifndef ACTIONTRANSMITER_H
#define ACTIONTRANSMITER_H

#include <QObject>
#include "search-plugin-iface.h"
namespace LingmoUISearch {
/**
 * @brief The ActionTransmiter class
 * 转发搜索插件的action调用信号
 */
class ActionTransmiter : public QObject
{
    Q_OBJECT
public:
    static ActionTransmiter *getInstance();
    Q_INVOKABLE void invokeActions(SearchPluginIface::InvokableActions actions);

Q_SIGNALS:
    void hideUIAction();

private:
    ActionTransmiter(QObject *parent = nullptr);
};

}
#endif // ACTIONTRANSMITER_H
