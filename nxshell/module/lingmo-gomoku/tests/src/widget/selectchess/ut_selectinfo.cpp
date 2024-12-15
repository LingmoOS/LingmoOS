// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_selectinfo.h"

#include <QPaintEvent>

TEST_F(UT_SelectInfo, UT_SelectInfo_paint)
{
    QRect rect;
    QPaintEvent event(rect);
    m_SelectInfo->paintEvent(&event);
}
