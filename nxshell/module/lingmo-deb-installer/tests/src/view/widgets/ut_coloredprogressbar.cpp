// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../deb-installer/view/widgets/coloredprogressbar.h"
#include <stub.h>
#include "utils/utils.h"

#include <DGuiApplicationHelper>
#include <DObjectPrivate>

#include <QPaintEvent>
#include <QObject>

#include <gtest/gtest.h>

DGuiApplicationHelper::ColorType stub_themeType()
{
    return DGuiApplicationHelper::DarkType;
}

class ut_coloredProgressBar_Test : public ::testing::Test
{
    // Test interface
protected:
    void SetUp() { m_colorProcessBar = new ColoredProgressBar; }
    void TearDown() { delete m_colorProcessBar; }

    ColoredProgressBar *m_colorProcessBar = nullptr;
};

DApplicationHelper::ColorType ut_themeType()
{
    return DApplicationHelper::DarkType;
}

DApplicationHelper::ColorType ut_themeType01()
{
    return DApplicationHelper::LightType;
}

TEST_F(ut_coloredProgressBar_Test, ColoredProgressBar_UT_themeChanged)
{
    Stub stub;
    stub.set(ADDR(DApplicationHelper, themeType), ut_themeType01);
    m_colorProcessBar->themeChanged();
    ASSERT_EQ(DGuiApplicationHelper::LightType, m_colorProcessBar->m_themeType);
    //    m_colorProcessBar->d_d_ptr.get()
}

TEST_F(ut_coloredProgressBar_Test, ColoredProgressBar_UT_themeChanged_01)
{
    Stub stub;
    stub.set(ADDR(DApplicationHelper, themeType), ut_themeType);
    m_colorProcessBar->themeChanged();
    ASSERT_EQ(DGuiApplicationHelper::DarkType, m_colorProcessBar->m_themeType);
}

TEST_F(ut_coloredProgressBar_Test, ColoredProgressBar_UT_addThreshold)
{
    m_colorProcessBar->addThreshold(0, QBrush(nullptr));
    EXPECT_EQ(1, m_colorProcessBar->threshmap.size());
}

TEST_F(ut_coloredProgressBar_Test, ColoredProgressBar_UT_removeThreshold)
{
    m_colorProcessBar->removeThreshold(1);
    EXPECT_TRUE(m_colorProcessBar->threshmap.isEmpty());
}

TEST_F(ut_coloredProgressBar_Test, ColoredProgressBar_UT_thresholds)
{
    EXPECT_TRUE(m_colorProcessBar->thresholds().isEmpty());
}

TEST_F(ut_coloredProgressBar_Test, ColoredProgressBar_UT_paintEvent_Light)
{
    QPaintEvent paint(QRect(m_colorProcessBar->rect()));
    m_colorProcessBar->paintEvent(&paint);
}

TEST_F(ut_coloredProgressBar_Test, ColoredProgressBar_UT_paintEvent_Dark)
{
    Stub stub;
    stub.set(ADDR(DGuiApplicationHelper, themeType), stub_themeType);

    QPaintEvent paint(QRect(m_colorProcessBar->rect()));
    m_colorProcessBar->paintEvent(&paint);
}
