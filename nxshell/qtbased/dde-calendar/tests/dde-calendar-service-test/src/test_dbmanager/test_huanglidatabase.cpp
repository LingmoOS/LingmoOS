// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_huanglidatabase.h"
#include "../third-party_stub/stub.h"
#include "config.h"

bool stub_OpenHuangliDatabase(void *obj, const QString &dbpath)
{
    Q_UNUSED(dbpath);
    HuangLiDataBase *o = reinterpret_cast<HuangLiDataBase *>(obj);
    o->m_database = QSqlDatabase::addDatabase("QSQLITE");
    o->m_database.setDatabaseName(HL_DATABASE_DIR);
    return o->m_database.open();
}

test_huanglidatabase::test_huanglidatabase()
{
    Stub stub;
    stub.set(ADDR(HuangLiDataBase, OpenHuangliDatabase), stub_OpenHuangliDatabase);
    hlDb = new HuangLiDataBase();
}

test_huanglidatabase::~test_huanglidatabase()
{
    if (hlDb->m_database.isOpen()) {
        hlDb->m_database.close();
    }
    delete hlDb;
}

//QString HuangLiDataBase::QueryFestivalList(quint32 year, quint8 month)
TEST_F(test_huanglidatabase, QueryFestivalList)
{
    quint32 year = 2020;
    quint8 month = 10;
    const QString strFes = "[{\"description\":\"10月1日至10月8日放假8天，9月27日，10月10日上班\",\"id\":\"2020100110\",\"list\":"
                           "[{\"date\":\"2020-10-1\",\"status\":1},"
                           "{\"date\":\"2020-10-2\",\"status\":1},"
                           "{\"date\":\"2020-10-3\",\"status\":1},"
                           "{\"date\":\"2020-10-4\",\"status\":1},"
                           "{\"date\":\"2020-10-5\",\"status\":1},"
                           "{\"date\":\"2020-10-6\",\"status\":1},"
                           "{\"date\":\"2020-10-7\",\"status\":1},"
                           "{\"date\":\"2020-10-8\",\"status\":1},"
                           "{\"date\":\"2020-9-27\",\"status\":2},"
                           "{\"date\":\"2020-10-10\",\"status\":2}],"
                           "\"month\":10,\"name\":\"中秋节\",\"rest\":"
                           "\"10月9日至10月10日请假2天，与周末连休可拼11天长假。\"}]";
    QString getFes = hlDb->QueryFestivalList(year, month);
    assert(strFes == getFes);
}

//QList<stHuangLi> HuangLiDataBase::QueryHuangLiByDays(const QList<stDay> &days)
TEST_F(test_huanglidatabase, QueryHuangLiByDays)
{
    QList<stDay> days;
    stDay day1, day2;
    day1.Year = day2.Year = 2020;
    day1.Month = day2.Month = 10;
    day1.Day = 1;
    day2.Day = 2;
    days << day1 << day2;

    QList<stHuangLi> getHuangli = hlDb->QueryHuangLiByDays(days);

    stHuangLi hl1, hl2;
    hl1 = getHuangli.at(0);
    hl2 = getHuangli.at(1);
    QString hl2Suit = "开光.解除.起基.动土.拆卸.上梁.立碑.修坟.安葬.破土.启攒.移柩.";
    QString hl2Avoid = "嫁娶.出行.安床.作灶.祭祀.入宅.移徙.出火.进人口.置产.";
    assert(20201001 == hl1.ID && 20201002 == hl2.ID);
    assert(hl2Suit == hl2.Suit && hl2Avoid == hl2.Avoid);
}

////bool HuangLiDataBase::OpenHuangliDatabase(const QString &dbpath)
//TEST_F(test_huanglidatabase, OpenHuangliDatabase)
//{
//    QString dbpath = "";
//    hlDb->OpenHuangliDatabase(dbpath);

//    dbpath = "123123";
//    hlDb->OpenHuangliDatabase(dbpath);

//    dbpath = "%s%s%s%s%s%s";
//    hlDb->OpenHuangliDatabase(dbpath);

//    hlDb->OpenHuangliDatabase(HL_DATABASE_DIR);
//}
