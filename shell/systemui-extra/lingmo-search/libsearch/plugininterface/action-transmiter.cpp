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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#include "action-transmiter.h"
#include <mutex>
using namespace LingmoUISearch;

static std::once_flag flag;
static ActionTransmiter *global_intance = nullptr;
ActionTransmiter::ActionTransmiter(QObject *parent) : QObject(parent)
{
}

ActionTransmiter *ActionTransmiter::getInstance()
{
    std::call_once(flag, [ & ] {
        global_intance = new ActionTransmiter();
    });
    return global_intance;
}

void ActionTransmiter::invokeActions(SearchPluginIface::InvokableActions actions)
{
    if(SearchPluginIface::InvokableAction::HideUI == actions) {
        Q_EMIT hideUIAction();
    }
}
