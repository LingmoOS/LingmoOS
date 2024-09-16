// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lockcontent.h"
#include "sessionbasemodel.h"

#include <QTest>

#include <gtest/gtest.h>

class UT_LockContent : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    SessionBaseModel *m_model;
    LockContent *m_content;
};

void UT_LockContent::SetUp()
{
    m_model = new SessionBaseModel();
    std::shared_ptr<User> user_ptr(new User);
    m_model->updateCurrentUser(user_ptr);

    m_content = new LockContent(m_model);
}

void UT_LockContent::TearDown()
{
    delete m_content;
    delete m_model;
}

TEST_F(UT_LockContent, BasicTest)
{
    m_content->onCurrentUserChanged(m_model->currentUser());
    m_content->pushPasswordFrame();
    m_content->pushUserFrame();
    m_content->pushShutdownFrame();
    m_content->setMPRISEnable(true);
}

TEST_F(UT_LockContent, BackgroundTest)
{
    m_content->updateDesktopBackgroundPath("/usr/share/backgrounds/default_background.jpg");
    m_content->updateGreeterBackgroundPath("/usr/share/backgrounds/default_background.jpg");
}

TEST_F(UT_LockContent, ModeTest)
{
    m_content->onStatusChanged(SessionBaseModel::NoStatus);
    m_content->onStatusChanged(SessionBaseModel::PowerMode);
    m_content->onStatusChanged(SessionBaseModel::ConfirmPasswordMode);
    m_content->onStatusChanged(SessionBaseModel::UserMode);
    m_content->onStatusChanged(SessionBaseModel::PowerMode);
    m_content->onStatusChanged(SessionBaseModel::ShutDownMode);
    m_content->restoreMode();
    m_content->updateTimeFormat(true);
    m_content->tryGrabKeyboard();
    m_content->hideToplevelWindow();
    m_content->currentWorkspaceChanged();
    m_content->updateWallpaper("/usr/share/wallpapers/deepin");
    QTest::mouseRelease(m_content, Qt::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0, 0));
    QTest::keyPress(m_content, Qt::Key_0, Qt::KeyboardModifier::NoModifier);
    m_content->show();
    m_content->hide();
}
