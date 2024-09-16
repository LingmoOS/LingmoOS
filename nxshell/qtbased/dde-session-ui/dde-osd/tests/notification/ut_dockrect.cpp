// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "types/dockrect.h"
#undef private

#include <QDebug>

#include <gtest/gtest.h>

class UT_DockRect : public testing::Test
{

};

TEST_F(UT_DockRect, coverageTest)
{
    DockRect obj;
    qDebug() << obj;
    QDBusArgument arg ;
    arg << obj;
    arg >> obj;
}
