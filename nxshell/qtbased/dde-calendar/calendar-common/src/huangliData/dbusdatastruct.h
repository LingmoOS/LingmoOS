// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBUSDATASTRUCT_H
#define DBUSDATASTRUCT_H

#include <QtCore/QDebug>
#include <QtDBus/QtDBus>

class CaLunarDayInfo
{
public:
    CaLunarDayInfo()
        : mGanZhiYear("")
        , mGanZhiMonth("")
        , mGanZhiDay("")
        , mLunarMonthName("")
        , mLunarDayName("")
        , mLunarLeapMonth(0)
        , mZodiac("")
        , mTerm("")
        , mSolarFestival("")
        , mLunarFestival("")
        , mWorktime(0)
    {
    }

    static void registerMetaType();

    friend QDebug operator<<(QDebug argument, const CaLunarDayInfo &what);
    friend QDBusArgument &operator<<(QDBusArgument &argument, const CaLunarDayInfo &what);
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, CaLunarDayInfo &what);

public:
    QString mGanZhiYear; // 农历年的干支
    QString mGanZhiMonth; // 农历月的干支
    QString mGanZhiDay; // 农历日的干支
    QString mLunarMonthName; // 农历月名
    QString mLunarDayName; // 农历日名
    qint32 mLunarLeapMonth; // 未使用
    QString mZodiac; // 农历年的生肖
    QString mTerm; // 农历节气
    QString mSolarFestival; // 公历节日
    QString mLunarFestival; // 农历节日
    qint32 mWorktime; // 未使用
};

class CaLunarMonthInfo
{
public:
    CaLunarMonthInfo()
        : mFirstDayWeek(0)
        , mDays(0)
    {
    }

    static void registerMetaType();

    friend QDebug operator<<(QDebug argument, const CaLunarMonthInfo &what);
    friend QDBusArgument &operator<<(QDBusArgument &argument, const CaLunarMonthInfo &what);
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, CaLunarMonthInfo &what);

public:
    qint32 mFirstDayWeek;
    qint32 mDays;
    QList<CaLunarDayInfo> mCaLunarDayInfo;
};

class CaHuangLiDayInfo
{
public:
    CaHuangLiDayInfo()
        : mGanZhiYear("")
        , mGanZhiMonth("")
        , mGanZhiDay("")
        , mLunarMonthName("")
        , mLunarDayName("")
        , mLunarLeapMonth(0)
        , mZodiac("")
        , mTerm("")
        , mSolarFestival("")
        , mLunarFestival("")
        , mWorktime(0)
        , mSuit("")
        , mAvoid("")
    {
    }
    static void registerMetaType();
    friend QDebug operator<<(QDebug argument, const CaHuangLiDayInfo &what);
    friend QDBusArgument &operator<<(QDBusArgument &argument, const CaHuangLiDayInfo &what);
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, CaHuangLiDayInfo &what);
    QString toJson();
    void strJsonToInfo(const QString &strJson, bool &isVaild);
    void jsonObjectToInfo(const QJsonObject &jsonObject);
public:
    QString mGanZhiYear; //年干支
    QString mGanZhiMonth; //月干支
    QString mGanZhiDay; //日干支
    QString mLunarMonthName; //农历月名称
    QString mLunarDayName; //农历日名称
    qint32 mLunarLeapMonth; //闰月
    QString mZodiac; //生肖
    QString mTerm; //农历节气
    QString mSolarFestival; //阳历节日
    QString mLunarFestival; //农历节日
    qint32 mWorktime; //未使用
    QString mSuit; //黄历宜
    QString mAvoid; //黄历忌
};

class CaHuangLiMonthInfo
{
public:
    CaHuangLiMonthInfo()
        : mFirstDayWeek(0)
        , mDays(0)
    {
    }
    static void registerMetaType();

    friend QDebug operator<<(QDebug argument, const CaHuangLiMonthInfo &what);
    friend QDBusArgument &operator<<(QDBusArgument &argument, const CaHuangLiMonthInfo &what);
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, CaHuangLiMonthInfo &what);
    QString toJson();
    void strJsonToInfo(const QString &strJson, bool &isVaild);
    void clear();
public:
    qint32 mFirstDayWeek; //第一天所在周
    qint32 mDays; //每月天数
    QVector<CaHuangLiDayInfo> mCaLunarDayInfo;
};

Q_DECLARE_METATYPE(CaLunarDayInfo)
Q_DECLARE_METATYPE(CaLunarMonthInfo)
Q_DECLARE_METATYPE(CaHuangLiDayInfo)
Q_DECLARE_METATYPE(CaHuangLiMonthInfo)

#endif // DBUSDATASTRUCT_H
