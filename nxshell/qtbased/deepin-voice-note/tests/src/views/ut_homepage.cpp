// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_homepage.h"
#include "homepage.h"

UT_HomePage::UT_HomePage()
{
}

void UT_HomePage::SetUp()
{
    m_homepage = new HomePage;
}

void UT_HomePage::TearDown()
{
    delete m_homepage;
}

TEST_F(UT_HomePage, UT_HomePage_eventFilter_001)
{
    QKeyEvent keyTab(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    EXPECT_TRUE(m_homepage->eventFilter(nullptr, &keyTab));
    QKeyEvent keyEnter(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    EXPECT_TRUE(m_homepage->eventFilter(nullptr, &keyEnter));
}
