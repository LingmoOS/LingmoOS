// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbusdatastruct.h"

#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>

void CaLunarDayInfo::registerMetaType()
{
    qRegisterMetaType<CaLunarDayInfo>();
    qDBusRegisterMetaType<CaLunarDayInfo>();
}

QDebug operator<<(QDebug argument, const CaLunarDayInfo &what)
{
    argument << what.mGanZhiYear << what.mGanZhiMonth << what.mGanZhiDay;
    argument << what.mLunarMonthName << what.mLunarDayName;
    argument << what.mLunarLeapMonth;
    argument << what.mZodiac << what.mTerm;
    argument << what.mSolarFestival << what.mLunarFestival;
    argument << what.mWorktime;

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const CaLunarDayInfo &what)
{
    argument.beginStructure();
    argument << what.mGanZhiYear << what.mGanZhiMonth << what.mGanZhiDay;
    argument << what.mLunarMonthName << what.mLunarDayName;
    argument << what.mLunarLeapMonth;
    argument << what.mZodiac << what.mTerm;
    argument << what.mSolarFestival << what.mLunarFestival;
    argument << what.mWorktime;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, CaLunarDayInfo &what)
{
    argument.beginStructure();
    argument >> what.mGanZhiYear >> what.mGanZhiMonth >> what.mGanZhiDay;
    argument >> what.mLunarMonthName >> what.mLunarDayName;
    argument >> what.mLunarLeapMonth;
    argument >> what.mZodiac >> what.mTerm;
    argument >> what.mSolarFestival >> what.mLunarFestival;
    argument >> what.mWorktime;
    argument.endStructure();

    return argument;
}

void CaLunarMonthInfo::registerMetaType()
{
    qRegisterMetaType<CaLunarMonthInfo>();
    qDBusRegisterMetaType<CaLunarMonthInfo>();
}

QDebug operator<<(QDebug argument, const CaLunarMonthInfo &what)
{
    argument << what.mFirstDayWeek << what.mDays;
    argument << what.mCaLunarDayInfo;

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const CaLunarMonthInfo &what)
{
    argument.beginStructure();
    argument << what.mFirstDayWeek << what.mDays;
    argument << what.mCaLunarDayInfo;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, CaLunarMonthInfo &what)
{
    argument.beginStructure();
    argument >> what.mFirstDayWeek >> what.mDays;
    argument >> what.mCaLunarDayInfo;
    argument.endStructure();

    return argument;
}

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

void CaHuangLiMonthInfo::registerMetaType()
{
    qRegisterMetaType<CaHuangLiMonthInfo>();
    qDBusRegisterMetaType<CaHuangLiMonthInfo>();
}

QString CaHuangLiMonthInfo::toJson()
{
    QJsonArray daysarr;
    QJsonDocument doc;
    QJsonObject obj;
    obj.insert("Days", mDays);
    obj.insert("FirstDayWeek", mFirstDayWeek);
    foreach (CaHuangLiDayInfo dayinfo, mCaLunarDayInfo) {
        QJsonObject dayobj;
        dayobj.insert("Suit", dayinfo.mSuit);
        dayobj.insert("Avoid", dayinfo.mAvoid);
        dayobj.insert("Worktime", dayinfo.mWorktime);
        dayobj.insert("LunarFestival", dayinfo.mLunarFestival);
        dayobj.insert("SolarFestival", dayinfo.mSolarFestival);
        dayobj.insert("Term", dayinfo.mTerm);
        dayobj.insert("Zodiac", dayinfo.mZodiac);
        dayobj.insert("LunarLeapMonth", dayinfo.mLunarLeapMonth);
        dayobj.insert("LunarDayName", dayinfo.mLunarDayName);
        dayobj.insert("LunarMonthName", dayinfo.mLunarMonthName);
        dayobj.insert("GanZhiDay", dayinfo.mGanZhiDay);
        dayobj.insert("GanZhiMonth", dayinfo.mGanZhiMonth);
        dayobj.insert("GanZhiYear", dayinfo.mGanZhiYear);
        daysarr.append(dayobj);
    }
    obj.insert("Datas", daysarr);
    doc.setObject(obj);
    QString strJson = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    return strJson;
}

void CaHuangLiMonthInfo::strJsonToInfo(const QString &strJson, bool &isVaild)
{
    isVaild = true;
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(strJson.toLocal8Bit(), &json_error));

    if (json_error.error != QJsonParseError::NoError) {
        isVaild = false;
        return;
    }

    QJsonObject rootObj = jsonDoc.object();

    //因为是预先定义好的JSON数据格式，所以这里可以这样读取
    if (rootObj.contains("Days")) {
        this->mDays = rootObj.value("Days").toInt();
    }
    if (rootObj.contains("FirstDayWeek")) {
        this->mFirstDayWeek = rootObj.value("FirstDayWeek").toInt();
    }
    if (rootObj.contains("Datas")) {
        QJsonArray subArray = rootObj.value("Datas").toArray();
        for (int i = 0; i < subArray.size(); i++) {
            QJsonObject subObj = subArray.at(i).toObject();
            CaHuangLiDayInfo huangliday;
            huangliday.jsonObjectToInfo(subObj);
            this->mCaLunarDayInfo.append(huangliday);
        }
    }
}

void CaHuangLiMonthInfo::clear()
{
    this->mDays = 0;
    this->mCaLunarDayInfo.clear();
}

QDebug operator<<(QDebug argument, const CaHuangLiMonthInfo &what)
{
    argument << what.mDays << what.mFirstDayWeek;
    argument << what.mCaLunarDayInfo;

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const CaHuangLiMonthInfo &what)
{
    argument.beginStructure();
    argument << what.mDays << what.mFirstDayWeek;
    argument << what.mCaLunarDayInfo;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, CaHuangLiMonthInfo &what)
{
    argument.beginStructure();
    argument >> what.mDays >> what.mFirstDayWeek;
    argument >> what.mCaLunarDayInfo;
    argument.endStructure();

    return argument;
}
