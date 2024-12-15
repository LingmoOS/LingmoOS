// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lockframe.h"
#include "sessionbasemodel.h"

#include <QSignalSpy>

#include <gtest/gtest.h>

class UT_LockFrame : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    SessionBaseModel *m_model;
    LockFrame *m_lockFrame;
};

void UT_LockFrame::SetUp()
{
    m_model = new SessionBaseModel();
    std::shared_ptr<User> user_ptr(new User);
    m_model->updateCurrentUser(user_ptr);

    m_lockFrame = new LockFrame(m_model);
}

void UT_LockFrame::TearDown()
{
    delete m_model;
    delete m_lockFrame;
}

TEST_F(UT_LockFrame, frame)
{
    m_lockFrame->showUserList();
    m_lockFrame->showLockScreen();
    m_lockFrame->showShutdown();
    m_lockFrame->cancelShutdownInhibit(false);
    m_lockFrame->shutdownInhibit(SessionBaseModel::PowerAction::RequireNormal, false);
    m_lockFrame->handlePoweroffKey();
}

TEST_F(UT_LockFrame, connection)
{
    emit m_model->prepareForSleep(false);
    emit m_model->authFinished(true);
}

TEST_F(UT_LockFrame, event)
{
     //m_lockFrame->keyPressEvent(new QKeyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier));
     //m_lockFrame->event(new QKeyEvent(QEvent::KeyRelease, Qt::Key_PowerOff, Qt::NoModifier));
     //m_lockFrame->event(new QKeyEvent(QEvent::KeyRelease, Qt::Key_NumLock, Qt::NoModifier));
     //m_lockFrame->event(new QKeyEvent(QEvent::KeyRelease, Qt::Key_TouchpadOn, Qt::NoModifier));
     //m_lockFrame->event(new QKeyEvent(QEvent::KeyRelease, Qt::Key_TouchpadOff, Qt::NoModifier));
     //m_lockFrame->event(new QKeyEvent(QEvent::KeyRelease, Qt::Key_TouchpadToggle, Qt::NoModifier));
     //m_lockFrame->event(new QKeyEvent(QEvent::KeyRelease, Qt::Key_CapsLock, Qt::NoModifier));
     //m_lockFrame->event(new QKeyEvent(QEvent::KeyRelease, Qt::Key_VolumeDown, Qt::NoModifier));
     //m_lockFrame->event(new QKeyEvent(QEvent::KeyRelease, Qt::Key_VolumeUp, Qt::NoModifier));
     //m_lockFrame->event(new QKeyEvent(QEvent::KeyRelease, Qt::Key_VolumeMute, Qt::NoModifier));
     //m_lockFrame->event(new QKeyEvent(QEvent::KeyRelease, Qt::Key_MonBrightnessUp, Qt::NoModifier));
     //m_lockFrame->event(new QKeyEvent(QEvent::KeyRelease, Qt::Key_MonBrightnessDown, Qt::NoModifier));
     m_lockFrame->hide();
     m_lockFrame->show();
}
