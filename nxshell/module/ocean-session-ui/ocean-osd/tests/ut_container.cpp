// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define protected public
#define private public
#include "container.h"
#undef protected
#undef private

#include <QTest>

#include <gtest/gtest.h>

class UT_Container : public testing::Test
{

};

TEST_F(UT_Container, coverageTest)
{
    Container container;
    QWidget w;
    container.setContent(&w);

    container.show();
    container.hide();
    container.onDelayQuit();
}
