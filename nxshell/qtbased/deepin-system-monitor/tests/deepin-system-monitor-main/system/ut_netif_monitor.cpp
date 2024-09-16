// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/netif_monitor.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <atomic>

//gtest
#include "stub.h"
#include <gtest/gtest.h>

//qt
#include <QDebug>
#include <QTimer>
#include <QWaitCondition>
#include <QMap>

using namespace core::system;

/***************************************STUB begin*********************************************/

bool stub_isEmpty_false()
{
    return false;
}

bool stub_wait(void* obj, QMutex *lockedMutex, unsigned long time = ULONG_MAX)
{
    return false;
}

template <class Key, class T>
bool stub_contains(void* obj, const int key)
{
    return false;
}

/***************************************STUB end**********************************************/


class UT_NetifMonitor: public QObject, public ::testing::Test
{
//    Q_OBJECT
public:
    UT_NetifMonitor() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new NetifMonitor();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    NetifMonitor *m_tester;
};

TEST_F(UT_NetifMonitor, initTest)
{
}

TEST_F(UT_NetifMonitor, test_instace)
{
    EXPECT_TRUE(m_tester->instance() != nullptr);
}

TEST_F(UT_NetifMonitor, test_startNetmonitorJob)
{
    m_tester->startNetmonitorJob();
    EXPECT_TRUE( m_tester->m_packetMonitorThread.isRunning() == true);
}

TEST_F(UT_NetifMonitor, test_handleNetData)
{
//    QTimer::singleShot(1000, this, [=]() {
//        Stub stub;
//        stub.set(ADDR(PacketPayloadQueue, isEmpty), stub_isEmpty_false());
//    });

//    Stub stub;
////    stub.set((bool(QWaitCondition::*)(QMutex *, unsigned long time))ADDR(QWaitCondition, wait), stub_wait);
//    stub.set(ADDR(std::atomic_bool, load), stub_load);
//    m_tester->handleNetData();
}

//TEST_F(UT_NetifMonitor, test_getSockIOStatByInode)
//{
//    Stub stub;
//    stub.set((bool(QMap::*)(int))ADDR(QMap,contains), stub_contains);
//    ino_t ino = 1;
//    SockIOStat stat;
//    qulonglong totalDelta = m_tester->getSockIOStatByInode(ino,stat);
//    EXPECT_NE(totalDelta, 0);
//}
