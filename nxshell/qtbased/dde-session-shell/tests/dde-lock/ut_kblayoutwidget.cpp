// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "kblayoutlistview.h"

#include <QTest>

#include <gtest/gtest.h>

class UT_KbLayoutWidget : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    KBLayoutListView *m_kbLayoutWidget;
};

void UT_KbLayoutWidget::SetUp()
{
    m_kbLayoutWidget = new KBLayoutListView();
}

void UT_KbLayoutWidget::TearDown()
{
    delete m_kbLayoutWidget;
}

TEST_F(UT_KbLayoutWidget, BasicTest)
{
    m_kbLayoutWidget->initData(QStringList());
}
