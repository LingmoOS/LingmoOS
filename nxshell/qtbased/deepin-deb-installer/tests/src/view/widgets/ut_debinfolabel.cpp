// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "../deb-installer/view/widgets/debinfolabel.h"

#include <QPaintEvent>

class UT_DebInfoLabel_TEST : public ::testing::Test
{
    // Test interface
protected:
    void SetUp() { m_label = new DebInfoLabel; }
    void TearDown() { delete m_label; }

    DebInfoLabel *m_label = nullptr;
};

TEST_F(UT_DebInfoLabel_TEST, DebInfoLabel_UT_setCustomQPalette)
{
    m_label->setCustomQPalette(QPalette::WindowText);
    ASSERT_EQ(QPalette::WindowText, m_label->m_colorRole);
    ASSERT_FALSE(m_label->m_bUserColorType);
}

TEST_F(UT_DebInfoLabel_TEST, DebInfoLabel_UT_setCustomQPalette_01)
{
    m_label->setCustomQPalette(QPalette::BrightText);
    ASSERT_EQ(QPalette::BrightText, m_label->m_colorRole);
    ASSERT_FALSE(m_label->m_bUserColorType);
}

TEST_F(UT_DebInfoLabel_TEST, DebInfoLabel_UT_setCustomDPalette)
{
    m_label->setCustomDPalette();
    ASSERT_TRUE(m_label->m_bMultiIns);
    ASSERT_TRUE(m_label->m_bUserColorType);
}

TEST_F(UT_DebInfoLabel_TEST, DebInfoLabel_UT_setCustomDPalette_01)
{
    m_label->setCustomDPalette(DPalette::TextLively);
    ASSERT_EQ(DPalette::TextLively, m_label->m_colorType);
    ASSERT_FALSE(m_label->m_bMultiIns);
    ASSERT_TRUE(m_label->m_bUserColorType);
}

TEST_F(UT_DebInfoLabel_TEST, DebInfoLabel_UT_PaintEvent)
{
    m_label->m_bUserColorType = true;
    m_label->m_bMultiIns = true;
    QPaintEvent paint(QRect(m_label->rect()));
    m_label->paintEvent(&paint);
    ASSERT_TRUE(m_label->m_bUserColorType);
    ASSERT_TRUE(m_label->m_bMultiIns);
}

TEST_F(UT_DebInfoLabel_TEST, DebInfoLabel_UT_PaintEvent_01)
{
    m_label->m_bUserColorType = true;
    m_label->m_bMultiIns = false;
    QPaintEvent paint1(QRect(m_label->rect()));
    m_label->paintEvent(&paint1);
    ASSERT_TRUE(m_label->m_bUserColorType);
    ASSERT_FALSE(m_label->m_bMultiIns);
}

TEST_F(UT_DebInfoLabel_TEST, DebInfoLabel_UT_PaintEvent_02)
{
    m_label->m_colorRole = QPalette::WindowText;
    m_label->m_bUserColorType = false;
    QPaintEvent paint2(QRect(m_label->rect()));
    m_label->paintEvent(&paint2);
    ASSERT_FALSE(m_label->m_bUserColorType);
    ASSERT_EQ(QPalette::WindowText, m_label->m_colorRole);
}

TEST_F(UT_DebInfoLabel_TEST, DebInfoLabel_UT_PaintEvent_03)
{
    m_label->m_colorRole = QPalette::BrightText;
    m_label->m_bUserColorType = false;
    QPaintEvent paint2(QRect(m_label->rect()));
    m_label->paintEvent(&paint2);
    ASSERT_FALSE(m_label->m_bUserColorType);
    ASSERT_EQ(QPalette::BrightText, m_label->m_colorRole);
}
