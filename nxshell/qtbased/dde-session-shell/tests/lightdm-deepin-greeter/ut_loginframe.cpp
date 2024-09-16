// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loginframe.h"
#include "sessionbasemodel.h"
#include "userinfo.h"

#include <QResizeEvent>

#include <gtest/gtest.h>

class UT_LoginFrame : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    SessionBaseModel *m_sessionBaseModel;
    std::shared_ptr<User> m_user;
    LoginFrame *m_loginwindow;
};

void UT_LoginFrame::SetUp()
{
    m_sessionBaseModel = new SessionBaseModel();
    m_user = std::make_shared<User>();
    m_sessionBaseModel->updateCurrentUser(m_user);
    m_loginwindow = new LoginFrame(m_sessionBaseModel);
}

void UT_LoginFrame::TearDown()
{
    delete m_sessionBaseModel;
    delete m_loginwindow;
}

TEST_F(UT_LoginFrame, Validity)
{
    ASSERT_NE(m_loginwindow, nullptr);
}

TEST_F(UT_LoginFrame, Visibility)
{
    m_loginwindow->show();
    m_loginwindow->hide();
}
