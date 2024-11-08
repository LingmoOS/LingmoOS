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

#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusReply>

#include "parmscontroller.h"
#include <QApplication>

namespace kdk
{

static Parmscontroller *g_parmscontroller;
static QDBusInterface *g_statusManagerDBus;
static bool g_isTablet;
const static QString DBUS_STATUS_MANAGER = "com.lingmo.statusmanager.interface";

const static int TABLET_CONTROL_HEIGHT = 48;
const static int TABLET_SLIDER_HANDLE_RADIUS = 40;
const static int TABLET_SLIDER_NODE_RADIUS = 16;
const static int TABLET_BADGE_HEIGHT = 20;
const static int TABLET_ICONBAR_HEIGHT = 64;
const static int TABLET_ICONBARICONSIZE =32;
const static int TABLET_WINDOWBUTTONBAR_SIZE =48;
const static int TABLET_NAVIGATION_INTERVAL = 4;
//const static int TABLET_NAVIGATION_WIDTH = 230;
const static int TABLET_WIDGET_SIDEWIDGET_WIDTH = 254;
const static int TABLET_INPUTDIALOG_HEIGHT = 222;
const static int TABLET_INPUTDIALOG_WIDTH = 352;
const static int TABLET_INPUTDIALOG_LABEL_SPACING = 16;
const static int TABLET_INPUTDIALOG_WIDGET_SPACING = 24;
const static int TABLET_INPUTDIALOG_BUTTON_SPACING = 16;
const static int TABLET_INPUTDIALOG_BOTTOM_SPACING = 16;
const static int TABLET_INPUTDIALOG_RIGHT_SPACING = 32;


const static int PC_CONTROL_HEIGHT = 36;
const static int PC_SLIDER_HANDLE_RADIUS = 20;
const static int PC_SLIDER_NODE_RADIUS = 10;
const static int PC_BADGE_HEIGHT = 30;
const static int PC_ICONBAR_HEIGHT = 40;
const static int PC_ICONBARICONSIZE =24;
const static int PC_WINDOWBUTTONBAR_SIZE =30;
const static int PC_NAVIGATION_INTERVAL = 4;
//const static int PC_NAVIGATION_WIDTH = 168;
const static int PC_WIDGET_SIDEWIDGET_WIDTH = 200;
const static int PC_INPUTDIALOG_HEIGHT = 198;
const static int PC_INPUTDIALOG_WIDTH = 336;
const static int PC_INPUTDIALOG_LABEL_SPACING = 8;
const static int PC_INPUTDIALOG_WIDGET_SPACING = 32;
const static int PC_INPUTDIALOG_BUTTON_SPACING = 10;
const static int PC_INPUTDIALOG_BOTTOM_SPACING = 24;
const static int PC_INPUTDIALOG_RIGHT_SPACING = 24;

Parmscontroller *Parmscontroller::self()
{
    if(g_parmscontroller)
        return g_parmscontroller;
    else
    {
        g_parmscontroller = new Parmscontroller;
        return g_parmscontroller;
    }
}

Parmscontroller::Parmscontroller(QObject *parent) : QObject(parent)
{
    g_statusManagerDBus = new QDBusInterface(DBUS_STATUS_MANAGER, "/" ,DBUS_STATUS_MANAGER,QDBusConnection::sessionBus());

    if (g_statusManagerDBus) {
        if (g_statusManagerDBus->isValid()) {
            //平板模式切换
            connect(g_statusManagerDBus, SIGNAL(mode_change_signal(bool)), this, SIGNAL(modeChanged(bool)));
            connect(this,&Parmscontroller::modeChanged,this,[=](bool flag)
            {
               g_isTablet = flag;
            });
        }
    }
    g_isTablet = isTabletMode();
}

Parmscontroller::~Parmscontroller()
{
    delete g_statusManagerDBus;
    g_statusManagerDBus = nullptr;
}

bool Parmscontroller::isTabletMode()
{
    if (g_statusManagerDBus && g_statusManagerDBus->isValid())
    {
        QDBusReply<bool> message = g_statusManagerDBus->call("get_current_tabletmode");
        if (message.isValid())
            return message.value();
        else
            return false;
    }
    else
        return false;
}

int Parmscontroller::parm(Parmscontroller::Parm p)
{
    if(g_isTablet)
    {
        switch (p) {
        case PM_TabBarHeight:
        case PM_PushButtonHeight:
        case PM_ToolButtonHeight:
        case PM_SearchLineEditHeight:
        case PM_PasswordEditHeight:
        case PM_NavigationBatHeight:
        case PM_TagHeight:
        case PM_SearchLineEditItemHeight:
            return TABLET_CONTROL_HEIGHT;
        case PM_SliderHandleRadius:
            return TABLET_SLIDER_HANDLE_RADIUS;
        case PM_SliderNodeRadius:
            return TABLET_SLIDER_NODE_RADIUS;
        case PM_BadgeHeight:
            return TABLET_BADGE_HEIGHT;
        case PM_IconbarHeight:
            return TABLET_ICONBAR_HEIGHT;
        case PM_IconBarIconSize:
            return TABLET_ICONBARICONSIZE;
        case PM_WindowButtonBarSize:
            return TABLET_WINDOWBUTTONBAR_SIZE;
        case PM_NavigationBatInterval:
            return TABLET_NAVIGATION_INTERVAL;
//        case PM_NavigationBarWidth:
//            return TABLET_NAVIGATION_WIDTH;
        case PM_Widget_SideWidget_Width:
            return TABLET_WIDGET_SIDEWIDGET_WIDTH;
        case PM_InputDialog_Height:
            return TABLET_INPUTDIALOG_HEIGHT;
        case PM_InputDialog_Width:
            return TABLET_INPUTDIALOG_WIDTH;
        case PM_InputDialog_Label_Spacing:
            return TABLET_INPUTDIALOG_LABEL_SPACING;
        case PM_InputDialog_Widget_Spacing:
            return TABLET_INPUTDIALOG_WIDGET_SPACING;
        case PM_InputDialog_Button_Spacing:
            return TABLET_INPUTDIALOG_BUTTON_SPACING;
        case PM_InputDialog_Bottom_Spacing:
            return TABLET_INPUTDIALOG_BOTTOM_SPACING;
        case PM_InputDialog_Right_Spacing:
            return TABLET_INPUTDIALOG_RIGHT_SPACING;
        default:
            return 0;
            break;
        }
    }
    else
    {
        switch (p) {
        case PM_TabBarHeight:
        case PM_PushButtonHeight:
        case PM_ToolButtonHeight:
        case PM_SearchLineEditHeight:
        case PM_PasswordEditHeight:
        case PM_NavigationBatHeight:
        case PM_TagHeight:
        case PM_SearchLineEditItemHeight:
            return PC_CONTROL_HEIGHT;
        case PM_SliderHandleRadius:
            return PC_SLIDER_HANDLE_RADIUS;
        case PM_SliderNodeRadius:
            return PC_SLIDER_NODE_RADIUS;
        case PM_BadgeHeight:
            return PC_BADGE_HEIGHT;
        case PM_IconbarHeight:
            return PC_ICONBAR_HEIGHT;
        case PM_IconBarIconSize:
            return PC_ICONBARICONSIZE;
        case PM_WindowButtonBarSize:
            return PC_WINDOWBUTTONBAR_SIZE;
        case PM_NavigationBatInterval:
            return PC_NAVIGATION_INTERVAL;
//        case PM_NavigationBarWidth:
//            return PC_NAVIGATION_WIDTH;
        case PM_Widget_SideWidget_Width:
            return PC_WIDGET_SIDEWIDGET_WIDTH;
        case PM_InputDialog_Height:
            return PC_INPUTDIALOG_HEIGHT;
        case PM_InputDialog_Width:
            return PC_INPUTDIALOG_WIDTH;
        case PM_InputDialog_Label_Spacing:
            return PC_INPUTDIALOG_LABEL_SPACING;
        case PM_InputDialog_Widget_Spacing:
            return PC_INPUTDIALOG_WIDGET_SPACING;
        case PM_InputDialog_Button_Spacing:
            return PC_INPUTDIALOG_BUTTON_SPACING;
        case PM_InputDialog_Bottom_Spacing:
            return PC_INPUTDIALOG_BOTTOM_SPACING;
        case PM_InputDialog_Right_Spacing:
            return PC_INPUTDIALOG_RIGHT_SPACING;
        default:
            return 0;
            break;
        }
    }

}
}



