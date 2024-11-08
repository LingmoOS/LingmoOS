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
 *
 */

#ifndef LINGMO_QUICK_WINDOW_TYPE_H
#define LINGMO_QUICK_WINDOW_TYPE_H

#include <QObject>

namespace LingmoUIQuick {

/**
 * usage:
 * windowType: WindowType.Tooltip
 */
class WindowType
{
    Q_GADGET
public:
    // copy from [KWindowSystem: netwm_def.h, KWayland: plasmashell.h]
    // 仅保留常用的层级
    enum Type {
        Normal = 0,
        Desktop,
        Dock,
        Panel,
        SystemWindow,
        Notification,
        CriticalNotification,
        ScreenLockNotification,
        OnScreenDisplay,
        Dialog,
        ToolTip,
        Menu,
        PopupMenu,
        AppletPopup,
        Switcher
    };
    Q_ENUM(Type)
};

}

#endif //LINGMO_QUICK_WINDOW_TYPE_H
