// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DSysInfo>

#include <QMap>
#include <QIcon>

DCORE_USE_NAMESPACE

// 文件大小单位，以b为基本单位
#define KB_COUNT (1 << 10)
#define MB_COUNT (1 << 20)
#define GB_COUNT (1 << 30)
#define TB_COUNT (long(1) << 40)

//系统更新标志
enum UpdatesStatus {
    UpdateDefault = 0,
    Checking,
    DeependenciesBrokenError,
    CheckUpdatesFailed
};

// 主题类型
enum ColorType {
    UnknownType,
    LightType,
    DarkType
};

const QSize Q_IconSize(28, 28);

// 登录密码安全级别枚举
enum PwdLimitLevel { Low = 1,
                     Medium,
                     High };

#define ARCH_AMD "x86_64" // AMD平台

// 安全中心右上角基础设置与安全设置名称
#define SETTING_JSON ":/dt-settings.json" // 获取文件配置
#define SETTING_JSON_FOR_SRV_EDITION ":/dt-settings-for-srv.json" // 服务器版本设置数据模板
#define SETTING_JSON_FOR_RESOURCE ":/dt-settings-for-resource.json" // 服务器版本设置数据模板

#define SETTING_BASE_VIRUS_ENGIN "base.virus_engin.virus_engin_type" // 病毒引擎设置
#define SETTING_BASE_CLOSE_WINDOW "base.close_window.close_window_type" // 关闭主窗口设置
#define SETTING_SAFETY_USB_STORAGE_DEVICES "safety.usb_storage_devices.usb_storage_devices_type" // USB存储设备设置
#define SETTING_SAFETY_REM_CONTROL "safety.remote_connection.rem_control_type" // 远程控制设置
#define SETTING_RIGHT_SCAN_VISABLE "base.virus_scan.right_virus_scan" // 右键病毒扫描
#define SETTING_VIURS_CUSTOM_SCAN_SIZE "base.virus_scan.coustom_scan_size" // 病毒扫描大小设置
#define SETTING_VIURS_UPDATE_ADDRESS "base.virus_scan.vrius_update_address" // 病毒库升级地址

// 设置窗口的json key
#define SETTING_VIRUS_ENGIN_TYPE "virus_engin_type" // 病毒引擎类型
#define SETTING_CLOSE_WINDOW_TYPE "close_window_type" // 关闭窗口类型
#define SETTING_USB_STORAGE_DEVICES_TYPE "usb_storage_devices_type" // USB存储设备类型
#define SETTING_REM_CONTROL_TYPE "rem_control_type" // 远程控制类型
#define SETTING_NET_CONTROL_TYPE "internet_control_type" // 联网控制类型

// caitao/2020.6.19 define global properties
// mainwindow modules' index
#define MODULE_HOMEPAGE_INDEX 0 // 主页下标
#define MODULE_NATIAV_INDEX 1 // 病毒查杀下标
#define MODULE_NETWORK_PROTECT_INDEX 2 // 网络防护下标
#define MODULE_STARTUP_INDEX 3 // 自启动下标
#define MODULE_SYSTEM_SAFETY_INDEX 4 // 系统安全下标

// 各模块名称
#define MODULE_HOMEPAGE_NAME "homepage" // 主页名称
#define MODULE_VIRUS_SCAN_NAME "virusscan" // 病毒查杀名称
#define MODULE_SAFETY_PROTECT_NAME "safetyprotection" // 安全防护名称
#define MODULE_DISK_CLEANER_NAME "diskcleaner" // 磁盘清理名称
#define MODULE_SECURITY_TOOLS_NAME "securitytools" // 安全工具名称

// system safety modules' index
#define LOGIN_SECURITY_INDEX 0 // login security index
#define SCREEN_SECURITY_INDEX 1 // screen security index
#define UPDATE_POLICY_INDEX 2 // updating policy index
#define USB_CONNECTION_INDEX 3 // usb connection index

// 系统工具各模块索引
#define DATA_USAGE_INDEX 1 // 流量详情索引
#define STARTUP_CONTROL_INDEX 2 // 自启动管理索引
#define NET_CONTROL_INDEX 3 // 网络权限控制索引
#define USB_CONN_INDEX 4 // usb管控索引
#define LOGIN_SAFETY_INDEX 5 // 登录安全索引

// 系统工具各模块名称
#define DATA_USAGE_NAME "data-usage" // 流量详情名称
#define STARTUP_CONTROL_NAME "startup-control" // 自启动管理名称
#define NET_CONTROL_NAME "network-control" // 网络权限控制名称
#define USB_CONN_NAME "usb-control" // usb控制名称
#define LOGIN_SAFETY_NAME "login-safety" // 登录安全名称

// 流量监控配置文件路径
#define NET_MONITOR_CONFIG_PATH "/opt/deepin-defender/file/config/net-monitor.ini" // 回收站路径
// 应用流量参数
struct NetFlowData {
    QString sPkgName;
    QString sExecPath;
    QString sTimeyMdh;
    double dDownloaded = 0;
    double dUploaded = 0;
    double dTotal = 0; //单个APP总流量
};
Q_DECLARE_METATYPE(NetFlowData);
// 时间跨度类型
enum TimeRangeType { None = 0,
                     Today,
                     Yesterday,
                     ThisMonth,
                     LastMonth };

// 联网状态
#define NET_STATUS_ITEMASK 0 // 询问
#define NET_STATUS_ITEMALLOW 1 // 允许
#define NET_STATUS_ITEMDISALLOW 2 // 禁止

// 垃圾清理
#define CLEANER_SYSTEM_TRASH_PATH "/.local/share/Trash" // 回收站路径
#define CLEANER_SYSTEM_LOG_PATH "/var/log" // 系统日志
#define CLEANER_SYSTEM_CACHE_APT_PATH "/var/cache/apt/archives" // 系统缓存
// 1040集成,接口方修改了JSON内容
#define CLEANER_SYSTEM_CACHE_LASTORE_PATH "/var/cache/lastore/archives" // 系统缓存
#define CLEANER_SYSTEM_CACHE_ARRAY_NAME "files"
#define CLEANER_SYSTEM_CACHE_DEB_NAME "name"
#define CLEANER_SYSTEM_TEMP_PATH "/tmp"
#define CLEANER_SYSTEM_HISTORY_PATH "/.bash_history"

#define CLEANER_APP_JSON_NAME "appArray"
#define CLEANER_APP_COM_NAME "comName"
#define CLEANER_APP_NAME "appName"
#define CLEANER_APP_TIP "appTip"
#define CLEANER_APP_CACHE_PATH "appCachePath"
#define CLEANER_APP_CONFIG_PATH "appConfigPath"
#define CLEANER_APP_INSTALL_FLAG "appInstallFlag"
#define CLEANER_APP_WHITE_LIST "appWhiteList"
#define CLEANER_APP_BLACK_LIST "appBlackList"
#define CLEANER_BROWSER_JSON_NAME "browserCookiesArray"
#define CLEANER_BROWSER_COOKIES_PATH "cookiesPath"

#define MIN_SCAN 20 // 最小扫描
#define MAX_SCAN 5120 // 最大扫描
#define TIP_DIALOG_WIDTH 380 // 提示dialog宽度
#define TIP_DIALOG_HEIGHT 145 // 提示dialog高度
#define DEFENDER_VRIUS_ADDRESS "https://defender.chinauos.com" // 安全中心病毒地址

// 安全日志类型
#define SECURITY_LOG_TYPE_ALL 0
#define SECURITY_LOG_TYPE_HOME 1
#define SECURITY_LOG_TYPE_ANTIAV 2
#define SECURITY_LOG_TYPE_PROTECTION 3
#define SECURITY_LOG_TYPE_CLEANER 4
#define SECURITY_LOG_TYPE_TOOL 5
#define SECURITY_LOG_TYPE_BASIC 6
#define SECURITY_LOG_TYPE_SAFY 7

// 日期
#define LAST_DATE_NOW 0
#define LAST_DATE_THREE 1
#define LAST_DATE_SEVEN 2
#define LAST_DATE_MONTH 3

// 缓存数据地址
#define DEFENDER_LOCAL_CACHE_DB_PATH "/usr/share/deepin-defender/localcache.db"
// 数据库相关
#define DEFENDER_DATA_DIR_PATH "/usr/share/deepin-defender/" // 数据库路径
#define LOCAL_CACHE_DB_NAME "localcache.db" // 数据库路径

// 公共dbus接口类
#define DBUS_PROPERTY_INVOKER_NAME "dbusPropertyInvoker" // 公共dbus接口属性获取识别

// 系统类型
const DSysInfo::UosType SystemType = DSysInfo::uosType();
const DSysInfo::UosEdition SystemEditionType = DSysInfo::uosEditionType();

// usb管控相关结构体和宏定义
#define SUBSYSTEM "usb"
// 全部允许/允许白名单/全部禁止
#define USB_LIMIT_MODEL_ALL "0"
#define USB_LIMIT_MODEL_PARTY "1"
#define USB_LIMIT_MODEL_ZERO "2"
// 配置目录
#define USB_CONTROL_CONFIG_DIR "/usr/share/deepin-defender/config/"
// 配置文件名
#define CONFIG_FILE_NAME "usbconnectionconfig.ini"
// 连接记录备份文件名
#define USB_CONNECTION_LOG_BAK_FILE_NAME "usbconnection.log.bak"
// 连接记录文件名
#define USB_CONNECTION_LOG_FILE_NAME "usbconnection.log"
// 白名单备份文件名
#define USB_WHITELIST_BAK_FILE_NAME "usbconnectionwhitelist.list.bak"
// 白名单文件名
#define USB_WHITELIST_FILE_NAME "usbconnectionwhitelist.list"

struct UsbDeviceInfo {
    QString sProduct; // 产品型号
    QString sSerial; // 序列号
    QString sIdVendor; // 生产商ID
    QString sIdProduct; // 产品ID
    QString sInterfaceClass; // 接口类型
    QString sBusnum; // bus总线序号
    QString sDevNum; // 设备序号
    QString sSysPath; // /sys/ 下路径
    QString sDevPath; // 设备device路径，与sSysPath一致
    QString sSubSystem; // 子系统
    QString sDevType; // 设备类型
    QString sDevNode; // 设备节点
    QString sLabel; // 分区标签
    bool isMtp; // 是否为mtp
    bool isPtp; // 是否为相册设备
    QString sParentSize; // 父设备大小
    QString sSize; // 分区大小
    UsbDeviceInfo()
    {
        isMtp = false;
        isPtp = false;
    }
};

struct BlkInfo {
    QString type;
    QString parentDevPath;
    QString devPath;
    QString mountPoint;
    QString canRemove;
};

//usb连接日志信息结构
struct UsbConnectionInfo {
    QString sName;
    QString sDatetime;
    QString sStatus;
    QString sId;
    QString sSerial;
    QString sVendorID;
    QString sProductID;
    QString sParentSize; // 父设备大小
    QString sSize; // 分区大小
};

namespace Utils {
//字符换行
QString wordwrapText(const QFontMetrics &font, const QString &text, int nLabelSize);
// 根据流量数据大小改变单位
QString changeFlowValueUnit(double input, int prec);
// 根据网速大小改变单位
QString changeFlowSpeedValueUnit(double input, int prec);

// 改变文件大小单位，以byte为基准
QString formatBytes(double input, int prec);
// 从数据库中通过文件路径获得包名
QString getPkgNameByFilePathFromDb(const QString &sExecPath, const QString &connectionName);
// 检查并转换QDbusVariant为QVariant
QVariant checkAndTransQDbusVarIntoQVar(const QVariant &var);
} // namespace Utils
