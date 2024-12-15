// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "controlwidget.h"
#include "sessionbasemodel.h"
#include <gtest/gtest.h>
#include <QApplication>
#include <QKeyEvent>

#include <QTest>

class UT_ControlWidget : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    ControlWidget *m_controlWidget;
    SessionBaseModel *m_model;
};

void UT_ControlWidget::SetUp()
{
    m_model = new SessionBaseModel();
    m_model->setAppType(Lock);
    std::shared_ptr<User> user_ptr(new User);
    m_model->updateCurrentUser(user_ptr);
    m_controlWidget = new ControlWidget(m_model);
}

void UT_ControlWidget::TearDown()
{
    delete m_controlWidget;
    delete m_model;
}

TEST_F(UT_ControlWidget, init)
{
    m_controlWidget->setUserSwitchEnable(true);
    m_controlWidget->setUserSwitchEnable(false);
    m_controlWidget->setSessionSwitchEnable(true);
    m_controlWidget->chooseToSession("");
    m_controlWidget->chooseToSession("aaaa");

    QTest::keyRelease(m_controlWidget, Qt::Key_0, Qt::KeyboardModifier::NoModifier);
}
