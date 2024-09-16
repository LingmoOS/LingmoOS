// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_resuleinfo.h"
#include <QPaintEvent>


TEST_F(UT_ResuleInfo, UT_ResuleInfo_setResult1)
{
    m_ResuleInof->setResult(true);
    EXPECT_EQ(m_ResuleInof->mResult, true)
            << "check resultInfo setResult";
}

TEST_F(UT_ResuleInfo, UT_ResuleInfo_setResult2)
{
    m_ResuleInof->setResult(false);
    EXPECT_EQ(m_ResuleInof->mResult, false)
            << "check resultInfo setResult";
}


TEST_F(UT_ResuleInfo, UT_ResuleInfo_paintEvent1)
{
    QRect rect;
    QPaintEvent event(rect);
    m_ResuleInof->paintEvent(&event);
}
