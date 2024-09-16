// SPDX-FileCopyrightText: 2015 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>
#include <QStringList>

namespace DDESESSIONCC
{

static const QString CONFIG_FILE("/var/lib/AccountsService/deepin/users/");
static const QString DEFAULT_CURSOR_THEME("/usr/share/icons/default/index.theme");
static const QString LAST_USER_CONFIG("/var/lib/lightdm/lightdm-deepin-greeter");
static const int PASSWDLINEEIDT_WIDTH = 280;
static const int PASSWDLINEEDIT_HEIGHT = 36;
static const int LOCK_CONTENT_TOPBOTTOM_WIDGET_HEIGHT = 132; // 顶部和底部控件(日期区域和底部区域)的高度
static const int LOCK_CONTENT_CENTER_LAYOUT_MARGIN = 33; // SessionBaseWindow 中mainlayout的上下间隔
static const int BIO_AUTH_STATE_PLACE_HOLDER_HEIGHT = 42; // 生物认证状态占位高度
static const int BIO_AUTH_STATE_BOTTOM_SPACING = 40; // 生物识别状态底部间隔
static const int CHOOSE_AUTH_TYPE_BUTTON_PLACE_HOLDER_HEIGHT = 50; // 认证类型选择按钮占位高度
static const int CHOOSE_AUTH_TYPE_BUTTON_BOTTOM_SPACING = 40; // 认证类型选择按钮底部间隔
static const double AUTH_WIDGET_TOP_SPACING_PERCENT = 0.35; // 认证窗口顶部间隔占整个屏幕的百分比
static const int LAYOUTBUTTON_HEIGHT =  34;
static const int KEYBOARDLAYOUT_WIDTH = 200;

static const int CapslockWarningWidth = 23;
static const int CapslockWarningRightMargin = 8;
static const int BASE_SCREEN_HEIGHT = 1080;
static const int MIN_AUTH_WIDGET_HEIGHT = 276; // 密码验证时整个登录控件的大小，默认为最小高度（不考虑无密码登录的情况，稍许影响）

const QStringList session_ui_configs {
    "/etc/lightdm/lightdm-deepin-greeter.conf",
    "/etc/deepin/dde-session-ui.conf",
    "/usr/share/dde-session-shell/dde-session-shell.conf",
    "/usr/share/dde-session-ui/dde-session-ui.conf"
};

const QStringList SHUTDOWN_CONFIGS {
    "/etc/lightdm/lightdm-deepin-greeter.conf",
    "/etc/deepin/dde-session-ui.conf",
    "/etc/deepin/dde-shutdown.conf",
    "/usr/share/dde-session-ui/dde-shutdown.conf"
};

static const QString SOLID_BACKGROUND_COLOR = "#000F27";        // 纯色背景色号
static const QString LOCK_DCONFIG_SOURCE = "org.deepin.dde.lock"; // 锁屏配置文件
static const QString LOGIN_DCONFIG_SOURCE = "org.deepin.dde.lightdm-deepin-greeter"; // 登录配置文件

enum AuthFactorType {
    SingleAuthFactor,
    MultiAuthFactor,
};
}

static constexpr int LINEEDIT_SPACING = 30;

namespace Popup
{
static constexpr int HorizontalMargin = 10;
static constexpr int VerticalMargin = 10;
}

#endif // CONSTANTS_H
