// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "lunarmanager.h"

LunarManager::LunarManager(QObject* parent)
    : QObject(parent)
{
}

LunarManager* LunarManager::instace()
{
    static LunarManager lunarManager;
    return &lunarManager;
}

void LunarManager::asyncRequestLunar(const QDate& date)
{
    // 创建 D-Bus 方法调用消息
    QDBusMessage message = QDBusMessage::createMethodCall(
        "com.lingmo.dataserver.Calendar",
        "/com/lingmo/dataserver/Calendar/HuangLi",
        "com.lingmo.dataserver.Calendar.HuangLi",
        "getHuangLiDay"
    );

    // 设置参数
    message << quint32(date.year()) << quint32(date.month()) << quint32(date.day());

    // 异步调用并创建一个 Watcher
    QDBusPendingCall asyncCall = QDBusConnection::sessionBus().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(asyncCall, this);

    // 连接 Watcher 的信号以处理结果
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher]() {
        QDBusPendingReply<QString> reply = *watcher;
        if (reply.isError()) {
            qWarning() << "Failed to call getHuangLiDay:" << reply.error().message();
        } else {
            QString replyStr = reply.value();
            bool isVaild;
            CaHuangLiDayInfo info;
            info.strJsonToInfo(replyStr, isVaild);
            if (isVaild) {
                emit lunarInfoReady(info);
            } else {
                qWarning()<< "parse huangli json error:" << replyStr;
            }
        }

        watcher->deleteLater();
    });
}
