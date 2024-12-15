// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../ut_Head.h"
#include <gtest/gtest.h>
#include "../stub.h"
#include "cpu/cpuinfo.h"
#include <sys/utsname.h>
#include "deviceinfomanager.h"
#include "DDLog.h"
#include <cstring>

using namespace DDLog;

class CpuInfo_UT : public UT_HEAD
{
public:
    void SetUp()
    {
    }
    void TearDown()
    {
    }
};

int stub_uname (struct utsname &__name){
    memset(&__name, 0, sizeof(utsname));
    memcpy(&__name.sysname   , "Linux", sizeof("Linux"));
    memcpy(&__name.nodename  , "uos-PC", sizeof("uos-PC"));
    memcpy(&__name.release   , "4.19.0-amd64-desktop", sizeof("4.19.0-amd64-desktop"));
    memcpy(&__name.version   , "#4016 SMP Wed Jun 23 13:35:29 CST 2021", sizeof("#4016 SMP Wed Jun 23 13:35:29 CST 2021"));
    memcpy(&__name.machine   , "x86_64", sizeof("x86_64"));
    memcpy(&__name.domainname, "(none)", sizeof("(none)"));
    return 1;
}

TEST_F(CpuInfo_UT, ThreadPoolTask_UT_lscpu)
{
    Stub stub;
    //stub.set(&uname, stub_uname);
    CpuInfo cpu;
    if (cpu.loadCpuInfo()) {

        qCInfo(appLog) << cpu.arch();
        EXPECT_TRUE(!cpu.arch().isEmpty());

        QString info;
        cpu.logicalCpus(info);
        DeviceInfoManager::getInstance()->addInfo("lscpu", info);
        EXPECT_TRUE(!info.isEmpty());

        QString numInfo;
        numInfo += QString("%1 : %2\n").arg("physical").arg(cpu.physicalNum());
        numInfo += QString("%1 : %2\n").arg("core").arg(cpu.coreNum());
        numInfo += QString("%1 : %2\n").arg("logical").arg(cpu.logicalNum());
        DeviceInfoManager::getInstance()->addInfo("lscpu_num", numInfo);
        EXPECT_TRUE(!numInfo.isEmpty());
    }
}
