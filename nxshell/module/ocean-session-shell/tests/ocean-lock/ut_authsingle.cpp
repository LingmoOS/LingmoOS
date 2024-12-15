// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "authcommon.h"
#include "auth_single.h"

#include <gtest/gtest.h>

class UT_AuthSingle : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    AuthSingle *m_authSingle;
};

void UT_AuthSingle::SetUp()
{
    m_authSingle = new AuthSingle;
}

void UT_AuthSingle::TearDown()
{
    delete m_authSingle;
}

TEST_F(UT_AuthSingle, basic)
{
    m_authSingle->lineEditText();
    m_authSingle->setLimitsInfo(LimitsInfo());
}

TEST_F(UT_AuthSingle, AuthResultTest)
{
    m_authSingle->setAuthState(INT_MAX, "default");
    m_authSingle->setAuthState(AuthCommon::AS_Success, "Success");
    m_authSingle->setAuthState(AuthCommon::AS_Failure, "Failure");
    m_authSingle->setAuthState(AuthCommon::AS_Cancel, "Cancel");
    m_authSingle->setAuthState(AuthCommon::AS_Timeout, "Timeout");
    m_authSingle->setAuthState(AuthCommon::AS_Error, "Error");
    m_authSingle->setAuthState(AuthCommon::AS_Verify, "Verify");
    m_authSingle->setAuthState(AuthCommon::AS_Exception, "Exception");
    m_authSingle->setAuthState(AuthCommon::AS_Prompt, "Prompt");
    m_authSingle->setAuthState(AuthCommon::AS_Started, "Started");
    m_authSingle->setAuthState(AuthCommon::AS_Ended, "Ended");
    m_authSingle->setAuthState(AuthCommon::AS_Locked, "Locked");
    m_authSingle->setAuthState(AuthCommon::AS_Recover, "Recover");
    m_authSingle->setAuthState(AuthCommon::AS_Unlocked, "Unlocked");
}

TEST_F(UT_AuthSingle, CapsStatusTest)
{
    m_authSingle->setCapsLockVisible(true);
    m_authSingle->setCapsLockVisible(false);
}

TEST_F(UT_AuthSingle, AnimationStateTest)
{
    m_authSingle->setAnimationState(true);
    m_authSingle->setAnimationState(false);
}

TEST_F(UT_AuthSingle, LineEditInfoTest)
{
    m_authSingle->setLineEditInfo("Alert", AuthModule::AlertText);
    m_authSingle->setLineEditInfo("Input", AuthModule::InputText);
    m_authSingle->setLineEditInfo("PlaceHolder", AuthModule::PlaceHolderText);
}

TEST_F(UT_AuthSingle, KeyboardButtonTest)
{
    m_authSingle->setKeyboardButtonVisible(true);
    m_authSingle->setKeyboardButtonInfo("cn;");
}
