// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "toolbar.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <QKeyEvent>



/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/

class UT_Toolbar : public ::testing::Test
{
public:
    UT_Toolbar() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static QWidget wid;
        m_tester = new Toolbar(&wid);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    Toolbar *m_tester;
};

TEST_F(UT_Toolbar, initTest)
{

}


TEST_F(UT_Toolbar, test_eventFilter_01)
{
    static QKeyEvent ev(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    m_tester->eventFilter(m_tester, &ev);
}

TEST_F(UT_Toolbar, test_eventFilter_02)
{
    static QKeyEvent ev(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    m_tester->eventFilter(m_tester, &ev);
}

TEST_F(UT_Toolbar, test_eventFilter_03)
{
    static QKeyEvent ev(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    m_tester->eventFilter(m_tester, &ev);
}

TEST_F(UT_Toolbar, test_eventFilter_04)
{
    static QKeyEvent ev(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    m_tester->eventFilter(m_tester, &ev);
}

TEST_F(UT_Toolbar, test_handleSearch_01)
{
    m_tester->handleSearch();
}

TEST_F(UT_Toolbar, test_handleSearchTextChanged_01)
{
    m_tester->handleSearchTextChanged();
}

TEST_F(UT_Toolbar, test_isSearchContentEmpty_01)
{
    bool rt = m_tester->isSearchContentEmpty();

    EXPECT_FALSE(rt);
}

TEST_F(UT_Toolbar, test_isSearchContentEmpty_02)
{
    m_tester->searchEdit->setText("1");
    bool rt = m_tester->isSearchContentEmpty();

    EXPECT_TRUE(rt);
}

TEST_F(UT_Toolbar, test_searchContent_01)
{
    EXPECT_EQ(m_tester->searchContent(), "");
}

TEST_F(UT_Toolbar, test_searchContent_02)
{
    m_tester->searchEdit->setText("1");
    EXPECT_EQ(m_tester->searchContent(), "1");
}

TEST_F(UT_Toolbar, test_setProcessButtonChecked_01)
{
    m_tester->setProcessButtonChecked(true);
}

TEST_F(UT_Toolbar, test_focusInput_01)
{
    m_tester->focusInput();

    EXPECT_EQ(m_tester->searchEdit->text(), "");
}


TEST_F(UT_Toolbar, test_focusInput_02)
{
    m_tester->searchEdit->setText("1");
    m_tester->focusInput();

    EXPECT_NE(m_tester->searchEdit->text(), "");
}

TEST_F(UT_Toolbar, test_clearSearchText_01)
{
    m_tester->clearSearchText();
}

