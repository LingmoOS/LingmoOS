// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "mem_summary_view_widget.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>

/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/

class UT_MemSummaryViewWidget : public ::testing::Test
{
public:
    UT_MemSummaryViewWidget() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static QWidget parent;
        m_tester = new MemSummaryViewWidget(&parent);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    MemSummaryViewWidget *m_tester;
};

TEST_F(UT_MemSummaryViewWidget, initTest)
{

}

TEST_F(UT_MemSummaryViewWidget, test_fontChanged_01)
{
    QFont font;
    font.setItalic(true);
    m_tester->fontChanged(font);

    EXPECT_TRUE(m_tester->m_font.italic());
}

TEST_F(UT_MemSummaryViewWidget, test_onModelUpdate_01)
{
    m_tester->onModelUpdate();
}

TEST_F(UT_MemSummaryViewWidget, test_paintEvent_01)
{
    EXPECT_TRUE(!m_tester->grab().isNull());
}

