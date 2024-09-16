// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define protected public
#include "listview.h"
#undef protected

#include <QMouseEvent>

#include <gtest/gtest.h>

class UT_ListView : public testing::Test
{

};

TEST_F(UT_ListView, coverageTest)
{
    ListView listView;
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    listView.mousePressEvent(&event);
}
