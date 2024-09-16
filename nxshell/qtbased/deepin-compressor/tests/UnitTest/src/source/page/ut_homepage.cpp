// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "homepage.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>

#include <DGuiApplicationHelper>
#include <DApplicationHelper>
#include <QTest>

DGUI_USE_NAMESPACE

class UT_HomePage : public ::testing::Test
{
public:
    UT_HomePage(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new HomePage;
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    HomePage *m_tester;
};

TEST_F(UT_HomePage, initTest)
{

}

TEST_F(UT_HomePage, dragEnterEvent)
{

}

TEST_F(UT_HomePage, dragMoveEvent)
{

}

TEST_F(UT_HomePage, dropEvent)
{

}

TEST_F(UT_HomePage, test_slotThemeChanged_001)
{
    Stub stub;
    DGuiApplicationHelperStub::stub_DGuiApplicationHelper_themeType(stub, DGuiApplicationHelper::LightType);
    m_tester->slotThemeChanged();
    EXPECT_EQ(m_tester->m_pSplitLbl->pixmap()->toImage() == QPixmap(":assets/icons/deepin/builtin/light/icons/split_line.svg").toImage(), true);
}

TEST_F(UT_HomePage, test_slotThemeChanged_002)
{
    Stub stub;
    DGuiApplicationHelperStub::stub_DGuiApplicationHelper_themeType(stub, DGuiApplicationHelper::DarkType);
    m_tester->slotThemeChanged();
    EXPECT_EQ(m_tester->m_pSplitLbl->pixmap()->toImage() == QPixmap(":assets/icons/deepin/builtin/dark/icons/split_line_dark.svg").toImage(), true);
}

TEST_F(UT_HomePage, test_slotThemeChanged_003)
{
    Stub stub;
    DGuiApplicationHelperStub::stub_DGuiApplicationHelper_themeType(stub, DGuiApplicationHelper::UnknownType);
    m_tester->slotThemeChanged();
    EXPECT_EQ(m_tester->m_pSplitLbl->pixmap()->toImage() == QPixmap(":assets/icons/deepin/builtin/light/icons/split_line.svg").toImage(), true);
}
