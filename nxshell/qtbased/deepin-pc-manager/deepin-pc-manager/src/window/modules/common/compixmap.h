// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPIXMAP_H
#define COMPIXMAP_H

#pragma once

#include <QString>

/*********************公共图标****************************/
#define ICON_SEARCH "search" // 搜索图标
#define ICON_ANIMATED_LINE_EDIT_GLOWING "def_animated_line_edit_glowing" // 编辑框动画
#define ICON_WARNING "def_warning" // 警告图标

/*********************提示框****************************/
#define DIALOG_TIP_YELLOW "dcc_login_tip" // 提示框---黄色感叹号
#define DIALOG_TIP_RED "dcc_warning" // 提示框---红色感叹号
#define DIALOG_WARNING_TIP_RED "dcc_warning_tip" // 提示框---红色感叹号(小图片)
#define DIALOG_CARE_TIP_YELLOW "dcc_care_tip" // 提示框---黄色感叹号(小图片)
#define DIALOG_OK_TIP_GREEN "dcc_ok_tip" // 提示框---绿色感叹号(小图片)
#define DIALOG_DEFENDER "deepin-defender" // 提示框---安全中心
#define DIALOG_PC_MANAGER "deepin-pc-manager" // 提示框---电脑管家

/********************导航栏*****************************/
#define HOMEPAGE_MODEL_LIGHT "dcc_home" // 首页模块--浅色
#define HOMEPAGE_MODEL_DARK "dcc_home_dark" // 首页模块--深色
#define HOMEPAGE_ANTIVIRUS_LIGHT "dcc_virus_scan" // 病毒查杀模块--浅色
#define HOMEPAGE_ANTIVIRUS_DARK "dcc_Virus_Scan_dark" // 病毒查杀模块--深色
#define ICON_MODULE_ICON_FIREWALL "dcc_network_protection" // 防火墙
#define ICON_MODULE_ICON_DARK_FIREWALL "dcc_network_protection_dark" // 防火墙
#define HOMEPAGE_CLEAR_LIGHT "dcc_cleanup" // 垃圾清理模块--浅色
#define HOMEPAGE_CLEAR_DARK "dcc_cleanup_dark" // 垃圾清理模块--深色
#define HOMEPAGE_SECURITY_LIGHT "dcc_application_security" // 安全防护模块--浅色
#define HOMEPAGE_SECURITY_DARK "dcc_application_security_dark" // 安全防护模块--深色
#define HOMEPAGE_TOOL_LIGHT "dcc_tools" // 安全工具模块--浅色
#define HOMEPAGE_TOOL_DARK "dcc_tools_dark" // 安全工具模块--深色

/********************首页体检*****************************/
#define HOMEPAGE_DEEPIN_LIGHT "dcc_Home_deepin_image" // 首页 deepin--浅色
#define HOMEPAGE_DEEPIN_DARK "dcc_Home_deepin_image_dark" // 首页 deepin--深色
#define HOMEPAGE_UOS_LIGHT "dcc_home_image_oem" // 首页 UOS--浅色
#define HOMEPAGE_UOS_DARK "dcc_home_image_oem_dark" // 首页 UOS--深色
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
#define VIRUS_HOME_PAGE "dcc_virus_home_page" // 病毒查杀首页图标
#define VIRUS_CHECK_UPDATE "dcc_virus_check_update" // 病毒查杀检查更新图标
#define VIRUS_UPDATE "dcc_virus_update" // 病毒查杀更新图标
#define ANTIAV_LOG_LIGHT "dcc_antiav_scanLogs" // 病毒扫描日志图标--浅色
#define ANTIAV_LOG_DARK "dcc_antiav_scanLogs_dark" // 病毒扫描日志图标--深色
#define ANTIAV_QUARANTINE_LIGHT "dcc_antiav_quarantine" // 隔离区图标--浅色
#define ANTIAV_QUARANTINE_DARK "dcc_antiav_quarantine_dark" // 隔离区图标--深色
#define ANTIAV_TRUST_LIGHT "dcc_antiav_whitelist" // 信任区图标--浅色
#define ANTIAV_TRUST_DARK "dcc_antiav_whitelist_dark" // 信任区图标--深色
#define ANTIAV_WHITELIST_LIGHT "def_Icon_Whitelist" // 白名单图标--浅色
#define ANTIAV_LOG_DETAIL_LIGHT "dcc_natiav_log_details" // 日志详情--浅色
#define ANTIAV_LOG_DETAIL_DARK "icon_log_details_dark" // 日志详情--深色
#define ANTIAV_OPERATION_SUCCESS "dcc_antiav_normal" // 操作状态操作成功以后
#define ANTIAV_RISING_LOG "dcc_rising_logo" // 瑞星log
#define ANTIAV_AH_LOG "dcc_Ahprotector_log" // 安恒log
#define UPDATE_VERSION_ICON "dcc_update_version" // 更新图标图标
#define WHITE_GARBAGE_ICON "def_delete" // 白名单删除图标
#define SCHEDULE_ICON "icon_schedulescan" // 定时扫描图标
#define USBSCAN_ICON "icon_usbscan" // USB扫描图标
#define SCANSETTING_ICON "icon_scansetting" // 扫描设置图标
#define SCANSETTING_LIGHT_ICON "icon_scansetting_light" // 扫描设置图标
#define SCANSETTING_DARK_ICON "icon_scansetting_dark" // 扫描设置图标

/*******************安全工具********************/
#define DATA_USAGE_ICON "def_data_usage" // 流量管理
#define START_PROGRAMS_ICON "def_startup_programs" // 启动项管理
#define INTERNET_CONTROL_ICON "def_internet_control" // 联网应用
#define USB_CONTROL_ICON "def_usb_connection" // usb管控图标
#define SYSTEM_SAFE_LEVEL_ICON "def_system_safe_level" // 系统安全登记图标
#define FILE_TRUST_PROTECTION_ICON "def_file_trust_protection" // 可信保护图标
#define USB_CONTROL_CONNECTED_ICON "dcc_startup_enable" // usb设备已连接
#define USB_CONTROL_LIMITED_ICON "dcc_startup_disable" // usb设备被限制连接
#define USB_CONTROL_DETAIL_DLG_ICON "def_usb_storage_detail_dlg" // usb白名单详情弹框图标
#define USB_CONTROL_DETAIL_ICON "def_usb_storage_detail" // usb白名单详情
#define USB_CONTROL_ADD_ICON "def_usb_storage_add_whitelist" // usb白名单移除
#define USB_CONTROL_REMOVE_ICON "def_usb_storage_remove_whitelist" // usb白名单移除
#define SECURITYLEVEL_NOTICE_ICON "def_securitylevel_privilege" // 三权分立问号图标
#define FILE_NET_CHECK_ICON "def_net_check" // 网络检测图标

// 登陆安全
#define LOGIN_SAFETY_ICON "def_login_safety" // 流量管理
#define LOGIN_SAFETY_LEVEL_SELECTED "def_select" // 登陆安全等级项选中
/*******************垃圾清理*******************/
#define TRASH_CLEAN_HEADER_LOGO "dcc_cleaner_header_logo" // 垃圾清理标题LOGO
#define TRASH_CLEAN_CONFIG_BIG_LOGO_DARK "dcc_cleaner_big_logo_dark" // 垃圾清理配置LOGO
#define TRASH_CLEAN_CONFIG_BIG_LOGO_LIGHT "dcc_cleaner_big_logo" // 垃圾清理配置LOGO

/*******************防火墙*******************/
#define ICON_FIREWALL_MODE_PUBLIC "def_firewall_mode_public"
#define ICON_FIREWALL_MODE_PRIVATE "def_firewall_mode_private"
#define ICON_FIREWALL_MODE_CUSTOMIZED "def_firewall_mode_customized"
#define ICON_ARROW_RIGHT "def_arrow_right"
#define ICON_FIREWALL_POLICY_ENABLE "def_firewall_policy_enable"
#define ICON_FIREWALL_POLICY_DISABLE "def_firewall_policy_disable"
#define ICON_FIREWALL "firewall"

/*******************系统安全等级*******************/
#define ICON_SYS_SEC_LEVEL_LOGO "System_security_level"
#define ICON_SYS_SEC_LEVEL_HIGH "dcc_system_level_high"
#define ICON_SYS_SEC_LEVEL_MEDIUM "dcc_system_level_medium"
#define ICON_SYS_SEC_LEVEL_LOW "dcc_system_level_low"

#endif // COMPIXMAP_H
