/*
 * Copyright (C) 2020 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Kobe Lee    xiangli@ubuntulingmo.com/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef QWER_H
#define QWER_H

/**
  *DBus
  */
#define LINGMO_USER_GUIDE_PATH "/"
#define LINGMO_USER_GUIDE_SERVICE "com.lingmoUserGuide.hotel"
#define LINGMO_USER_GUIDE_INTERFACE "com.guide.hotel"

/**
 * QT主题
 */
#define THEME_QT_SCHEMA "org.lingmo.style"
#define MODE_QT_KEY "style-name"
#define FONT_SIZE "system-font-size"
#define THEME_QT_TRANS "org.lingmo.control-center.personalise"

// process table view backup setting key
#define SETTINGSOPTION_PROCESSTABLEHEADERSTATE "process_table_header_state"

// filesystem table view backup setting key
#define SETTINGSOPTION_FILESYSTEMTABLEHEADERSTATE "filesystem_table_header_state"

// service table view backup setting key
#define SETTINGSOPTION_SERVICETABLEHEADERSTATE "service_table_header_state"

//main window size
#define MAINWINDOW_SIZE_W   "window_w"
#define MAINWINDOW_SIZE_H   "window_h"
#define MAINWINDOW_SIZE_MAX   "window_ismax"

/* QT图标主题 */
#define ICON_QT_KEY "icon-theme-name"

/*
 * about the size of every single control
 */
#define NORMALHEIGHT 36
#define SPECIALWIDTH 240
#define NORMALWIDTH 120
#define MAINWINDOWHEIGHT 480
#define MAINWINDOWHEIGHT_DEFAULT 750
#define MAINWINDOWWIDTH 1020
#define SEARCHBUTTON 16
#define DEFAULT_FONT_SIZE 11

#define SERVICE_NAME_SIZE 64
//#define LINGMO_SYSTEM_MONITOR_PATH "/"
//#define LINGMO_SYSTEM_MONITOR_SERVICE "com.lingmosystemmonitor.hotel"
//#define LINGMO_SYSTEM_MONITOR_INTERFACE "com.systemmonitor.hotel"

#define PADDING 1.5
#define POINTSPACE 10


//process setting
#define namepadding 140
#define userpadding 90
#define diskpadding 90
#define cpupadding 90
#define idpadding 90
#define networkpadding 90
#define memorypadding 100
#define prioritypadding 100
#define direciconposition 100
#define SLIDER_WIDTH 16

//file system setting
#define devicepadding 120
#define mounturiadding 150
#define typepadding 80
#define totalcapacitypadding 100
#define idlepadding 100
#define avaliablepadding 100
#define usedpadding 100

//service setting
#define servicenamepadding 180
#define serviceloadstatepadding 110
#define serviceactivestatepadding 110
#define servicestatepadding 110
#define servicedescpadding 180
#define servicesubstatepadding 110
#define servicemainpidpadding 110
#define servicestartupmodepadding 110

enum SIGTYPE
{
    REDTYPE = 0,
    PURPLETYPE,
    GREENTYPE,
    BLUETYPE,
    YELLOWTYPE
};

// custom proc map config
#define LINGMO_SYSTEM_MONITOR_CONF  "/usr/share/lingmo/lingmo-system-monitor.conf"

#define ITEMHEIGHT 30
#define ITEMHSPACE 10
#define ITEMVSPACE 5
#define PAGESPACE 20
#define ITEMWIDTH 650
#define SHADOW_LEFT_TOP_PADDING 2
#define SHADOW_RIGHT_BOTTOM_PADDING 4
#define MAIN_WINDOW_WIDTH 900
#define MAIN_WINDOW_HEIGHT 600
#define TITLE_BAR_HEIGHT 39
#define ITEM_LEFT_RIGHT_PADDING 5
//const int windowShadowPadding = 10;

//#define VERSION "2.4.1"

#define LINGMO_COMPANY_SETTING "lingmo"
#define LINGMO_SETTING_FILE_NAME_SETTING "lingmo-system-monitor"

#define LEFT_WIDGET_WIDTH 214
#define LEFT_WIDGET_HEIGHT MAINWINDOWHEIGHT
#define LEFT_BUTTON_WIDTH 184
#define LEFT_BUTTON_HEITHT 36
#define RIGHT_WIDGET_WIDTH 808
#define RIGHT_WIDGET_HEIGHT MAINWINDOWHEIGHT

#define MAINWINDOW_DBCLICK_WIDTH  50

#define MENU_SCHEMA "org.lingmo.system-monitor.menu"
#define WHICH_MENU "which-menu"
#define WHICH_MENU_KEY "whichMenu"
#define SUPPORT_SERVICE "support-service"
#define SUPPORT_SERVICE_KEY "supportService"

#endif
