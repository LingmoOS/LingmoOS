// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../../deepin-defender/src/window/modules/common/common.h"

#include <QObject>
#include <polkit-qt5-1/PolkitQt1/Authority>

using namespace PolkitQt1;

class PolicyKitHelper : public QObject
{
    Q_OBJECT
public:
    explicit PolicyKitHelper();
    ~PolicyKitHelper();

Q_SIGNALS:
    // 通知去改变连接记录开关
    void notifyChangeIsbSaveRecord();
    // 通知去改变usb限制模式
    void notifyChangeLimitationModel(const QString &limitModel);
    // 通知去添加到白名单
    void notifyAddUsbConnectionWhiteLst(const UsbConnectionInfo &info);
    // 通知去清除usb连接记录
    void notifyClearUsbConnectionLog();
    // 权限校验完成
    void checkAuthorizationFinished();

public Q_SLOTS:
    // 确认是否改变连接记录开关
    void confirmChangeIsbSaveRecord();
    // 确认选择usb限制模式
    void confirmSelectLimitationModel(const QString &limitModel);
    // 确认添加到白名单
    void confirmAddUsbConnectionWhiteLst(const UsbConnectionInfo &info);
    // 确认是否清除usb连接记录
    void confirmClearUsbConnectionLog();

private:
    bool checkAuthorization(const QString &actionId, qint64 applicationPid);
};
