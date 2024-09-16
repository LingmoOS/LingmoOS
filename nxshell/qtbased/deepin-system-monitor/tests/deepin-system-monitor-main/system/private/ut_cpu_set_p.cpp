// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "system/private/cpu_set_p.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

using namespace core::system;

class UT_CPUSetPrivate: public ::testing::Test
{
public:
    UT_CPUSetPrivate() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CPUSetPrivate();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    CPUSetPrivate *m_tester;
};

TEST_F(UT_CPUSetPrivate, initTest)
{
}


TEST_F(UT_CPUSetPrivate, test_cpoy)
{
    FILE *fp;
    QByteArray line(BUFSIZ, '\0');
    int ncpu = 0;
    int nr;

    if (!(fp = fopen("/proc/stat", "r"))) {
//        return;
    }

    while (fgets(line.data(), BUFSIZ, fp)) {
        if (!strncmp(line.data(), "cpu", 3)) {
                    // per cpu stat in jiffies
                    auto stat = std::make_shared<struct cpu_stat_t>();
                    if (stat) {
                        *stat = {};

                        nr = sscanf(line.data() + 3, "%d %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                                    &ncpu,
                                    &stat->user,
                                    &stat->nice,
                                    &stat->sys,
                                    &stat->idle,
                                    &stat->iowait,
                                    &stat->hardirq,
                                    &stat->softirq,
                                    &stat->steal,
                                    &stat->guest,
                                    &stat->guest_nice);

                        if (nr == 11) {
                            QByteArray cpu {"cpu"};
                            cpu.append(QByteArray::number(ncpu));
                            stat->cpu = cpu;

                            // usage calc
                            auto usage = std::make_shared<struct cpu_usage_t>();
                            if (usage) {
                                *usage = {};
                                usage->cpu = cpu;
                                usage->total = stat->user + stat->nice + stat->sys + stat->idle + stat->iowait + stat->hardirq + stat->softirq + stat->steal;
                                usage->idle = stat->idle + stat->iowait;

                                m_tester->m_statDB[stat->cpu] = stat;
                                m_tester->m_usageDB[usage->cpu] = usage;

                                QList<CPUInfo> infos{};
                                CPUInfo info{};
                                infos.append(info);
                                m_tester->m_infos = infos;
                            }

                        }
                    }
        }
    }

    CPUSetPrivate copy(*m_tester);
}


