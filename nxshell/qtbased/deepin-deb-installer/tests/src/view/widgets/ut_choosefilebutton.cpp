// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../deb-installer/view/widgets/choosefilebutton.h"
#include "utils/utils.h"

#include <QKeyEvent>

#include <gtest/gtest.h>

class ut_chooseFileButton_TEST : public ::testing::Test
{
    // Test interface
protected:
    void SetUp() { m_choosFileBtn = new ChooseFileButton(""); }
    void TearDown() { delete m_choosFileBtn; }

    ChooseFileButton *m_choosFileBtn = nullptr;
};

TEST_F(ut_chooseFileButton_TEST, ChooseFileButton_UT_setFamily)
{
    m_choosFileBtn->setText("1");
    ASSERT_EQ("1", m_choosFileBtn->text());
}

TEST_F(ut_chooseFileButton_TEST, ChooseFileButton_UT_keyPressEvent)
{
    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    QCoreApplication::sendEvent(m_choosFileBtn, &keyPressEvent);
}
