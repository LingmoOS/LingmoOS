// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_METHOD_INTERFACE_H
#define TEST_METHOD_INTERFACE_H

#include "method_interface.h"
#include "gtest/gtest.h"
#include <QObject>

class test_method_interface : public QObject, public::testing::Test
{
public:
    test_method_interface();
};

#endif // TEST_METHOD_INTERFACE_H
