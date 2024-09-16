// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "lunardbusinterface.h"
#include "commondef.h"

#include <QDebug>

LunarDBusInterface::LunarDBusInterface(QObject* parent)
    : QObject(parent)
    , m_huangLiInter(new QDBusInterface("com.deepin.dataserver.Calendar",
          "/com/deepin/dataserver/Calendar/HuangLi",
          "com.deepin.dataserver.Calendar.HuangLi",
          QDBusConnection::sessionBus(),
          parent))
{
}

/**
 * @brief HuangLiHelper::getHuangLiDay
 * 按天获取黄历信息
 * @param year
 * @param month
 * @param day
 */
bool LunarDBusInterface::huangLiDay(quint32 year,
    quint32 month,
    quint32 day,
    CaHuangLiDayInfo& info)
{
    QDBusPendingReply<QString> reply = m_huangLiInter->call("getHuangLiDay", QVariant(year), QVariant(month), QVariant(day));

    if (reply.isError()) {
        qWarning() << "Call DBus error: " << reply.error().message();
        return false;
    }

    QString json = reply.argumentAt<0>();
    bool isVoild;
    info.strJsonToInfo(json, isVoild);
    return isVoild;
}
