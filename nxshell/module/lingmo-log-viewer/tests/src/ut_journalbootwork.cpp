// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "journalbootwork.h"
#include "ut_stuballthread.h"
#include <stub.h>

#include <DApplication>

#include <QDebug>
#include <QThreadPool>
#include <QThread>

#include <gtest/gtest.h>
#include <systemd/sd-journal.h>

int stub_sd_journal_get_data_bootwork001(sd_journal *j, const char *field, const void **data, size_t *l)
{
    return 0;
}

QString stub_getReplaceColorStr_bootwork001(const char *d)
{
    return "testaaaa";
}
TEST(JournalBootWork_Constructor_UT, JournalBootWork_Constructor_UT_001)
{
    JournalBootWork *p = new JournalBootWork(nullptr);
    EXPECT_NE(p, nullptr);
    p->deleteLater();
}
TEST(JournalBootWork_Constructor_UT, JournalBootWork_Constructor_UT_002)
{
    JournalBootWork *p = new JournalBootWork(QStringList() << "", nullptr);
    EXPECT_NE(p, nullptr);
    p->deleteLater();
}

TEST(JournalBootWork_Constructor_UT, JournalBootWork_Constructor_UT_003)
{
    JournalBootWork *p = new JournalBootWork(QStringList() << "PRIORITY=1", nullptr);
    EXPECT_NE(p, nullptr);
    EXPECT_EQ(p->m_arg.contains("PRIORITY=1"), true);
    p->deleteLater();
}

TEST(JournalBootWork_Destructor_UT, JournalBootWork_Destructor_UT_001)
{
    JournalBootWork *p = new JournalBootWork(nullptr);
    EXPECT_NE(p, nullptr);
    p->~JournalBootWork();
    EXPECT_EQ(p->logList.isEmpty(), true);
    EXPECT_EQ(p->m_map.isEmpty(), true);
    p->deleteLater();
}

TEST(JournalBootWork_stopWork_UT, JournalBootWork_stopWork_UT_001)
{
    JournalBootWork *p = new JournalBootWork(nullptr);
    EXPECT_NE(p, nullptr);
    p->stopWork();
    EXPECT_EQ(p->m_canRun, false);
    p->deleteLater();
}

TEST(JournalBootWork_getIndex_UT, JournalBootWork_getIndex_UT_001)
{
    JournalBootWork *p = new JournalBootWork(nullptr);
    EXPECT_NE(p, nullptr);
    EXPECT_EQ(p->getIndex(), p->getPublicIndex());
    p->deleteLater();
}
TEST(JournalBootWork_setArg_UT, JournalBootWork_setArg_UT_001)
{
    JournalBootWork *p = new JournalBootWork(nullptr);
    EXPECT_NE(p, nullptr);
    QStringList testArg;
    testArg << "PRIORITY=1";
    p->setArg(testArg);
    EXPECT_EQ(p->m_arg.contains("PRIORITY=1"), true);
    p->deleteLater();
}

TEST(JournalBootWork_setArg_UT, JournalBootWork_setArg_UT_002)
{
    JournalBootWork *p = new JournalBootWork(nullptr);
    EXPECT_NE(p, nullptr);
    QStringList testArg;
    p->setArg(testArg);
    EXPECT_EQ(p->m_arg.isEmpty(), true);
    p->deleteLater();
}

TEST(JournalBootWork_doWork_UT, JournalBootWork_doWork_UT)
{
    Stub stub;
    stub.set(sd_journal_get_data, stub_sd_journal_get_data_bootwork001);
    stub.set(ADDR(JournalBootWork, getReplaceColorStr), stub_getReplaceColorStr_bootwork001);
    JournalBootWork *p = new JournalBootWork(nullptr);
    p->doWork();
    EXPECT_NE(p, nullptr);
    p->deleteLater();
}

TEST(JournalBootWork_getReplaceColorStr_UT, JournalBootWork_getReplaceColorStr_UT)
{
    JournalBootWork *p = new JournalBootWork(nullptr);
    EXPECT_NE(p, nullptr);
    EXPECT_EQ(p->getReplaceColorStr("\033[40;37m"), "");
    p->deleteLater();
}

TEST(JournalBootWork_getDateTimeFromStamp_UT, JournalBootWork_getDateTimeFromStamp_UT)
{
    JournalBootWork *p = new JournalBootWork(nullptr);
    EXPECT_NE(p, nullptr);
//    QString s_data = "2020-10-20 10:29:55";
    QString s_dataLine = "1603160995838863";
//    EXPECT_EQ(p->getDateTimeFromStamp(s_dataLine), s_data) << "check the status after getDateTimeFromStamp()";
    EXPECT_NE(p->getDateTimeFromStamp(s_dataLine), nullptr) << "check the status after getDateTimeFromStamp()";
    //  EXPECT_EQ(rs, true);
    p->deleteLater();
}

TEST(JournalBootWork_initMap_UT, JournalBootWork_initMap_UT)
{
    JournalBootWork *p = new JournalBootWork(nullptr);
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

TEST(JournalBootWork_i2str_UT, JournalBootWork_i2str_UT)
{
    JournalBootWork *p = new JournalBootWork(nullptr);
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

int stub_sd_journal_open(sd_journal **ret, int flags)
{
    return -1;
}


class JournalBootWork_UT : public testing::Test
{
public:
    //添加日志
    static void SetUpTestCase()
    {
        qDebug() << "SetUpTestCase" << endl;
    }
    static void TearDownTestCase()
    {
        qDebug() << "TearDownTestCase" << endl;
    }
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_bootWork = new JournalBootWork(nullptr);
        qDebug() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        delete m_bootWork;
    }
    JournalBootWork *m_bootWork;
};


TEST_F(JournalBootWork_UT, testBootWork_UT001)
{
    Stub stub;
    stub.set(sd_journal_open, stub_sd_journal_open);
    stub.set(sd_journal_seek_tail, stub_sd_journal_open);
    stub.set(sd_journal_add_match, stub_sd_journal_open);
    stub.set(sd_journal_add_conjunction, stub_sd_journal_open);
    stub.set(sd_journal_get_data, stub_sd_journal_open);
    m_bootWork->m_arg = QStringList() << "emg"<< "all";
    m_bootWork->doWork();
    EXPECT_EQ(m_bootWork->m_canRun, true);
    EXPECT_EQ(m_bootWork->logList.count(), 0);
}
