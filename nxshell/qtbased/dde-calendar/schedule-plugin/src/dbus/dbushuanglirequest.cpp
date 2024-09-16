// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbushuanglirequest.h"
#include "commondef.h"
#include <QDebug>

DbusHuangLiRequest::DbusHuangLiRequest(QObject *parent)
    : DbusRequestBase("/com/deepin/dataserver/Calendar/HuangLi", "com.deepin.dataserver.Calendar.HuangLi", QDBusConnection::sessionBus(), parent)
{
}

/**
 * @brief DbusHuangLiRequest::getFestivalMonth
 * 按月获取节假日信息
 * @param year
 * @param month
 */
void DbusHuangLiRequest::getFestivalMonth(quint32 year, quint32 month)
{
    asyncCall("getFestivalMonth", QVariant(year), QVariant(month));
}

/**
 * @brief DbusHuangLiRequest::getHuangLiDay
 * 按天获取黄历信息
 * @param year
 * @param month
 * @param day
 */
void DbusHuangLiRequest::getHuangLiDay(quint32 year, quint32 month, quint32 day)
{
    asyncCall("getHuangLiDay", QVariant(year), QVariant(month), QVariant(day));
}

/**
 * @brief DbusHuangLiRequest::getHuangLiMonth
 * 按月获取黄历信息
 * @param year
 * @param month
 * @param fill
 */
void DbusHuangLiRequest::getHuangLiMonth(quint32 year, quint32 month, bool fill)
{
    asyncCall("getHuangLiMonth", QVariant(year), QVariant(month), QVariant(fill));
}

/**
 * @brief DbusHuangLiRequest::getLunarInfoBySolar
 * 获取农历信息
 * @param year
 * @param month
 * @param day
 */
void DbusHuangLiRequest::getLunarInfoBySolar(quint32 year, quint32 month, quint32 day)
{
    asyncCall("getLunarInfoBySolar", QVariant(year), QVariant(month), QVariant(day));
}

/**
 * @brief DbusHuangLiRequest::getLunarMonthCalendar
 * 获取农历月日程
 * @param year
 * @param month
 * @param fill
 */
void DbusHuangLiRequest::getLunarMonthCalendar(quint32 year, quint32 month, bool fill)
{
    asyncCall("getLunarMonthCalendar", QVariant(year), QVariant(month), QVariant(fill));
}

void DbusHuangLiRequest::slotCallFinished(CDBusPendingCallWatcher *call)
{
    if (call->isError()) {
        qCWarning(CommonLogger) << call->reply().member() << call->error().message();
        return;
    }

    if (call->getmember() == "getFestivalMonth") {
        QDBusPendingReply<QString> reply = *call;
        QString str = reply.argumentAt<0>();
    }
    call->deleteLater();
}
