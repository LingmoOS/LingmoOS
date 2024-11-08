/*
 * liblingmosdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#ifndef PARMSCONTROLLER_H
#define PARMSCONTROLLER_H

#include <QObject>

class QDBusConnection;

namespace kdk
{
/**
 * @brief The Parmscontroller class
 * @since 2.0
 */
class Parmscontroller : public QObject
{

    Q_OBJECT

public:

enum Parm
{
    PM_TabBarHeight,
    PM_PushButtonHeight,
    PM_ToolButtonHeight,
    PM_SearchLineEditHeight,
    PM_PasswordEditHeight,
    PM_NavigationBatHeight,
    PM_TagHeight,
    PM_SearchLineEditItemHeight,
    PM_SliderHandleRadius,
    PM_SliderNodeRadius,
    PM_BadgeHeight,
    PM_IconbarHeight,
    PM_IconBarIconSize,
    PM_WindowButtonBarSize,
    PM_NavigationBatInterval,
//    PM_NavigationBarWidth,
    PM_Widget_SideWidget_Width,
    PM_InputDialog_Height,
    PM_InputDialog_Width,
    PM_InputDialog_Label_Spacing,
    PM_InputDialog_Widget_Spacing,
    PM_InputDialog_Button_Spacing,
    PM_InputDialog_Bottom_Spacing,
    PM_InputDialog_Right_Spacing,
};
    static Parmscontroller* self();
    static bool isTabletMode();
    static int parm(Parm p);

Q_SIGNALS:
    void modeChanged(bool);

private:
    explicit Parmscontroller(QObject *parent = nullptr);
    ~Parmscontroller();

};
}
#endif // PARMSCONTROLLER_H
