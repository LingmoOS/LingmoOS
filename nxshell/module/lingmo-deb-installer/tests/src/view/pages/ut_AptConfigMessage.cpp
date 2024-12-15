// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../deb-installer/view/pages/AptConfigMessage.h"
#include "../deb-installer/view/widgets/ShowInstallInfoTextEdit.h"
#include <stub.h>
#include <ut_Head.h>

#include <QDebug>

#include <gtest/gtest.h>

class AptConfigMessage_UT : public UT_HEAD
{
public:
    // 添加日志
    static void SetUpTestCase() { qDebug() << "SetUpTestCase" << endl; }
    static void TearDownTestCase() { qDebug() << "TearDownTestCase" << endl; }
    void SetUp()  // TEST跑之前会执行SetUp
    {
        aptConfig = new AptConfigMessage();
        usleep(100 * 1000);
        qDebug() << "SetUp" << endl;
    }
    void TearDown()  // TEST跑完之后会执行TearDown
    {
        delete aptConfig;
    }
    AptConfigMessage *aptConfig;
};

TEST_F(AptConfigMessage_UT, UT_AptConfigMessage_clearTexts)
{
    aptConfig->clearTexts();
    EXPECT_TRUE(aptConfig->m_inputEdit->text().isEmpty());
    EXPECT_TRUE(aptConfig->m_textEdit->m_editor->toPlainText().isEmpty());
}

TEST_F(AptConfigMessage_UT, UT_AptConfigMessage_appendTextEdit)
{
    aptConfig->appendTextEdit("test");
    aptConfig->appendTextEdit("test\\n");
    EXPECT_NE(4, aptConfig->m_textEdit->m_editor->toPlainText().size());
}

TEST_F(AptConfigMessage_UT, UT_AptConfigMessage_dealInput)
{
    aptConfig->dealInput();
    aptConfig->m_inputEdit->setText("test");
    aptConfig->dealInput();
    EXPECT_TRUE(aptConfig->m_inputEdit->text().isEmpty());
}

TEST_F(AptConfigMessage_UT, paintEvent_UT)
{
    QPaintEvent paint(QRect(aptConfig->rect()));
    aptConfig->paintEvent(&paint);
}
