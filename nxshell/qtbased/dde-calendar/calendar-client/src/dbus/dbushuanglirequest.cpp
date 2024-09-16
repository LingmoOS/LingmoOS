// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbushuanglirequest.h"

#include "commondef.h"

#include <QDebug>

DbusHuangLiRequest::DbusHuangLiRequest(QObject *parent)
    : DbusRequestBase("/com/deepin/dataserver/Calendar/HuangLi",
                      "com.deepin.dataserver.Calendar.HuangLi",
                      QDBusConnection::sessionBus(),
                      parent)
{
}

/**
 * @brief DbusHuangLiRequest::getFestivalMonth
 * 按月获取节假日信息
 * @param year
 * @param month
 */
bool DbusHuangLiRequest::getFestivalMonth(quint32 year, quint32 month, FestivalInfo &festivalInfo)
{
    QDBusPendingReply<QString> reply = call("getFestivalMonth", QVariant(year), QVariant(month));

    if (reply.isError()) {
        qCWarning(ClientLogger) << reply.error().message();
        return false;
    }

    QString json = reply.argumentAt<0>();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(json.toLocal8Bit(), &json_error));

    if (json_error.error != QJsonParseError::NoError) {
        return false;
    }
    // 解析数据
    QJsonArray rootarry = jsonDoc.array();
    for (int i = 0; i < rootarry.size(); i++) {
        QJsonObject subObj = rootarry.at(i).toObject();
        // 因为是预先定义好的JSON数据格式，所以这里可以这样读取
        if (subObj.contains("id")) {
            festivalInfo.ID = subObj.value("id").toString();
        }
        if (subObj.contains("name")) {
            festivalInfo.FestivalName = subObj.value("name").toString();
        }
        if (subObj.contains("description")) {
            festivalInfo.description = subObj.value("description").toString();
        }
        if (subObj.contains("rest")) {
            festivalInfo.Rest = subObj.value("rest").toString();
        }
        if (subObj.contains("month")) {
            festivalInfo.month = subObj.value("month").toInt();
        }
        if (subObj.contains("list")) {
            QJsonArray sublistArray = subObj.value("list").toArray();
            for (int j = 0; j < sublistArray.size(); j++) {
                QJsonObject hsubObj = sublistArray.at(j).toObject();
                HolidayInfo dayinfo;
                if (hsubObj.contains("status")) {
                    dayinfo.status = static_cast<char>(hsubObj.value("status").toInt());
                }
                if (hsubObj.contains("date")) {
                    dayinfo.date = QDate::fromString(hsubObj.value("date").toString(), "yyyy-M-d");
                }
                festivalInfo.listHoliday.append(dayinfo);
            }
        }
        festivalInfo.year = static_cast<int>(year);
    }
    return true;
}

/**
 * @brief DbusHuangLiRequest::getHuangLiDay
 * 按天获取黄历信息
 * @param year
 * @param month
 * @param day
 */
bool DbusHuangLiRequest::getHuangLiDay(quint32 year,
                                       quint32 month,
                                       quint32 day,
                                       CaHuangLiDayInfo &info)
{
    QDBusPendingReply<QString> reply =
        call("getHuangLiDay", QVariant(year), QVariant(month), QVariant(day));

    if (reply.isError()) {
        qCWarning(ClientLogger) << reply.error().message();
        return false;
    }

    QString json = reply.argumentAt<0>();
    bool isVoild;
    info.strJsonToInfo(json, isVoild);
    return isVoild;
}

/**
 * @brief DbusHuangLiRequest::getHuangLiMonth
 * 按月获取黄历信息
 * @param year
 * @param month
 * @param fill
 */
bool DbusHuangLiRequest::getHuangLiMonth(quint32 year,
                                         quint32 month,
                                         bool fill,
                                         CaHuangLiMonthInfo &info)
{
    QDBusPendingReply<QString> reply =
        call("getHuangLiMonth", QVariant(year), QVariant(month), QVariant(fill));
    if (reply.isError()) {
        qCWarning(ClientLogger) << reply.error().message();
        return false;
    }
    QString json = reply.argumentAt<0>();

    bool infoIsVaild;
    info.strJsonToInfo(json, infoIsVaild);
    return infoIsVaild;
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
        qCWarning(ClientLogger) << call->reply().member() << call->error().message();
        return;
    }
    call->deleteLater();
}
