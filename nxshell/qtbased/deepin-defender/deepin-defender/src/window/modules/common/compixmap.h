// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPIXMAP_H
#define COMPIXMAP_H

#pragma once

#include <QString>

/*********************提示框****************************/
#define DIALOG_TIP_YELLOW "dcc_login_tip" // 提示框---黄色感叹号
#define DIALOG_TIP_RED "dcc_warning" // 提示框---红色感叹号
#define DIALOG_WARNING_TIP_RED "dcc_warning_tip" // 提示框---红色感叹号(小图片)
#define DIALOG_DEFENDER "deepin-defender" // 提示框---安全中心

/********************导航栏*****************************/
#define HOMEPAGE_MODEL_LIGHT "dcc_home" // 首页模块--浅色
#define HOMEPAGE_MODEL_DARK "dcc_home_dark" // 首页模块--深色
#define HOMEPAGE_CLEAR_LIGHT "dcc_cleanup" // 垃圾清理模块--浅色
#define HOMEPAGE_CLEAR_DARK "dcc_cleanup_dark" // 垃圾清理模块--深色
#define HOMEPAGE_SECURITY_LIGHT "dcc_application_security" // 安全防护模块--浅色
#define HOMEPAGE_SECURITY_DARK "dcc_application_security_dark" // 安全防护模块--深色
#define HOMEPAGE_TOOL_LIGHT "dcc_tools" // 安全工具模块--浅色
#define HOMEPAGE_TOOL_DARK "dcc_tools_dark" // 安全工具模块--深色

/********************首页体检*****************************/
#define HOMEPAGE_DEEPIN_LIGHT "dcc_Home_deepin_image" // 首页 deepin--浅色
#define HOMEPAGE_DEEPIN_DARK "dcc_Home_deepin_image_dark" // 首页 deepin--深色
#define HOMEPAGE_DEEPIN_RESULT_LIGHT "dcc_Home_detection-deepin_image" // 结果页面 deepin--浅色
#define HOMEPAGE_DEEPIN_RESULT_DARK "dcc_Home_detection_deepin_image_dark" // 结果页面 deepin--深色
#define HOMEPAGE_UOS_LIGHT "dcc_home_image" // 首页 UOS--浅色
#define HOMEPAGE_UOS_DARK "dcc_home_image_dark" // 首页 UOS--深色
#define HOMEPAGE_UOS_OEM_LIGHT "dcc_home_image_oem" // 首页 UOS--浅色
#define HOMEPAGE_UOS_OEM_DARK "dcc_home_image_oem_dark" // 首页 UOS--深度
#define HOMEPAGE_UOS_RESULT_LIGHT "dcc_home_detection_image" // 结果页面 UOS--浅色
#define HOMEPAGE_UOS_RESULT_DARK "dcc_home_detection_image_dark" // 结果页面 UOS--深色
#define CHECK_ITEM_ABNORMAL "dcc_home_restore" // 修复结果异常
#define CHECK_ITEM_NORMAL "dcc_home_normal" // 修复结果正常
#define CHECK_ITEM_ANTIVIRUS "def_trojan_virus" // 病毒查杀体检项 标题图片
#define CHECK_ITEM_VIRUS_LIB_VER "def_virus_reservoir" // 病毒库体检项 标题图片
#define CHECK_ITEM_AUTO_START "def_self_starting" // 自启动项 标题图片
#define CHECK_ITEM_SSH "def_rdp_port" // 远程访问端口 标题图片
#define CHECK_ITEM_SYS_UPDATE "def_system_upgrade" // 系统更新 标题图片
#define CHECK_ITEM_CLEANER "def_garbage" // 垃圾清理 标题图片
#define CHECK_ITEM_DISK_CHECK "def_disk" // 磁盘检测 标题图片
#define CHECK_ITEM_ROOT_MODE "def_developers" // 开发者模式 标题图片

/*******************病毒查杀*********************/
#define FULL_SCAN_ICON "dcc_antiav_all1" // 全盘扫描图标
#define QUICK_SCAN_ICON "dcc_antiav_quick1" // 快速扫描图标
#define CUSTOMER_SCAN_ICON "dcc_antiav_customer1" // 自定义扫描图标
#define ANTIAV_LOG_LIGHT "dcc_antiav_scanLogs" // 病毒扫描日志图标--浅色
#define ANTIAV_LOG_DARK "dcc_antiav_scanLogs_dark" // 病毒扫描日志图标--深色
#define ANTIAV_QUARANTINE_LIGHT "dcc_antiav_quarantine" // 隔离区图标--浅色
#define ANTIAV_QUARANTINE_DARK "dcc_antiav_quarantine_dark" // 隔离区图标--深色
#define ANTIAV_TRUST_LIGHT "dcc_antiav_whitelist" // 信任区图标--浅色
#define ANTIAV_TRUST_DARK "dcc_antiav_whitelist_dark" // 信任区图标--深色
#define ANTIAV_LOG_DETAIL_LIGHT "dcc_natiav_log_details" // 日志详情--浅色
#define ANTIAV_LOG_DETAIL_DARK "icon_log_details_dark" // 日志详情--深色
#define ANTIAV_REPAIR_BUTTON_LIGHT "icon_repair" // 修复按钮--浅色
#define ANTIAV_REPAIR_BUTTON_DARK "icon_repair_dark" // 修复按钮--深色
#define ANTIAV_ISOLATION_BUTTON_LIGHT "icon_isolation" // 隔离按钮--浅色
#define ANTIAV_ISOLATION_BUTTON_DARK "icon_isolation_dark" // 隔离按钮--深色
#define ANTIAV_TRUST_BUTTON_LIGHT "icon_trust" // 信任按钮--浅色
#define ANTIAV_TRUST_BUTTON_DARK "icon_trust_dark" // 信任按钮--深色
#define ANTIAV_RESULT_EMPTY "dcc_antiav_empty" // 扫描结果异常为空页面
#define ANTIAV_OPERATION_SUCCESS "dcc_antiav_normal" // 操作状态操作成功以后
#define ANTIAV_OPERATION_WARNING "dcc_antiav_warning" // 异常数据没有操作前
#define ANTIAV_RISING_LOG "dcc_rising_logo" // 瑞星log
#define ANTIAV_AH_LOG "dcc_Ahprotector_log" // 安恒log
#define UPDATE_VERSION_ICON "dcc_update_version" //更新图标图标

/*******************安全工具********************/
#define DATA_USAGE_ICON "def_data_usage" // 流量管理
#define START_PROGRAMS_ICON "def_startup_programs" // 启动项管理
#define INTERNET_CONTROL_ICON "def_internet_control" // 联网应用
#define USB_CONTROL_ICON "def_usb_connection" // usb管控图标
#define USB_CONTROL_CONNECTED_ICON "dcc_startup_enable" // usb设备已连接
#define USB_CONTROL_LIMITED_ICON "dcc_startup_disable" // usb设备被限制连接
#define USB_CONTROL_DETAIL_ICON "def_usb_connection_detail" // usb白名单详情
#define USB_CONTROL_REMOVE_ICON "def_usb_connection_remove" // usb白名单移除
// 登陆安全
#define LOGIN_SAFETY_ICON "def_login_safety" // 流量管理
#define LOGIN_SAFETY_LEVEL_SELECTED "def_select" // 登陆安全等级项选中
/*******************垃圾清理*******************/
#define TRASH_CLEAN_HEADER_LOGO "dcc_cleaner_header_logo" // 垃圾清理标题LOGO
#define TRASH_CLEAN_CONFIG_BIG_LOGO_DARK "dcc_cleaner_big_logo_dark" // 垃圾清理配置LOGO
#define TRASH_CLEAN_CONFIG_BIG_LOGO_LIGHT "dcc_cleaner_big_logo" // 垃圾清理配置LOGO

#endif // COMPIXMAP_H
