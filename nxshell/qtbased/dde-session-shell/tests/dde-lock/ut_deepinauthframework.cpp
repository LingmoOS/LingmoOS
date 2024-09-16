// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "authcommon.h"
#include "deepinauthframework.h"

#include <gtest/gtest.h>

class UT_DeepinAuthFramework : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    //DeepinAuthFramework *m_authFramework;
};

void UT_DeepinAuthFramework::SetUp()
{
//    m_authFramework = new DeepinAuthFramework();
}

void UT_DeepinAuthFramework::TearDown()
{
//    delete m_authFramework;
}

TEST_F(UT_DeepinAuthFramework, PAMTest)
{
    // m_authFramework->CreateAuthenticate("uos");
//    m_authFramework->SendToken("123");
//    m_authFramework->DestroyAuthenticate();
}

TEST_F(UT_DeepinAuthFramework, DATest)
{
//    const QString UserName("uos");
//    m_authFramework->GetFrameworkState();
//    m_authFramework->GetSupportedMixAuthFlags();
//    m_authFramework->GetPreOneKeyLogin(0);
//    m_authFramework->GetLimitedInfo(UserName);
//    m_authFramework->GetSupportedEncrypts();

//    m_authFramework->CreateAuthController(UserName, 19, AuthCommon::AppTypeLock);
//    m_authFramework->StartAuthentication(UserName, 19, -1);
//    m_authFramework->SendTokenToAuth(UserName, 1, "123");
//    m_authFramework->EndAuthentication(UserName, 19);
//    m_authFramework->DestroyAuthController(UserName);

//    m_authFramework->GetFuzzyMFA(UserName);
//    m_authFramework->GetMFAFlag(UserName);
//    m_authFramework->GetPINLen(UserName);
//    m_authFramework->GetFactorsInfo(UserName);
//    m_authFramework->GetPrompt(UserName);
//    m_authFramework->SetPrivilegesDisable(UserName);
//    m_authFramework->SetPrivilegesEnable(UserName, "");
//    m_authFramework->AuthSessionPath(UserName);
}
