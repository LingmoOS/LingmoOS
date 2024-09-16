// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEST_PERFORMANCEMONITOR_H
#define TEST_PERFORMANCEMONITOR_H
#include "gtest/gtest.h"
#include <QObject>

class test_performanceMonitor : public QObject, public::testing::Test
{
public:
    test_performanceMonitor();
};

#endif // TEST_PERFORMANCEMONITOR_H
