// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LUNARANDFESTIVAL_H
#define LUNARANDFESTIVAL_H

#include <QVector>
#include <QMap>
#include <QDateTime>

// MonthInfo 保存农历信息
typedef struct _lunarInfo {
    int LunarMonthName = 0; // 农历月名
    int LunarMonthDays = 0; // 本月天数
    double ShuoJD = 0; // 本月朔日时间 北京时间 儒略日
    QDateTime ShuoTime; // 本月朔日时间 北京时间
    bool IsLeap = false; // 是否为闰月
    int LunarYear = 0; // 农历年
    int LunarDay = 0; // 农历日
    int MonthZhi = 0; // 农历日所在的月的地支
    int SolarTerm = 0; // 0~23 二十四节气 ，-1 非节气
    bool operator==(const _lunarInfo &info)
    {
        return this->LunarMonthName == info.LunarMonthName
               && this->LunarYear == info.LunarYear
               && this->LunarDay == info.LunarDay
               && this->IsLeap == info.IsLeap;
    }
} lunarInfo;

typedef struct _day {
    int Year;
    int Month;
    int Day;
} stDay;

typedef struct _LunarDayInfo {
    _LunarDayInfo()
    {
        LunarLeapMonth = 0;
    }
    QString GanZhiYear {}; // 农历年的干支
    QString GanZhiMonth {}; // 农历月的干支
    QString GanZhiDay {}; // 农历日的干支
    QString LunarMonthName {}; // 农历月名
    QString LunarDayName {}; // 农历日名
    int32_t LunarLeapMonth = 0; // 未使用
    QString Zodiac {}; // 农历年的生肖
    QString Term {}; // 农历节气
    QString SolarFestival {}; // 公历节日
    QString LunarFestival {}; // 农历节日
    int32_t Worktime = 0; // 未使用
} stLunarDayInfo;

//农历
typedef struct _LunarMonthInfo {
    qint32 FirstDayWeek = 0;
    qint32 Days = 0;
    QList<stLunarDayInfo> Datas {};
} LunarMonthInfo;

//阳历 公历
typedef struct _SolarMonthInfo {
    qint32 FirstDayWeek = 0;
    qint32 Days = 0;
    QList<stDay> Datas {};
} SolarMonthInfo;

typedef struct DayFestival {
    QDateTime date;
    QStringList Festivals {};
} stDayFestival;

// 十二月名
static QVector<QString> lunarMonthNames = {"正", "二", "三", "四", "五", "六", "七", "八", "九", "十", "冬", "腊"};

static QVector<int> monthDays = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// 农历日名
// 月份分为大月和小月，大月三十天，小月二十九天
static QVector<QString> lunarDayNames = {
    "初一", "初二", "初三", "初四", "初五", "初六", "初七", "初八", "初九", "初十",
    "十一", "十二", "十三", "十四", "十五", "十六", "十七", "十八", "十九", "二十",
    "廿一", "廿二", "廿三", "廿四", "廿五", "廿六", "廿七", "廿八", "廿九", "三十"
};

//农历节日
static QMap<int, QString> lunarFestival {
    {101, "春节"},
    {115, "元宵节"},
    {505, "端午节"},
    {707, "七夕节"},
    {815, "中秋节"},
    {909, "重阳节"},
    {1208, "腊八节"}};

// 十二生肖
static QVector<QString> Animals = {"鼠", "牛", "虎", "兔", "龙", "蛇", "马", "羊", "猴", "鸡", "狗", "猪"};
// 天干
static QVector<QString> TianGan = {"甲", "乙", "丙", "丁", "戊", "己", "庚", "辛", "壬", "癸"};
// 地支
static QVector<QString> DiZhi = {"子", "丑", "寅", "卯", "辰", "巳", "午", "未", "申", "酉", "戌", "亥"};

//二十四节气，中文
static QVector<QString> SolarTermNames = {
    "春分",
    "清明",
    "谷雨",
    "立夏",
    "小满",
    "芒种",
    "夏至",
    "小暑",
    "大暑",
    "立秋",
    "处暑",
    "白露",
    "秋分",
    "寒露",
    "霜降",
    "立冬",
    "小雪",
    "大雪",
    "冬至",
    "小寒",
    "大寒",
    "立春",
    "雨水",
    "惊蛰",
};

//二十四节气
enum solarTerms {
    ChunFen = 0,
    QingMing,
    GuYu,
    LiXia,
    XiaoMan,
    MangZhong,
    XiaZhi,
    XiaoShu,
    DaShu,
    LiQiu,
    ChuShu,
    BaiLu,
    QiuFen,
    HanLu,
    ShuangJiang,
    LiDong,
    XiaoXue,
    DaXue,
    DongZhi,
    XiaoHan,
    DaHan,
    LiChun,
    YuShui,
    JingZhe,
};

typedef struct _Festival {
    QString name {};
    int startYear = 0;
} stSolarFestival;

static QMap<int, QString> solarFestivals = {
    {101, "元旦"},
    {214, "情人节"},
    {305, "学雷锋纪念日"},
    {308, "妇女节"},
    {312, "植树节"},
    {401, "愚人节"},
    {415, "全民国家安全教育日"},
    {501, "劳动节"},
    {504, "青年节"},
    {601, "儿童节"},
    {701, "建党节,香港回归纪念日"},
    {801, "建军节"},
    {903, "抗日战争胜利纪念日"},
    {910, "教师节"},
    {1001, "国庆节"},
    {1213, "南京大屠杀死难者国家公祭日"},
    {1220, "澳门回归纪念"},
    {1224, "平安夜"},
    {1225, "圣诞节"},
    {1226, "毛泽东诞辰纪念"}
};
static QMap<QString, int> solarFestivalStarYear = {
    {"元旦", 1949},
    {"情人节", 0},
    {"学雷锋纪念日", 1963},
    {"妇女节", 1975},
    {"植树节", 1928},
    {"愚人节", 0},
    {"全民国家安全教育日", 2015},
    {"劳动节", 0},
    {"青年节", 1939},
    {"儿童节", 1949},
    {"建党节", 1941},
    {"香港回归纪念日", 1997},
    {"建军节", 1933},
    {"抗日战争胜利纪念日", 2014},
    {"教师节", 1985},
    {"国庆节", 1949},
    {"南京大屠杀死难者国家公祭日", 2014},
    {"澳门回归纪念", 1999},
    {"平安夜", 0},
    {"圣诞节", 0},
    {"毛泽东诞辰纪念", 1893}
};

#endif // ABOUTLUNARANDFESTIVAL_H
