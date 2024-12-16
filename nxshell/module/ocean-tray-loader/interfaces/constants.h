// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QtCore>

#define DOCK_API_VERSION_MAJOR 2
#define DOCK_API_VERSION_MINOR 0
#define DOCK_API_VERSION_PATCH 0

// 以下为插件适配会用到的常量
namespace Dock {

#define DOCK_PLUGIN_MIME "dock/plugin"
#define DOCK_PLUGIN_API_VERSION "2.0.0" // Deprecated, use DOCK_API_VERSION_CHECK

#define PROP_DISPLAY_MODE "DisplayMode"

#define PLUGIN_BACKGROUND_MAX_SIZE 40 // Deprecated, use DOCK_PLUGIN_ITEM_FIXED_SIZE
#define PLUGIN_BACKGROUND_MIN_SIZE 16 // Deprecated, use DOCK_PLUGIN_ITEM_FIXED_SIZE

#define PLUGIN_ITEM_WIDTH 300
#define PLUGIN_ICON_MAX_SIZE 16 // Deprecated, use DOCK_PLUGIN_ITEM_FIXED_SIZE
#define PLUGIN_ICON_MIN_SIZE 16 // Deprecated, use DOCK_PLUGIN_ITEM_FIXED_SIZE

// 插件最小尺寸，图标采用深色
#define PLUGIN_MIN_ICON_NAME "-dark"

// dock最大尺寸
#define DOCK_MAX_SIZE 100
///
/// \brief The DisplayMode enum
/// spec dock display mode
///
enum DisplayMode {
    Fashion = 0,
    Efficient = 1,
};

#define PROP_HIDE_MODE "HideMode"
///
/// \brief The HideMode enum
/// spec dock hide behavior
///
enum HideMode {
    KeepShowing = 0,
    KeepHidden = 1,
    SmartHide = 3,
};

#define PROP_POSITION "Position"
///
/// \brief The Position enum
/// spec dock position, dock always placed at primary screen,
/// so all position is the primary screen edge.
///
enum Position {
    Top = 0,
    Right = 1,
    Bottom = 2,
    Left = 3,
};

#define PROP_HIDE_STATE "HideState"
///
/// \brief The HideState enum
/// spec current dock should hide or shown.
/// this argument works only HideMode is SmartHide
///
enum HideState {
    Unknown = 0,
    Show = 1,
    Hide = 2,
};

#define IS_TOUCH_STATE "isTouchState"


/**
 * @brief 任务栏 API 版本号，插件在编译期可以通过比对版本号判断接口是否兼容
 * DOCK_API_VERSION_VERSION is (major << 16) + (minor << 8) + patch.
 * @since 2.0.0
 */
#define DOCK_API_VERSION      DOCK_API_VERSION_CHECK(DOCK_API_VERSION_MAJOR, DOCK_API_VERSION_MINOR, DOCK_API_VERSION_PATCH)

/**
 * @brief 用来和 DOCK_API_VERSION 进行比对
 * can be used like #if (DOCK_API_VERSION >= DOCK_API_VERSION_CHECK(2, 0, 0))
 * @since 2.0.0
 */
#define DOCK_API_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))

/**
 * @brief 插件的标志位
 * @since 2.0.0
 */
enum PluginFlag {
    Type_None = 0x00, // 默认值，插件无需关注
    Type_Unadapted = 0x01, // 插件类型-未适配，插件适配时勿使用这个flag
    Type_Quick = 0x02, // 插件类型-快捷插件区
    Type_Tool = 0x04, // 插件类型-工具插件，例如回收站
    Type_System = 0x08, // 插件类型-系统插件，例如关机插件
    Type_Tray = 0x10, // 插件类型-托盘区，例如U盘插件
    Type_Fixed = 0x20, // 插件类型-固定区域,例如多任务视图和显示桌面

    Quick_Panel_Single = 0x40, // 当插件类型为Common时,快捷插件区域只有一列的那种插件
    Quick_Panel_Multi = 0x80, // 当插件类型为Common时,快捷插件区占两列的那种插件
    Quick_Panel_Full = 0x100, // 当插件类型为Common时,快捷插件区占用4列的那种插件，例如声音、亮度设置和音乐等

    Attribute_CanDrag = 0x200, // 插件属性-是否支持拖动
    Attribute_CanInsert = 0x400, // 插件属性-是否支持在其前面插入其他的插件，普通的快捷插件是支持的
    Attribute_CanSetting = 0x800, // 插件属性-是否可以在控制中心设置显示或隐藏，如果设置了这个属性，请实现PluginsItemInterfaceV2::icon 接口，并返回在`控制中心-个性化-任务栏-插件区域`中显示的图标
    Attribute_ForceDock = 0x1000, // 插件属性-强制显示在任务栏上

    Attribute_Normal = Attribute_CanDrag | Attribute_CanInsert | Attribute_CanSetting, // 普通插件

    FlagMask = 0xffffffff // 掩码
};
Q_DECLARE_FLAGS(PluginFlags, PluginFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(PluginFlags)

/**
 * @brief 图标的类型
 * @since 2.0.0
 */
enum IconType
{
    IconType_None = 0, // 默认，无实际意义
};

/**
 * @brief 主题类型，和 dtk 的标志位对应
 * @since 2.0.0
 */
enum ThemeType {
    ThemeType_None, // 不涉及
    ThemeType_Light, // 亮色
    ThemeType_Dark // 暗色
};

/**
 * @brief 2.0.0 新增常量
 * @since 2.0.0
 */
const QString QUICK_TOP_ACTION = QStringLiteral("quick_top_action"); // 快捷面板子页面右上角控件
const QString QUICK_ITEM_KEY = QStringLiteral("quick_item_key"); // 快捷面板详情页面的itemWidget对应的itemKey
// 在 QApplication 设置的属性，插件可以在运行时获取API版本号,可以通过 qApp->property(DOCK_API_VERSION_PROPERTY) 获取版本号
// 然后是同DOCK_API_VERSION_CHECK的方法去比较版本大小
const QByteArray DOCK_API_VERSION_PROPERTY = "dock_api_version";
const int DOCK_PLUGIN_ITEM_FIXED_WIDTH = 16; // 插件在任务栏上面的固定宽度
const int DOCK_PLUGIN_ITEM_FIXED_HEIGHT = 16; // 插件在任务栏上面的固定高度
const QSize DOCK_PLUGIN_ITEM_FIXED_SIZE(DOCK_PLUGIN_ITEM_FIXED_WIDTH, DOCK_PLUGIN_ITEM_FIXED_HEIGHT); // 插件在任务栏上面的固定大小
const int TRAY_PLUGIN_ITEM_FIXED_WIDTH = 16; // 托盘插件的固定宽度
const int TRAY_PLUGIN_ITEM_FIXED_HEIGHT = 16; // 托盘插件的固定高度
const QSize TRAY_PLUGIN_ITEM_FIXED_SIZE(TRAY_PLUGIN_ITEM_FIXED_WIDTH, TRAY_PLUGIN_ITEM_FIXED_HEIGHT); // 托盘插件的固定大小
const int DOCK_POPUP_WIDGET_WIDTH = 330; // 任务栏弹窗的宽度
const int DOCK_POPUP_WIDGET_MAX_HEIGHT = 600; // 任务栏弹窗的最大高度
const int QUICK_PANEL_ICON_WIDTH = 24; // 快捷面板中插件图标的宽度
const int QUICK_PANEL_ICON_HEIGHT = 24; // 快捷面板中插件图标的高度
const QSize QUICK_PANEL_ICON_SIZE(QUICK_PANEL_ICON_WIDTH, QUICK_PANEL_ICON_HEIGHT); // 快捷面板中插件图标的大小
const int QUICK_ITEM_HEIGHT = 60; // 快捷面板插件高度
const int QUICK_ITEM_SINGLE_WIDTH = 70; // 单格快捷面板插件宽度
const int QUICK_ITEM_MULTI_WIDTH = 150; // 双格快捷面板插件宽度
const int QUICK_ITEM_FULL_WIDTH = 310; // 整行快捷面板插件宽度

/**
 * @brief 用于在插件的 message 和 MessageCallbackFunc 方法中解析 json格式 数据。
 * 详细的说明见 plugins-developer-guide.md 文档
 * @since 2.0.0
 */
const QString MSG_TYPE = QStringLiteral("msgType"); // 固定 key 值，表明当前消息类型是什么
const QString MSG_DATA = QStringLiteral("data"); // 固定 key 值，从该字段中获取具体数据

/**
 * @brief 插件功能是否可用
 * eg：蓝牙被拔掉后，蓝牙插件的 support 的状态应该是 false，任务栏会把蓝牙的图标从控制中心-个性化-任务栏-插件区域中移除
 */
const QString MSG_GET_SUPPORT_FLAG = QStringLiteral("getSupportFlag");
const QString MSG_SUPPORT_FLAG = QStringLiteral("supportFlag");
const QString MSG_SUPPORT_FLAG_CHANGED = QStringLiteral("supportFlagChanged");

/**
 * @brief 任务栏应用溢出状态
 */
const QString MSG_UPDATE_OVERFLOW_STATE = QStringLiteral("updateOverflowState");
const int OVERFLOW_STATE_NOT_EXIST = 0; // 没有溢出区
const int OVERFLOW_STATE_EXIST = 1; // 有溢出区
const int OVERFLOW_STATE_ALL = 2; // 所有应用都在溢出区

/**
 * @brief 最小弹窗高度，根据快捷面板的高度动态变化
 * 任务栏主动发送给插件，只会给快捷插件发送
 */
const QString MSG_SET_APPLET_MIN_HEIGHT = QStringLiteral("setAppletMinHeight");

/**
 * @brief 插件自行决定是否要被任务栏加载，不发送此消息默认被加载
 * ture: 希望被加载，false: 不希望被加载
 */
const QString MSG_WHETHER_WANT_TO_BE_LOADED = QStringLiteral("whetherWantToBeLoaded");

/**
 * @brief 弹窗是在任务栏上直接显示还是快捷面板的二级页面显示
 * 插件可以根据显示的位置做一些样式调整
 */
const QString MSG_APPLET_CONTAINER = QStringLiteral("appletContainer");
const int APPLET_CONTAINER_DOCK = 0; // 任务栏
const int APPLET_CONTAINER_QUICK_PANEL = 1; // 快捷面板

/**
 * @brief 插件图标的激活状态；当状态发生变化时，插件需要主动把状态发送给任务栏
 * true: 插件处于激活状态，false：插件出于失活状态
 */
const QString MSG_ITEM_ACTIVE_STATE = QStringLiteral("itemActiveState");
/**
 * @brief 插件请求任务栏更新插件的 tooltips
 * 任务栏收到请求后会主动调用 itemTips() 方法。
 * 一般用于一个插件里面含有多个图标，鼠标 hover 到不同图标上时显示不同 tooltips 的场景。
 */
const QString MSG_UPDATE_TOOLTIPS_VISIBLE = QStringLiteral("updateTooltipsVisible");

/**
 * @brief 任务栏面板Size；当任务栏size发生改变时，通知插件
 * 插件根据任务栏size做出大小调整，例如时间日期插件
 */
const QString MSG_DOCK_PANEL_SIZE_CHANGED = QStringLiteral("dockPanelSizeChanged");

/**
 * @brief 插件属性，MSG_TYPE
 * 任务栏通过获取插件属性来确定插件状态，例如是否需要变色龙效果
 * 返回的MSG_DATA类型为QMap<QString, QVariant>，其中QString为属性名称，QVariant为对应值
 */
const QString MSG_PLUGIN_PROPERTY = QStringLiteral("pluginProperty");

/**
 * @brief 属性 - 需要变色龙，即插件需要hover、press样式
 * true: 需要；false: 不需要；
 */
const QString PLUGIN_PROP_NEED_CHAMELEON = QStringLiteral("needChameleon");

/**
 * @brief 属性 - 变色龙边距
 * 返回需要设置的QMargin()
 */
const QString PLUGIN_PROP_CHAMELEON_MARGIN = QStringLiteral("chameleonMargin");

// 插件右键菜单中"驻留任务栏/移除驻留"选项
const QString dockMenuItemId = QStringLiteral("dock-item-id");
const QString unDockMenuItemId = QStringLiteral("undock-item-id");

// 电源插件左键点击请求调出电源管理
const QString REQUEST_SHUTDOWN = QStringLiteral("requestShutdown");

}

Q_DECLARE_METATYPE(Dock::DisplayMode)
Q_DECLARE_METATYPE(Dock::Position)


#endif // CONSTANTS_H
