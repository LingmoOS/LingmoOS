// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "userbutton.h"
#include "userinfo.h"

#include <gtest/gtest.h>

class UT_UserButton : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    UserButton *m_button;
};

void UT_UserButton::SetUp()
{
//    std::shared_ptr<User> user_ptr(new User);

//    m_button = new UserButton(user_ptr);
}

void UT_UserButton::TearDown()
{
     //delete m_button;
}

TEST_F(UT_UserButton, BasicTest)
{
//     m_button->userInfo();
//     m_button->setSelected(true);
//     m_button->selected();
//     m_button->setImageSize(UserButton::AvatarSize::AvatarNormalSize);
//     m_button->move(QPoint(0, 0));
//     //m_button->show();
//     m_button->hide();
//     m_button->stopAnimation();
//     m_button->setOpacity(1.0);
//     m_button->opacity();
//     m_button->setCustomEffect();
//     m_button->addTextShadowAfter();
}
