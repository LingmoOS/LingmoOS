// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
using namespace std;

#include <limits.h>
#include "gtest/gtest.h"
#include <QApplication>

DCORE_USE_NAMESPACE

//  gtest的入口函数
int main(int argc, char **argv)
{

    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication a(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    auto c = RUN_ALL_TESTS();
    //Q_UNUSED(c);
    return c;
}
