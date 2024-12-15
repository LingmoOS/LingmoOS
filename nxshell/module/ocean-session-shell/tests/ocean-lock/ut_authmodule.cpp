// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "auth_module.h"
#include "authcommon.h"

#include <gtest/gtest.h>

class UT_AuthModule : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    AuthModule *m_authModule;
};

void UT_AuthModule::SetUp()
{
    m_authModule = new AuthModule(AuthCommon::AT_None);
}

void UT_AuthModule::TearDown()
{
    delete m_authModule;
}

TEST_F(UT_AuthModule, BasicTest)
{
    m_authModule->authState();
    m_authModule->authType();
    m_authModule->setAnimationState(false);
    m_authModule->setAuthState(0, "test");
    // m_authModule->setAuthState("");
    m_authModule->setLimitsInfo(LimitsInfo());
}
