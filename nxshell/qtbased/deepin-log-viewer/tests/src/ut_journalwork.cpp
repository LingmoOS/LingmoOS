// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stub.h>
#include "journalwork.h"
#include "ut_stuballthread.h"

#include <DApplication>

#include <QDebug>

#include <gtest/gtest.h>

int stub_sd_journal_get_data_work(sd_journal *j, const char *field, const void **data, size_t *l)
{
    return 0;
}

QString stub_getReplaceColorStr_work(const char *d)
{
    return "testaaaa";
}

TEST(journalWork_Constructor_UT, journalWork_Constructor_UT_001)
{
    journalWork *p = new journalWork(nullptr);
    EXPECT_NE(p, nullptr);
    p->deleteLater();
}
TEST(journalWork_Constructor_UT, journalWork_Constructor_UT_002)
{
    journalWork *p = new journalWork(QStringList() << "", nullptr);
    EXPECT_NE(p, nullptr);
    p->deleteLater();
}

TEST(journalWork_Constructor_UT, journalWork_Constructor_UT_003)
{
    journalWork *p = new journalWork(QStringList() << "PRIORITY=1", nullptr);
    EXPECT_NE(p, nullptr);
    EXPECT_EQ(p->m_arg.contains("PRIORITY=1"), true);
    p->deleteLater();
}

TEST(journalWork_Destructor_UT, journalWork_Destructor_UT_001)
{
    journalWork *p = new journalWork(nullptr);
    EXPECT_NE(p, nullptr);
    p->~journalWork();
    EXPECT_EQ(p->logList.isEmpty(), true);
    EXPECT_EQ(p->m_map.isEmpty(), true);
    p->deleteLater();
}

TEST(journalWork_stopWork_UT, journalWork_stopWork_UT_001)
{
    journalWork *p = new journalWork(nullptr);
    EXPECT_NE(p, nullptr);
    p->stopWork();
    EXPECT_EQ(p->m_canRun, false);
    p->deleteLater();
}

TEST(journalWork_getIndex_UT, journalWork_getIndex_UT_001)
{
    journalWork *p = new journalWork(nullptr);
    EXPECT_NE(p, nullptr);
    EXPECT_EQ(p->getIndex(), p->getPublicIndex());
    p->deleteLater();
}
TEST(journalWork_setArg_UT, journalWork_setArg_UT_001)
{
    journalWork *p = new journalWork(nullptr);
    EXPECT_NE(p, nullptr);
    QStringList testArg;
    testArg << "PRIORITY=1";
    p->setArg(testArg);
    EXPECT_EQ(p->m_arg.contains("PRIORITY=1"), true);
    p->deleteLater();
}

TEST(journalWork_setArg_UT, journalWork_setArg_UT_002)
{
    journalWork *p = new journalWork(nullptr);
    EXPECT_NE(p, nullptr);
    QStringList testArg;
    p->setArg(testArg);
    EXPECT_EQ(p->m_arg.isEmpty(), true);
    p->deleteLater();
}

TEST(journalWork_doWork_UT, journalWork_doWork_UT)
{
    journalWork *p = new journalWork(nullptr);
    EXPECT_NE(p, nullptr);
    Stub stub;
    stub.set(sd_journal_get_data, stub_sd_journal_get_data_work);
    stub.set(ADDR(journalWork, getReplaceColorStr), stub_getReplaceColorStr_work);
    p->doWork();
    p->deleteLater();
}

TEST(journalWork_getReplaceColorStr_UT, journalWork_getReplaceColorStr_UT)
{
    journalWork *p = new journalWork(nullptr);
    EXPECT_NE(p, nullptr);
    EXPECT_EQ(p->getReplaceColorStr("\033[40;37m"), "");
    p->deleteLater();
}

TEST(journalWork_getDateTimeFromStamp_UT, journalWork_getDateTimeFromStamp_UT)
{
    journalWork *p = new journalWork(nullptr);
    EXPECT_NE(p, nullptr);
    QString s_dataLine = "1603160995838863";
    EXPECT_NE(p->getDateTimeFromStamp(s_dataLine), " ") << "check the status after getDateTimeFromStamp()";
    p->deleteLater();
}

TEST(journalWork_initMap_UT, journalWork_initMap_UT)
{
    journalWork *p = new journalWork(nullptr);
    EXPECT_NE(p, nullptr);
    p->initMap();
    QMap<int, QString> map;
    map.insert(0, Dtk::Widget::DApplication::translate("Level", "Emergency"));
    map.insert(1, Dtk::Widget::DApplication::translate("Level", "Alert"));
    map.insert(2, Dtk::Widget::DApplication::translate("Level", "Critical"));
    map.insert(3, Dtk::Widget::DApplication::translate("Level", "Error"));
    map.insert(4, Dtk::Widget::DApplication::translate("Level", "Warning"));
    map.insert(5, Dtk::Widget::DApplication::translate("Level", "Notice"));
    map.insert(6, Dtk::Widget::DApplication::translate("Level", "Info"));
    map.insert(7, Dtk::Widget::DApplication::translate("Level", "Debug"));
    EXPECT_EQ(map, p->m_map);
    p->deleteLater();
}

TEST(journalWork_i2str_UT, journalWork_i2str_UT)
{
    journalWork *p = new journalWork(nullptr);
    EXPECT_NE(p, nullptr);
    QMap<int, QString> map;
    map.insert(0, Dtk::Widget::DApplication::translate("Level", "Emergency"));
    map.insert(1, Dtk::Widget::DApplication::translate("Level", "Alert"));
    map.insert(2, Dtk::Widget::DApplication::translate("Level", "Critical"));
    map.insert(3, Dtk::Widget::DApplication::translate("Level", "Error"));
    map.insert(4, Dtk::Widget::DApplication::translate("Level", "Warning"));
    map.insert(5, Dtk::Widget::DApplication::translate("Level", "Notice"));
    map.insert(6, Dtk::Widget::DApplication::translate("Level", "Info"));
    map.insert(7, Dtk::Widget::DApplication::translate("Level", "Debug"));
    foreach (int key, map.keys()) {
        EXPECT_EQ(p->i2str(key), map.value(key));
    }

    p->deleteLater();
}
