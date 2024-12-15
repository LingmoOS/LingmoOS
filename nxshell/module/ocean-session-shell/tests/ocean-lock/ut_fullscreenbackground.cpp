// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fullscreenbackground.h"
#include "sessionbasemodel.h"

#include <QTest>

#include <gtest/gtest.h>

class UT_FullscreenBackground : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    FullscreenBackground *m_background;
    SessionBaseModel *m_model;
};

void UT_FullscreenBackground::SetUp()
{
    m_model = new SessionBaseModel();
    std::shared_ptr<User> user_ptr(new User);
    m_model->updateCurrentUser(user_ptr);
    m_background = new FullscreenBackground(m_model);
}

void UT_FullscreenBackground::TearDown()
{
    delete m_background;
    delete m_model;
}

TEST_F(UT_FullscreenBackground, BasicTest)
{
    QWidget w;
    m_background->setContent(&w);
    m_background->isPicture("/usr/share/backgrounds/default_background.jpg");
    m_background->getLocalFile("/usr/share/backgrounds/default_background.jpg");
    m_background->setContentVisible(true);
    m_background->contentVisible();
    m_background->setEnterEnable(true);
    m_background->updateBackground("/usr/share/backgrounds/default_background.jpg");
    m_background->updateBlurBackground("/usr/share/backgrounds/default_background.jpg");
    QTest::keyPress(m_background, Qt::Key_0, Qt::KeyboardModifier::NoModifier);
}
