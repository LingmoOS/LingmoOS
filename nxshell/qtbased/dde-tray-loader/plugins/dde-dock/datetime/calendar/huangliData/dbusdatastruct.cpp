// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbusdatastruct.h"

#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>

void CaHuangLiDayInfo::registerMetaType()
{
    qRegisterMetaType<CaHuangLiDayInfo>();
    qDBusRegisterMetaType<CaHuangLiDayInfo>();
}

QString CaHuangLiDayInfo::toJson()
{
    QJsonDocument doc;
    QJsonObject obj;

    obj.insert("Suit", mSuit);
    obj.insert("Avoid", mAvoid);
    obj.insert("Worktime", mWorktime);
    obj.insert("LunarFestival", mLunarFestival);
    obj.insert("SolarFestival", mSolarFestival);
    obj.insert("Term", mTerm);
    obj.insert("Zodiac", mZodiac);
    obj.insert("LunarLeapMonth", mLunarLeapMonth);
    obj.insert("LunarDayName", mLunarDayName);
    obj.insert("LunarMonthName", mLunarMonthName);
    obj.insert("GanZhiDay", mGanZhiDay);
    obj.insert("GanZhiMonth", mGanZhiMonth);
    obj.insert("GanZhiYear", mGanZhiYear);

    doc.setObject(obj);
    QString strJson = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    return strJson;
}

void CaHuangLiDayInfo::strJsonToInfo(const QString &strJson, bool &isVaild)
{
    isVaild = true;
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(strJson.toLocal8Bit(), &json_error));
    if (json_error.error != QJsonParseError::NoError) {
        isVaild = false;
        return ;
    }
    QJsonObject rootObj = jsonDoc.object();
    jsonObjectToInfo(rootObj);
}

void CaHuangLiDayInfo::jsonObjectToInfo(const QJsonObject &jsonObject)
{
    //因为是预先定义好的JSON数据格式，所以这里可以这样读取int
    if (jsonObject.contains("Suit")) {
        this->mSuit = jsonObject.value("Suit").toString();
    }
    if (jsonObject.contains("Avoid")) {
        this->mAvoid = jsonObject.value("Avoid").toString();
    }
    if (jsonObject.contains("Worktime")) {
        this->mWorktime = jsonObject.value("Worktime").toInt();
    }
    if (jsonObject.contains("LunarFestival")) {
        this->mLunarFestival = jsonObject.value("LunarFestival").toString();
    }
    if (jsonObject.contains("SolarFestival")) {
        this->mSolarFestival = jsonObject.value("SolarFestival").toString();
    }
    if (jsonObject.contains("Term")) {
        this->mTerm = jsonObject.value("Term").toString();
    }
    if (jsonObject.contains("Zodiac")) {
        this->mZodiac = jsonObject.value("Zodiac").toString();
    }
    if (jsonObject.contains("LunarLeapMonth")) {
        this->mLunarLeapMonth = jsonObject.value("LunarLeapMonth").toInt();
    }
    if (jsonObject.contains("LunarDayName")) {
        this->mLunarDayName = jsonObject.value("LunarDayName").toString();
    }
    if (jsonObject.contains("LunarMonthName")) {
        this->mLunarMonthName = jsonObject.value("LunarMonthName").toString();
    }
    if (jsonObject.contains("GanZhiDay")) {
        this->mGanZhiDay = jsonObject.value("GanZhiDay").toString();
    }
    if (jsonObject.contains("GanZhiMonth")) {
        this->mGanZhiMonth = jsonObject.value("GanZhiMonth").toString();
    }
    if (jsonObject.contains("GanZhiYear")) {
        this->mGanZhiYear = jsonObject.value("GanZhiYear").toString();
    }
}

QDebug operator<<(QDebug argument, const CaHuangLiDayInfo &what)
{
    argument << what.mSuit << what.mAvoid;
    argument << what.mWorktime;
    argument << what.mLunarFestival << what.mSolarFestival;
    argument << what.mTerm << what.mZodiac;
    argument << what.mLunarLeapMonth;
    argument << what.mLunarDayName << what.mLunarMonthName;
    argument << what.mGanZhiDay << what.mGanZhiMonth << what.mGanZhiYear;
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const CaHuangLiDayInfo &what)
{
    argument.beginStructure();
    argument << what.mSuit << what.mAvoid;
    argument << what.mWorktime;
    argument << what.mLunarFestival << what.mSolarFestival;
    argument << what.mTerm << what.mZodiac;
    argument << what.mLunarLeapMonth;
    argument << what.mLunarDayName << what.mLunarMonthName;
    argument << what.mGanZhiDay << what.mGanZhiMonth << what.mGanZhiYear;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, CaHuangLiDayInfo &what)
{
    argument.beginStructure();
    argument >> what.mSuit >> what.mAvoid;
    argument >> what.mWorktime;
    argument >> what.mLunarFestival >> what.mSolarFestival;
    argument >> what.mTerm >> what.mZodiac;
    argument >> what.mLunarLeapMonth;
    argument >> what.mLunarDayName >> what.mLunarMonthName;
    argument >> what.mGanZhiDay >> what.mGanZhiMonth >> what.mGanZhiYear;
    argument.endStructure();
    return argument;
}
