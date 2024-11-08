/*
 *
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */
#ifndef LINGMO_PANEL_GENERALCONFIGDEFINE_H
#define LINGMO_PANEL_GENERALCONFIGDEFINE_H
#include <QObject>
class GeneralConfigDefine
{
    Q_GADGET
public:
    //task manager
    enum MergeIcons {
        Always = 0,
        Never
    };
    //panel
    enum PanelLocation {
        Bottom = 0,
        Left,
        Top,
        Right
    };
    enum PanelSizePolicy {
        Small = 0,
        Medium,
        Large,
        Custom
    };

    /**
     * 多屏时，任务栏图标显示在哪个任务栏 枚举值
     * 0-所有屏幕，1-主屏和打开了窗口的任务栏 2-只显示在打开了窗口的任务栏
     */
    enum TaskBarIconsShowedOn {
        AllPanels = 0,
        PrimaryScreenPanelAndPanelWhereWindowIsOpen,
        PanelWhereWindowIsOpen
    };

    Q_ENUM(MergeIcons)
    Q_ENUM(PanelLocation)
    Q_ENUM(PanelSizePolicy)
    Q_ENUM(TaskBarIconsShowedOn)

};
#endif //LINGMO_PANEL_GENERALCONFIGDEFINE_H
