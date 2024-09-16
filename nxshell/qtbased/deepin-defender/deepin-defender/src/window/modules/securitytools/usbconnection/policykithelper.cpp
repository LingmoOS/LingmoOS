// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "policykithelper.h"

PolicyKitHelper::PolicyKitHelper()
{
}

PolicyKitHelper::~PolicyKitHelper()
{
}

// 确认是否改变连接记录开关
void PolicyKitHelper::confirmChangeIsbSaveRecord()
{
    bool isbChange = false;
    // 权限验证
    isbChange = checkAuthorization("com.deepin.deepin-defender.checkAuthentication", getpid());
    if (isbChange) {
        Q_EMIT notifyChangeIsbSaveRecord();
    }
    // 权限校验完成
    Q_EMIT checkAuthorizationFinished();
}

// 确认选择usb限制模式
void PolicyKitHelper::confirmSelectLimitationModel(const QString &limitModel)
{
    bool isbChange = false;
    // 权限验证
    isbChange = checkAuthorization("com.deepin.deepin-defender.checkAuthentication", getpid());
    if (isbChange) {
        Q_EMIT notifyChangeLimitationModel(limitModel);
    }
    // 权限校验完成
    Q_EMIT checkAuthorizationFinished();
}

// 确认添加到白名单
void PolicyKitHelper::confirmAddUsbConnectionWhiteLst(const UsbConnectionInfo &info)
{
    bool isbChange = false;
    // 权限验证
    isbChange = checkAuthorization("com.deepin.deepin-defender.checkAuthentication", getpid());
    if (isbChange) {
        Q_EMIT notifyAddUsbConnectionWhiteLst(info);
    }
    // 权限校验完成
    Q_EMIT checkAuthorizationFinished();
}

// 确认是否清除usb连接记录
void PolicyKitHelper::confirmClearUsbConnectionLog()
{
    bool clear = false;
    // 权限验证
    clear = checkAuthorization("com.deepin.deepin-defender.checkAuthentication", getpid());
    if (clear) {
        Q_EMIT notifyClearUsbConnectionLog();
    }
    // 权限校验完成
    Q_EMIT checkAuthorizationFinished();
}

bool PolicyKitHelper::checkAuthorization(const QString &actionId, qint64 applicationPid)
{
    bool ret;
    Authority::Result result;
    result = Authority::instance()->checkAuthorizationSync(actionId, UnixProcessSubject(applicationPid), /// 第一个参数是需要验证的action，和规则文件写的保持一致
                                                           Authority::AllowUserInteraction);
    if (result == Authority::Yes) {
        ret = true;
    } else {
        ret = false;
    }

    // 权限校验完成
    Q_EMIT checkAuthorizationFinished();
    return ret;
}
