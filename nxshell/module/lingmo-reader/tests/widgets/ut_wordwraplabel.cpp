// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "WordWrapLabel.h"

#include <QPaintEvent>

#include <gtest/gtest.h>

class UT_WordWrapLabel : public ::testing::Test
{
public:
    UT_WordWrapLabel(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new WordWrapLabel();
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    WordWrapLabel *m_tester;
};

TEST_F(UT_WordWrapLabel, initTest)
{

}

TEST_F(UT_WordWrapLabel, UT_WordWrapLabel_setText)
{
    m_tester->setText("123");
    EXPECT_TRUE(m_tester->m_text == "123");
}

TEST_F(UT_WordWrapLabel, UT_WordWrapLabel_setMargin)
{
    m_tester->setMargin(1);
    EXPECT_TRUE(m_tester->m_margin == 1);
}

TEST_F(UT_WordWrapLabel, UT_WordWrapLabelt_paintEvent)
{
    QPaintEvent paint(QRect(m_tester->rect()));
    m_tester->paintEvent(&paint);
    EXPECT_FALSE(m_tester->grab().isNull());
}

TEST_F(UT_WordWrapLabel, UT_WordWrapLabel_adjustContent)
{
    QFontMetrics fontMetris(m_tester->font());
    int wordHeight = fontMetris.boundingRect(0, 0, m_tester->width() - 2 * m_tester->m_margin, 0, static_cast<int>(m_tester->alignment() | Qt::TextWrapAnywhere), m_tester->m_text).height();
    m_tester->adjustContent();
    EXPECT_TRUE(m_tester->height() == wordHeight);
}
