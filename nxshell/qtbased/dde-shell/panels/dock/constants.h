// Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dsglobal.h"
#include "environments.h"

#include <QtCore>
#include <QQmlEngine>
#include <DGuiApplicationHelper>

namespace dock {

Q_NAMESPACE
QML_NAMED_ELEMENT(Dock)

// dock keep 10:36:10
enum SIZE {
    MIN_DOCK_SIZE = 37,
    DEFAULT_DOCK_SIZE = 56,
    MAX_DOCK_SIZE = 100,
    MIN_DOCK_TASKMANAGER_ICON_SIZE = 24,
    MAX_DOCK_TASKMANAGER_ICON_SIZE = 64
};

enum IndicatorStyle {
    Fashion     = 0,
    Efficient   = 1,
};

enum ItemAlignment {
    CenterAlignment      = 0,
    LeftAlignment        = 1,
};

enum ColorTheme {
    Light = Dtk::Gui::DGuiApplicationHelper::ColorType::LightType,
    Dark  = Dtk::Gui::DGuiApplicationHelper::ColorType::DarkType,
};

enum HideMode {
    KeepShowing     = 0,
    KeepHidden      = 1,
    SmartHide       = 2
};

enum Position {
    Top     = 0,
    Right   = 1,
    Bottom  = 2,
    Left    = 3,
};

enum HideState {
    Unknown     = 0,
    Show        = 1,
    Hide        = 2,
};

enum AniAction {
    AA_Show = 0,
    AA_Hide
};

enum TrayPopupType {
    TrayPopupTypePanel = 1,
    TrayPopupTypeTooltip = 2,
    TrayPopupTypeMenu = 3,
    TrayPopupTypeEmbed = 4,
    TrayPopupTypeSubPopup = 5
};

enum TrayPluginType {
    Tray = 1,
    Fixed,
    Quick,
};

enum TrayPluginSizePolicy {
    System = 1 << 0, // Follow the system
    Custom = 1 << 1  // The custom
};

/**
 * @brief 用于在插件的 message 和 MessageCallbackFunc 方法中解析 json格式 数据。
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
 * @brief 任务栏应用溢出状态
 */
const QString MSG_UPDATE_OVERFLOW_STATE = QStringLiteral("updateOverflowState");
const int OVERFLOW_STATE_NOT_EXIST = 0; // 没有溢出区
const int OVERFLOW_STATE_EXIST = 1; // 有溢出区
const int OVERFLOW_STATE_ALL = 2; // 所有应用都在溢出区

// for qml
enum OverFlowState {
    OverflowNotExist = OVERFLOW_STATE_NOT_EXIST,
    OverflowExist = OVERFLOW_STATE_EXIST,
    OverflowAll = OVERFLOW_STATE_ALL
};

/**
 * @brief 任务栏面板Size；当任务栏size发生改变时，通知插件
 * 插件根据任务栏size做出大小调整，例如时间日期插件
 */
const QString MSG_DOCK_PANEL_SIZE_CHANGED = QStringLiteral("dockPanelSizeChanged");

/**
 * @brief 最小弹窗高度，根据快捷面板的高度动态变化
 * 任务栏主动发送给插件，只会给快捷插件发送
 */
const QString MSG_SET_APPLET_MIN_HEIGHT = QStringLiteral("setAppletMinHeight");

const QString KeyHiddenSurfaceIds = "hiddenSurfaceIds";

const QString DCCIconPath = CMAKE_INSTALL_PREFIX + QString("/share/dde-dock/icons/dcc-setting/");

Q_ENUM_NS(SIZE)
Q_ENUM_NS(IndicatorStyle)
Q_ENUM_NS(ItemAlignment)
Q_ENUM_NS(ColorTheme)
Q_ENUM_NS(HideMode)
Q_ENUM_NS(Position)
Q_ENUM_NS(HideState)
Q_ENUM_NS(AniAction)
Q_ENUM_NS(TrayPopupType)
Q_ENUM_NS(OverFlowState)
Q_ENUM_NS(TrayPluginType)
Q_ENUM_NS(TrayPluginSizePolicy)
}

Q_DECLARE_METATYPE(dock::SIZE)
Q_DECLARE_METATYPE(dock::IndicatorStyle)
Q_DECLARE_METATYPE(dock::ItemAlignment)
Q_DECLARE_METATYPE(dock::ColorTheme)
Q_DECLARE_METATYPE(dock::HideMode)
Q_DECLARE_METATYPE(dock::HideState)
Q_DECLARE_METATYPE(dock::AniAction)
Q_DECLARE_METATYPE(dock::Position)
Q_DECLARE_METATYPE(dock::TrayPopupType)
Q_DECLARE_METATYPE(dock::OverFlowState)
Q_DECLARE_METATYPE(dock::TrayPluginType)
Q_DECLARE_METATYPE(dock::TrayPluginSizePolicy)
