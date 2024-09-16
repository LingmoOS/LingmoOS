// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../ut_Head.h"
#include <gtest/gtest.h>
#include "../stub.h"
#include "threadpooltask.h"
#include <QThreadPool>
#include "cpu/cpuinfo.h"
#include "deviceinfomanager.h"

class ThreadPoolTask_UT : public UT_HEAD
{
public:
    void SetUp()
    {
    }
    void TearDown()
    {
    }
};

bool ut_loadCpuInfo()
{
    return true;
}
void ut_logicalCpus(QString &info)
{
    info  = QString("%1\n").arg("Architecture:        x86_64");
    info += QString("%1\n").arg("CPU op-mode(s):      32-bit, 64-bit");
    info += QString("%1\n").arg("Byte Order:          Little Endian");
    info += QString("%1\n").arg("Address sizes:       39 bits physical, 48 bits virtual");
    info += QString("%1\n").arg("CPU(s):              16");
    info += QString("%1\n").arg("On-line CPU(s) list: 0-15");
    info += QString("%1\n").arg("Thread(s) per core:  2");
    info += QString("%1\n").arg("Core(s) per socket:  8");
    info += QString("%1\n").arg("Socket(s):           1");
    info += QString("%1\n").arg("NUMA node(s):        1");
    info += QString("%1\n").arg("Vendor ID:           GenuineIntel");
    info += QString("%1\n").arg("CPU family:          6");
    info += QString("%1\n").arg("Model:               165");
    info += QString("%1\n").arg("Model name:          Intel(R) Core(TM) i7-10700 CPU @ 2.90GHz");
    return;
}
int ut_physicalNum()
{
    return 1;
}

int ut_coreNum()
{
    return 8;
}

int ut_logicalNum()
{
    return 16;
}
TEST_F(ThreadPoolTask_UT, ThreadPoolTask_UT_lscpu)
{
    Stub stub;
    stub.set(ADDR(CpuInfo, loadCpuInfo), ut_loadCpuInfo);
    stub.set(ADDR(CpuInfo, physicalNum), ut_physicalNum);
    stub.set(ADDR(CpuInfo, coreNum), ut_coreNum);
    stub.set(ADDR(CpuInfo, logicalNum), ut_logicalNum);

    QThreadPool tp;
    ThreadPoolTask *task = new ThreadPoolTask("lscpu", "lscpu.txt",true, 500);
    tp.start(task);

    EXPECT_TRUE(!DeviceInfoManager::getInstance()->getInfo("lscpu").isEmpty());
    EXPECT_TRUE(!DeviceInfoManager::getInstance()->getInfo("lscpu_num").isEmpty());
}
