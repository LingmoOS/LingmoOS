// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logincontent.h"
#include "sessionbasemodel.h"

#include <gtest/gtest.h>

class UT_LoginContent : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    SessionBaseModel *m_model;
    LoginContent *m_content;
};

void UT_LoginContent::SetUp()
{
    m_model = new SessionBaseModel();
    std::shared_ptr<User> user_ptr(new User);
    m_model->updateCurrentUser(user_ptr);

    m_content = new LoginContent(m_model);
}

void UT_LoginContent::TearDown()
{
    delete m_content;
}

TEST_F(UT_LoginContent, BasicTest)
{
    m_content->onCurrentUserChanged(m_model->currentUser());
    m_content->tryPushTipsFrame();
    m_content->pushLoginFrame();
}

TEST_F(UT_LoginContent, ModeTest)
{
    m_content->onStatusChanged(SessionBaseModel::NoStatus);
    m_content->onStatusChanged(SessionBaseModel::PowerMode);
    m_content->onStatusChanged(SessionBaseModel::ConfirmPasswordMode);
    m_content->onStatusChanged(SessionBaseModel::UserMode);
    m_content->onStatusChanged(SessionBaseModel::PowerMode);
    m_content->onStatusChanged(SessionBaseModel::ShutDownMode);
    m_content->restoreMode();
}
