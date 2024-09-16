// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_calendarhuangli.h"
#include "../third-party_stub/stub.h"
#include "config.h"
#include <QSqlDatabase>
#include <QDebug>

bool stub_OpenHuangliDB(void *obj, const QString &dbpath)
{
    Q_UNUSED(dbpath);
    HuangLiDataBase *o = reinterpret_cast<HuangLiDataBase *>(obj);
    o->m_database = QSqlDatabase::addDatabase("QSQLITE");
    o->m_database.setDatabaseName(HL_DATABASE_DIR);
    return o->m_database.open();
}

test_calendarhuangli::test_calendarhuangli()
{
    Stub stub;
    stub.set(ADDR(HuangLiDataBase, OpenHuangliDatabase), stub_OpenHuangliDB);
    calendarHuangLi = new CalendarHuangLi();
}

test_calendarhuangli::~test_calendarhuangli()
{
    if (calendarHuangLi->m_database->m_database.isOpen()) {
        calendarHuangLi->m_database->m_database.close();
    }
    delete calendarHuangLi;
}

//QString CalendarHuangLi::GetFestivalMonth(quint32 year, quint32 month)
TEST_F(test_calendarhuangli, GetFestivalMonth)
{
    quint32 year = 2020;
    quint32 month = 12;
    QString fesMonth = calendarHuangLi->getFestivalMonth(year, month);
}

//QString CalendarHuangLi::GetHuangLiDay(quint32 year, quint32 month, quint32 day)
TEST_F(test_calendarhuangli, GetHuangLiDay)
{
    //2020年12月13日黄历信息
    const QString huangli_20201213 = "{\"Avoid\":\"嫁娶.祈福.出火.移徙.入宅.\",\"GanZhiDay\":\"庚寅\",\"GanZhiMonth\":\"戊子\","
                                     "\"GanZhiYear\":\"庚子\",\"LunarDayName\":\"廿九\",\"LunarFestival\":\"\",\"LunarLeapMonth\":0,"
                                     "\"LunarMonthName\":\"十月\",\"SolarFestival\":\"南京大屠杀死难者国家公祭日\","
                                     "\"Suit\":\"纳财.开市.交易.立券.会亲友.进人口.经络.祭祀.祈福.安香.出火.求医.治病.修造.动土.拆卸.扫舍.安床."
                                     "栽种.牧养.开生坟.合寿木.入殓.安葬.启攒.\",\"Term\":\"\",\"Worktime\":0,\"Zodiac\":\"鼠\"}";
    quint32 year = 2020;
    quint32 month = 12;
    quint32 day = 13;
    QString gethuangli = calendarHuangLi->getHuangLiDay(year, month, day);
    assert(huangli_20201213 == gethuangli);

    //2020年12月14日黄历信息
    const QString huangli_20201214 = "{\"Avoid\":\"入宅.修造.动土.破土.安门.上梁.\",\"GanZhiDay\":\"辛卯\",\"GanZhiMonth\":\"戊子\","
                                     "\"GanZhiYear\":\"庚子\",\"LunarDayName\":\"三十\",\"LunarFestival\":\"\",\"LunarLeapMonth\":0,"
                                     "\"LunarMonthName\":\"十月\",\"SolarFestival\":\"\",\"Suit\":\"祭祀.入殓.移柩.余事勿取.\",\"Term\":"
                                     "\"\",\"Worktime\":0,\"Zodiac\":\"鼠\"}";
    day = 14;
    gethuangli = calendarHuangLi->getHuangLiDay(year, month, day);
    assert(huangli_20201214 == gethuangli);
}

//QString CalendarHuangLi::GetHuangLiMonth(quint32 year, quint32 month, bool fill)
TEST_F(test_calendarhuangli, GetHuangLiMonth)
{
    quint32 year = 2020;
    quint32 month = 12;
    bool fill = false;
    calendarHuangLi->getHuangLiMonth(year, month, fill);

    fill = true;
    calendarHuangLi->getHuangLiMonth(year, month, fill);
}

//CaLunarDayInfo CalendarHuangLi::GetLunarInfoBySolar(quint32 year, quint32 month, quint32 day)
TEST_F(test_calendarhuangli, GetLunarInfoBySolar)
{
    quint32 year = 2020;
    quint32 month = 12;
    quint32 day = 13;
    CaLunarDayInfo huangliDayInfo = calendarHuangLi->getLunarInfoBySolar(year, month, day);
}

//CaLunarMonthInfo CalendarHuangLi::GetLunarCalendarMonth(quint32 year, quint32 month, bool fill)
TEST_F(test_calendarhuangli, GetLunarCalendarMonth)
{
    quint32 year = 2020;
    quint32 month = 12;
    bool fill = false;
    calendarHuangLi->getLunarCalendarMonth(year, month, fill);

    fill = true;
    calendarHuangLi->getLunarCalendarMonth(year, month, fill);
}
