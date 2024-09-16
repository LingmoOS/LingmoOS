// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "launchersys.h"
#undef private

#include <gtest/gtest.h>

class Tst_LauncherSys : public testing::Test
{
public:
    void SetUp() override
    {
        m_launcherSys = new LauncherSys(nullptr);
    }

    void TearDown() override
    {
        if (m_launcherSys) {
            delete m_launcherSys;
            m_launcherSys = nullptr;
        }
    }

public:
    LauncherSys *m_launcherSys;
};

TEST_F(Tst_LauncherSys, launcherSys_test)
{
    m_launcherSys->registerRegion();
    m_launcherSys->displayModeChanged();

    m_launcherSys->onDisplayModeChanged();
    m_launcherSys->onVisibleChanged();

    m_launcherSys->hideLauncher();
    m_launcherSys->unRegisterRegion();
}
