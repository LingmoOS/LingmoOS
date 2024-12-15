// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WAKEUPUTILS_H
#define WAKEUPUTILS_H

#include "ethtool-copy.h"

#include <QString>

#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>

class WakeupUtils
{
    /**
     * @brief The EthStatus enum
     * ioctl操作网络设备的返回值定义
     */
    enum EthStatus {
        ES_SOCKET_FAILED,           // socket 返回错误
        ES_IOCTL_ERROR,             // ioctl 返回错误
        ES_NOT_SUPPORT_WAKE_ON,     // 网卡不支持唤醒功能
        ES_WAKE_ON_OPEN,            // 当前唤醒功能已经开启
        ES_WAKE_ON_CLOSE,           // 当前唤醒功能已经关闭
        ES_WAKE_ON_UNKNOW           // 位置错误
    };

public:
    WakeupUtils();

    /**
     * @brief updateWakeupDeviceInfo
     * @param info
     */
    static void updateWakeupDeviceInfo(const QString &info);

    /**
     * @brief wakeupPath : get wakeup path by sys path
     * @param syspath : sys path
     * @param wakeuppath : wakeup path
     * @return
     */
    static bool wakeupPath(const QString &syspath, QString &wakeuppath);

    /**
     * @brief writeWakeupFile : write wakeup file
     * @param path : wakeup file path
     * @param wakeup : enable or disable
     * @return
     */
    static bool writeWakeupFile(const QString &path, bool wakeup);

    /**
     * @brief wakeOnLanIsOpen 判断网卡的唤醒功能是否开启
     * @param logicalName 网卡的逻辑名称
     * @return 返回定义值
     */
    static EthStatus wakeOnLanIsOpen(const QString &logicalName);

    /**
     * @brief setWakeOnLan 开启或者关闭网卡的远程唤醒功能
     * @param logicalName 网卡的逻辑名称
     * @param open 开启或者关闭
     * @return 返回设置状态
     */
    static bool setWakeOnLan(const QString &logicalName, bool open);

private:
    /**
     * @brief getMapInfo 解析usb信息
     * @param item
     * @param mapInfo
     * @return
     */
    static bool getMapInfo(const QString &item, QMap<QString, QString> &mapInfo);

    /**
     * @brief getPS2Syspath 获取ps2鼠标键盘的syspath
     * @param dfs : Device Files 属性
     * @return
     */
    static QString getPS2Syspath(const QString &dfs);
};

#endif // WAKEUPUTILS_H
