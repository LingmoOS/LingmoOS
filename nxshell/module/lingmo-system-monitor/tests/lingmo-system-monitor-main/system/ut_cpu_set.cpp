// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/cpu_set.h"
#include "system/cpu.h"
#include "system/private/cpu_set_p.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

//qt
#include <QString>
#include <QFile>
#include <QMap>

using namespace core::system;

/***************************************STUB begin*********************************************/

FILE *stub_fopen(const char *__restrict __filename,
                 const char *__restrict __modes)
{
    return nullptr;
}

int stub_sscanf_stats(const char *__restrict __s, const char *__restrict __format, ...)
{
    return 1;
}


/***************************************STUB end**********************************************/

class UT_CPUSet : public ::testing::Test
{
public:
    UT_CPUSet() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CPUSet();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    CPUSet *m_tester;
};

TEST_F(UT_CPUSet, initTest)
{
}

TEST_F(UT_CPUSet, test_copy)
{
    CPUSet copy(*m_tester);
}

TEST_F(UT_CPUSet, test_copy_02)
{
    CPUSet copy1;
    CPUSet copy2 = copy1;
}

TEST_F(UT_CPUSet, test_copy_03)
{
    const CPUSet rhs;
    m_tester->operator=(rhs);
}

TEST_F(UT_CPUSet, test_copy_04)
{
    CPUSet *rhs = m_tester;
    m_tester->operator= (*rhs);
}

TEST_F(UT_CPUSet, test_modelName)
{
    m_tester->update();
    QString retString = m_tester->modelName();
    EXPECT_NE(retString, "");
}

TEST_F(UT_CPUSet, test_vendor_01)
{
    m_tester->update();
    QString retString = m_tester->vendor();
    EXPECT_NE(retString, "");
}

TEST_F(UT_CPUSet, test_vendor_02)
{
    m_tester->update();
    m_tester->mIsEmptyModelName = true;
    QString retString = m_tester->vendor();
}

TEST_F(UT_CPUSet, test_cpuCount)
{
    m_tester->update();
    int iCpuCount = m_tester->cpuCount();
    EXPECT_NE(iCpuCount, 0);
}

TEST_F(UT_CPUSet, test_socketCount)
{
    m_tester->update();
    int iSocketCount = m_tester->socketCount();
    EXPECT_NE(iSocketCount, 0);
}

TEST_F(UT_CPUSet, test_virtualization)
{
    m_tester->update();
    QString retString = m_tester->virtualization();
    EXPECT_NE(retString, "");
}

TEST_F(UT_CPUSet, test_curFreq)
{
    m_tester->update();
    QString retString = m_tester->curFreq();
    EXPECT_NE(retString, "");
}

TEST_F(UT_CPUSet, test_minFreq_01)
{
    m_tester->update();
    QString retString = m_tester->minFreq();
    EXPECT_NE(retString, "");
}

TEST_F(UT_CPUSet, test_minFreq_02)
{
    m_tester->update();
    m_tester->mIsEmptyModelName = true;
    QString retString = m_tester->minFreq();
    EXPECT_NE(retString, "");
}

TEST_F(UT_CPUSet, test_maxFreq_01)
{
    m_tester->update();
    QString retString = m_tester->maxFreq();
    EXPECT_NE(retString, "");
}

TEST_F(UT_CPUSet, test_maxFreq_02)
{
    m_tester->update();
    m_tester->mIsEmptyModelName = true;
    QString retString = m_tester->maxFreq();
    EXPECT_NE(retString, "");
}


TEST_F(UT_CPUSet, test_l1dCache)
{
    m_tester->update();
    QString retString = m_tester->l1dCache();
    EXPECT_NE(retString, "");
}


TEST_F(UT_CPUSet, test_l1iCache)
{
    m_tester->update();
    QString retString = m_tester->l1iCache();
    EXPECT_NE(retString, "");
}

TEST_F(UT_CPUSet, test_l2Cache)
{
    m_tester->update();
    QString retString = m_tester->l2Cache();
    EXPECT_NE(retString, "");
}

TEST_F(UT_CPUSet, test_l3Cache)
{
    m_tester->update();
    QString retString = m_tester->l3Cache();
    EXPECT_NE(retString, "");
}

TEST_F(UT_CPUSet, test_coreId)
{
    m_tester->update();
    QString retString = m_tester->coreId(1);
    EXPECT_NE(retString, "");
}

TEST_F(UT_CPUSet, test_usage)
{
    m_tester->update();
    core::system::CPUUsage Usage = m_tester->usage();
    EXPECT_NE(Usage->total, 1);
}

TEST_F(UT_CPUSet, test_stat)
{
    m_tester->update();
    core::system::CPUStat stat = m_tester->stat();
    EXPECT_NE(stat->idle, 0);
}

TEST_F(UT_CPUSet, test_cpuLogicName)
{
    m_tester->update();
    for (auto &cpuname : m_tester->cpuLogicName()) {
        //cpu逻辑名不能为空
//        qInfo()<<"cpuname:"<<cpuname;
        EXPECT_NE(cpuname, "");
    }
}

TEST_F(UT_CPUSet, test_statDB)
{
    m_tester->update();
    QByteArray  TEST_F = "cpu0";
    core::system::CPUStat stat = m_tester->statDB(TEST_F);
    qInfo() << "stat->user:" << stat->user;
    EXPECT_NE(stat->user, 0);
}

TEST_F(UT_CPUSet, test_usageDB)
{
    m_tester->update();
    QByteArray  TEST_F = "cpu0";
    core::system::CPUUsage cPUUsage = m_tester->usageDB(TEST_F);
    qInfo() << "cPUUsage->total:" << cPUUsage->total;
    EXPECT_NE(cPUUsage->total, 0);
}

TEST_F(UT_CPUSet, test_update)
{
    m_tester->update();
    core::system::CPUStat stat = m_tester->stat();
    EXPECT_NE(stat->idle, 0);
}

TEST_F(UT_CPUSet, test_read_stats)
{
    m_tester->read_stats();
    core::system::CPUStat stat = m_tester->stat();
    EXPECT_NE(stat->idle, 0);
}

TEST_F(UT_CPUSet, test_read_stats_02)
{
    Stub stub;
    stub.set(fopen, stub_fopen);
    m_tester->read_stats();
    core::system::CPUStat stat = m_tester->stat();
    EXPECT_NE(stat->idle, -1);
}

TEST_F(UT_CPUSet, test_read_stats_03)
{
    Stub stub;
    stub.set(sscanf, stub_sscanf_stats);
    m_tester->read_stats();
    core::system::CPUStat stat = m_tester->stat();
    EXPECT_NE(stat->idle, -1);
}

TEST_F(UT_CPUSet, test_read_overall_info)
{
    m_tester->read_overall_info();
    QString retString = m_tester->l1iCache();
    EXPECT_NE(retString, "");
}


TEST_F(UT_CPUSet, test_read_lscpu_01)
{
    m_tester->read_lscpu();
    QString retString = m_tester->vendor();
    EXPECT_NE(retString, "");
}

bool stub_isEmpty()
{
    return true;
}

TEST_F(UT_CPUSet, test_read_lscpu_02)
{
    Stub stub;
    stub.set(ADDR(QString, isEmpty), stub_isEmpty);
    m_tester->read_lscpu();
    QString retString = m_tester->vendor();
    EXPECT_NE(retString, "");
}

bool stub_contains()
{
    return false;
}

TEST_F(UT_CPUSet, test_read_lscpu_03)
{
//   Stub stub;
//   stub.set(ADDR(QMap, contains), stub_contains);
//   m_tester->read_lscpu();
//   QString retString = m_tester->vendor();
//   EXPECT_NE(retString, "");
}

TEST_F(UT_CPUSet, test_getUsageTotalDelta_02)
{
    m_tester->d->cpusageTotal[kCurrentStat] = 0;
    qulonglong totalDelta = m_tester->getUsageTotalDelta();
    EXPECT_NE(totalDelta, 0);
}
