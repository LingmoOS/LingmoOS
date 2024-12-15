// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logintipswindow.h"

#include <QLabel>
#include <QPushButton>
#include <QSignalSpy>
#include <QTest>

#include <gtest/gtest.h>

class UT_LoginTipsWindow : public testing::Test
{
public:
    void SetUp() override;
    void TearDown() override;

public:
    LoginTipsWindow *m_loginTipsWindow;
};

void UT_LoginTipsWindow::SetUp()
{
    m_loginTipsWindow = new LoginTipsWindow();
}

void UT_LoginTipsWindow::TearDown()
{
    delete m_loginTipsWindow;
}

TEST_F(UT_LoginTipsWindow, BasicTest)
{
    ASSERT_NE(m_loginTipsWindow, nullptr);
    m_loginTipsWindow->isValid();
}

TEST_F(UT_LoginTipsWindow, buttonClicked)
{
    QPushButton *button = m_loginTipsWindow->findChild<QPushButton *>("RequireSureButton");
    QTest::mouseClick(button, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0, 0));
}
