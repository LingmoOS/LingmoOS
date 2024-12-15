// Copyright (C) 2019 ~ 2022 Lingmo Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "systemmessagehelper.h"

#include <QDBusMessage>
#include <QDBusConnection>

SystemMessageHelper::SystemMessageHelper(QObject *parent)
    : QObject(parent)
{
}

void SystemMessageHelper::createMessageForAVEngineExpiredInDays(const QString &engineName, int days)
{
    // 提示时间为0-6天
    // 第0天是否过期在调用处判断
    if (engineName.isEmpty() || days < 0 || days >= 7) {
        return;
    }

    QString notifyInfo = tr("The %1 antivirus engine service will expire in %2 days. After the expiration, files in the quarantine list cannot be restored. Please restore them in time if you want.");
    notifyInfo = notifyInfo.arg(engineName).arg(days);

    // 跳转到安全中心的dbus调用
    QString jumpToAVWidget = QString("qdbus,com.lingmo.pc.manager,"
                                     "/com/lingmo/pc/manager,"
                                     "com.lingmo.pc.manager.ShowModule,"
                                     "virusscan");

    QDBusMessage m = QDBusMessage::createMethodCall("com.lingmo.ocean.Notification",
                                                    "/com/lingmo/ocean/Notification",
                                                    "com.lingmo.ocean.Notification",
                                                    "Notify");
    QStringList action;
    action << "_open1" << tr("View", "button"); //添加按钮
    QVariantMap inform; //按钮的点击操作
    inform.insert(QString("x-lingmo-action-_open1"), jumpToAVWidget);

    m << QString("lingmo-defender")
      << uint(0)
      << QString("lingmo-defender")
      << QString("") << notifyInfo
      << action
      << inform
      << int(10 * 1000); // 停留10秒
    QDBusMessage respone = QDBusConnection::sessionBus().call(m);
}
