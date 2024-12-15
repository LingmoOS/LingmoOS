// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/cpu.h"
#include "system/private/cpu_p.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

//qt
#include <QString>
#include <QProcess>
using namespace core::system;

class UT_CPU: public ::testing::Test
{
public:
    UT_CPU() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CPUInfo();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    CPUInfo *m_tester;
};

TEST_F(UT_CPU, initTest)
{
}

TEST_F(UT_CPU, test_copy_01)
{
    CPUInfo copy(*m_tester);
}

TEST_F(UT_CPU, test_copy_02)
{
    CPUInfo copy(1);
}

TEST_F(UT_CPU, test_copy_03)
{
    const CPUInfo rhs;
    m_tester->operator=(rhs);
}

TEST_F(UT_CPU, test_copy_04)
{
    CPUInfo* rhs = m_tester;
    m_tester->operator= (*rhs);
}


TEST_F(UT_CPU, test_logicalIndex)
{
    QList<CPUInfo> infos;

    QProcess process;
    process.start("cat /proc/cpuinfo");
    process.waitForFinished(3000);
    QString cpuinfo = process.readAllStandardOutput();
    QStringList processors = cpuinfo.split("\n\n", QString::SkipEmptyParts);

    for (int i = 0; i < 1; ++i) {
        core::system::CPUInfo info;
        QStringList list = processors[i].split("\n", QString::SkipEmptyParts);
        for (QString text : list) {
            if (text.startsWith("processor")) {
                info.setIndex(text.split(":").value(1).toInt());
            } else if (text.startsWith("vendor_id")) {
                info.setVendorId(text.split(":").value(1));
            } else if (text.startsWith("cpu MHz")) {
                info.setCpuFreq(text.split(":").value(1));
            } else if (text.startsWith("core id")) {
                info.setCoreId(text.split(":").value(1).toUInt());
            } else if (text.startsWith("model name")) {
                info.setModelName(text.split(":").value(1));
            } else if (text.startsWith("cache size")) {
                info.setCacheSize(text.split(":").value(1));
            }
        }

        infos.append(info);

//        qInfo()<<"logicalIndex:"<<info.logicalIndex();
        EXPECT_EQ(0, info.logicalIndex());
    }
}

TEST_F(UT_CPU, test_logicalName)
{
    m_tester->logicalName();
}

TEST_F(UT_CPU, test_coreID)
{
    m_tester->setCoreId(10);
    qInfo()<<"coreID:"<<m_tester->coreID();
    EXPECT_EQ("10", m_tester->coreID());
}

TEST_F(UT_CPU, test_modelName)
{
    m_tester->setModelName("abc");
    EXPECT_EQ("abc", m_tester->modelName());
}

TEST_F(UT_CPU, test_vendorId)
{
    m_tester->setVendorId("abc");
    EXPECT_EQ("abc", m_tester->vendorId());
}


TEST_F(UT_CPU, test_cpuFreq_01)
{
    m_tester->setCpuFreq("abc");
    QString retString =  m_tester->cpuFreq();
//    qInfo()<<"cpuFreq:"<<retString.length();
    EXPECT_NE(3, retString.length());
}

TEST_F(UT_CPU, test_cpuFreq_02)
{
    m_tester->setCpuFreq("-");
    QString retString =  m_tester->cpuFreq();
    EXPECT_NE(0, retString.length());
}

TEST_F(UT_CPU, test_cacheSize)
{
    m_tester->setCacheSize("10");
    qInfo()<<"cacheSize:"<<m_tester->cacheSize();
    EXPECT_EQ("10", m_tester->cacheSize());
}

TEST_F(UT_CPU, test_l1iCache)
{
    m_tester->l1iCache();
}

TEST_F(UT_CPU, test_l1dCache)
{
    m_tester->l1dCache();
}

TEST_F(UT_CPU, test_l3Cache)
{
    m_tester->l3Cache();
}

TEST_F(UT_CPU, test_l2Cache)
{
    m_tester->l2Cache();
}

TEST_F(UT_CPU, test_setIndex)
{
    m_tester->setIndex(10);
    EXPECT_EQ(10, m_tester->logicalIndex());
}

TEST_F(UT_CPU, test_setCoreId)
{
    m_tester->setCoreId(10);
    EXPECT_EQ("10", m_tester->coreID());
}

TEST_F(UT_CPU, test_setModelName)
{
    m_tester->setModelName("abc");
    EXPECT_EQ("abc", m_tester->modelName());
}

TEST_F(UT_CPU, test_setVendorId)
{
    m_tester->setVendorId("abc");
    EXPECT_EQ("abc", m_tester->vendorId());
}

TEST_F(UT_CPU, test_setCpuFreq)
{
    m_tester->setCpuFreq("abcde");
    QString retString =  m_tester->cpuFreq();
    EXPECT_NE(5, retString.length());
}

TEST_F(UT_CPU, test_setCacheSize)
{
    m_tester->setCacheSize("10");
    qInfo()<<"cacheSize:"<<m_tester->cacheSize();
    EXPECT_EQ("10", m_tester->cacheSize());
}

