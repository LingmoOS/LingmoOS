// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_checkerboarditme.h"

#include <stub.h>

TEST_F(UT_CheckerboardItme, UT_CheckerboardItem_boundingRect)
{
    QRectF rectf = m_checkerboardItem->boundingRect();
    EXPECT_EQ(rectf.height(), 662)
            << "check checkerboardItem boundingRect";
}

TEST_F(UT_CheckerboardItme, UT_CheckerboardItem_paint)
{
    QStyleOptionGraphicsItem *option = nullptr;
    QWidget *widget = nullptr;
    QPainter painter;
    m_checkerboardItem->paint(&painter, option, widget);
}
