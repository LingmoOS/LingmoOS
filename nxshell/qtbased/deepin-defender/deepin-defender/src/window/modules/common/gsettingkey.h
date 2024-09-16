// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QMap>

#define DEEPIN_DEFENDER_GSETTING_PATH "com.deepin.dde.deepin-defender" // 安全中心gsetting路径
#define AUTO_SCAN_USB "auto-scan-usb" // USB自动检测配置项
#define CLOSE_TYPE "close-type" // 主界面关闭方式配置项
#define ASK_TYPE "ask-type" // 主界面退出询问弹框配置项
#define NET_CONTROL_ON_OFF "net-control-on-off" // 联网管控总开关配置项
#define REM_CONTROL_ON_OFF "rem-control-on-off" // 远程访问总开关配置项
#define REM_REGISTER_ON_OFF "rem-register-on-off" // 远程登陆端口配置项
#define DATA_USAGE_ON_OFF "data-usage-on-off" // 流量监控配置项
#define NET_CONTROL_APPS_STATUS "net-control-apps-status" // 联网管控所有应用状态配置项
#define REM_CONTROL_DEFAULT_STATUS "rem-control-default-status" // 远程访问默认状态配置项
#define CLEANER_LAST_TIME_CLEANED "cleaner-last-time-cleaned" // 垃圾清理-上次清理大小
// 首页体检
#define LAST_CHECK_TIME "last-check-time" // 上次体检时间
#define SAFETY_SCORE "safety-score" // 上次体检得分
#define IGNORE_AUTO_START_CHECKING "ignore-auto-start-checking" // 是否忽略检测自启动项
#define IGNORE_DEV_MODE_CHECKING "ignore-dev-mode-checking" // 是否忽略检测开发者模式项
// 病毒查杀
#define RISING_LOCAL_VERSION "rising-local-version" // 瑞星引擎本地版本
#define RISING_SERVER_VERSION "rising-server-version" // 瑞星引擎服务器版本
#define AH_ENGINE_LOCAL_VERSION "ah-engine-local-version" // 安恒引擎本地版本
#define LAST_SCAN_TIME "last-scan-time" // 病毒配置项-上次扫描时间
#define CURRENT_VERSION "current-version" // 病毒配置项-当前版本
#define DEFENDER_INSTALL_TIME "defender-install-time" // 病毒配置项-安装时间
#define USB_STORAGE_DEVICES_TYPE "usb-storage-devices-type" // usb存储设备保护配置项
#define EXECUTE_SCAN_SIZE "execute-scan-size" // 是否执行扫描大小设置
#define SHOW_SCAN_SIZE "show-scan-size" // 设置表中的数据
#define EXECUTE_VRIUS_UPDATE_ADDRESS "execute-vrius-update-address" // 是否使用病毒升级地址
#define VRIUS_UPDATE_ADDRESS "vrius-update-address" // 设置病毒升级地址
#define VRIUS_LAST_TYPE "vrius-last-type" // 病毒引擎上一次类型
#define VRIUS_ADDRESS_CHANGHE "update-address-change" // 病毒地址是否改变
// 登录安全
#define PWD_LIMIT_LEVEL "pwd-limit-level" // 密码限制等级
#define PWD_CHANGE_DEADLINE_TYPE "pwd-change-deadline-type" // 密码修改截止时间类型

// usb管控
#define USB_STORAGE_LIMIT_MODE "usb-storage-limit-mode" // usb存储设备管控模式
#define USB_STORAGE_CONN_LOG_SWITCH "usb-storage-conn-switch" // usb存储设备连接日志开关
#define USB_STORAGE_WHITELIST "usb-storage-whitelist" // usb存储设备管控白名单
