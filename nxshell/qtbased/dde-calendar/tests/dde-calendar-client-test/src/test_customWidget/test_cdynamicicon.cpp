// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_cdynamicicon.h"

test_cdynamicicon::test_cdynamicicon()
{
    mDynamicicon = new CDynamicIcon(8, 8);
}

test_cdynamicicon::~test_cdynamicicon()
{
    delete mDynamicicon;
    mDynamicicon = nullptr;
}

//void CDynamicIcon::setDate(const QDate &date)
TEST_F(test_cdynamicicon, setDate)
{
    mDynamicicon->setDate(QDate::currentDate());
}

//void CDynamicIcon::setTitlebar(DTitlebar *titlebar)
TEST_F(test_cdynamicicon, setTitlebar)
{
    DTitlebar *titlebar = new DTitlebar();
    mDynamicicon->setTitlebar(titlebar);
    EXPECT_EQ(mDynamicicon->m_Titlebar, titlebar);
    delete titlebar;
}

